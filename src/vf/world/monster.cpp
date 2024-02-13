#include "monster.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "verdant.h"

namespace vf {

Monster::Monster () {
    types |= Types::Monster;
}

void Monster::init () {
    if (!defined(home_pos)) home_pos = pos;
    if (!home_left) home_left = left;
}

void Monster::Walker_on_hit (
    const Hitbox& hb, Walker& victim, const Hitbox& o_hb
) {
    victim.left = !left;
    bool high = pos.y - victim.pos.y > 8;
    uint8 decal_i = high ? 3 : 1;
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    auto& victim_body = *victim.data->poses->dead[0].body;
    float weapon_tip = pos.x + left_flip(
        weapon_offset.x + data->poses->hit[0].weapon->hitbox.r
    );
    Vec decal_pos = victim.pos + victim.left_flip(victim_body.decals[decal_i]);
    float depth = left_flip(weapon_tip - decal_pos.x);
    victim.decal_type = high ? DecalType::SlashHigh : DecalType::SlashLow;
    victim.decal_index = decal_i;
    if (high || depth > 4) delay_weapon_layers = 0x5;
    else if (delay_weapon_layers == 0x1) delay_weapon_layers = 0x3;
    if (depth > 6) {
        victim.pos.x += left_flip(depth - 6);
    }
    else if (depth < 1) {
        pos.x += left_flip(1 - depth);
    }
    Walker::Walker_on_hit(hb, victim, o_hb);
}

void Monster::Walker_draw_weapon (const Pose& pose) {
    if (state == WS::Attack && anim_phase == 5) {
        weapon_layers = delay_weapon_layers;
    }
    return Walker::Walker_draw_weapon(pose);
}

void Monster::Resident_on_exit () {
    if (state == WS::Dead) {
         // Finish dying offscreen
        anim_phase = 10;
        anim_timer = 0;
    }
    else {
         // Otherwise return to start
        pos = home_pos;
        vel = {};
        walk_start_x = pos.x;
        left = *home_left;
        set_state(WS::Neutral);
    }
    Walker::Resident_on_exit();
}

 // Evaluate quadratic, but stopping when velocity reaches 0 instead of going
 // backwards.
float quadratic_until_stop (float p, float v, float a, float t) {
     // 0 = v + at
     // -v = at
     // -v/a = t
    if (a) {
        float peak_t = -(v / a);
        if (peak_t >= 0 && t > peak_t) t = peak_t;
    }
    return p + v*t + a*(t*t);
}

float predict (Walker& self, Walker& target, float self_acc, float time) {
    auto predicted_x = quadratic_until_stop(
        self.pos.x, self.vel.x,
        self.left ? -self_acc : self_acc,
        time
    );
     // Assume target will attack and decelerate.
    float target_dec =
        target.floor ? target.data->coast_dec : target.data->air_dec;
    auto predicted_target_x = quadratic_until_stop(
        target.pos.x, target.vel.x,
        target.left ? target_dec : -target_dec,
        time
    );
    return predicted_target_x - predicted_x;
}

Controls MonsterMind::Mind_think (Resident& s) {
    Controls r {};
    if (!(s.types & Types::Monster)) return r;
    auto& self = static_cast<Monster&>(s);
    if (!target || target->state == WS::Dead) return r;

     // Calculcate some distances
    float dist = target->pos.x - self.pos.x;
     // Predict where target will be when attack animation finishes.
    float attack_dist = predict(
        self, *target,
        self.floor ? -self.data->coast_dec : -self.data->air_dec,
        self.data->attack_sequence[0] + self.data->attack_sequence[1]
    );
     // For jump distance, use the amount of time it's likely to take for our
     // damage box to leave the hazard area of the target's weapon.
    float jump_dist = predict(self, *target, 0, 4);

     // Work with right-facing coordinates
    auto forward = self.left ? Control::Left : Control::Right;
    auto backward = self.left ? Control::Right : Control::Left;
    if (self.left) {
        dist = -dist;
        attack_dist = -attack_dist;
        jump_dist = -jump_dist;
    }

    switch (self.alert_phase) {
        case 0: {
            if (length(dist) < sight_range) {
                self.alert_phase += 1;
            }
            else break;
            [[fallthrough]];
        }
        case 1: {
            if (self.alert_timer >= alert_sequence[0]) {
                self.alert_phase += 1;
                self.alert_timer = 0;
            }
            else {
                self.alert_timer += 1;
                 // Do nothing, haven't reacted yet
                break;
            }
            [[fallthrough]];
        }
        case 2: {
            if (self.alert_timer == 0) {
                 // Alert other monsters
                for (auto& r : self.room->residents) {
                    if (!(r.types & Types::Monster)) continue;
                    auto& fren = static_cast<Monster&>(r);
                    if (fren.state == WS::Dead) continue;
                    if (fren.alert_phase == 0) fren.alert_phase = 1;
                }
            }
            if (self.alert_timer >= alert_sequence[1]) {
                self.alert_phase += 1;
                self.alert_timer = 0;
            }
            else {
                self.alert_timer += 1;
                 // Look at target
                if (dist < 0) r[backward] = 1;
                break;
            }
            [[fallthrough]];
        }
        case 3: {
            if (dist < 0) {
                r[backward] = 1;
            }
            else if (attack_dist < attack_range) {
                if (
                     // Don't attack too early when jumping unless the player is
                     // above.
                    (self.vel.y < 1 || target->pos.y > self.pos.y + 8) &&
                     // Don't hold preattack pose
                    (self.state != WS::Attack || self.anim_phase >= 3)
                ) {
                    r[Control::Attack] = 1;
                }
            }
            else if (jump_dist < jump_range) {
                r[forward] = 1;
                r[Control::Jump] = 1;
            }
            else r[forward] = 1;
             // Wait for or jump over other monsters
            for (auto& other : self.room->residents) {
                if (&other == &s || !(other.types & Types::Monster)) continue;
                auto& fren = static_cast<Monster&>(other);
                if (fren.state == WS::Dead) continue; // :(
                auto dist = fren.pos.x - self.pos.x;
                if (self.left) dist = -dist;
                if (dist > 0 && dist < social_distance) {
                    if (jump_dist && fren.left != self.left) {
                        r[Control::Jump] = 1;
                    }
                    else {
                        r[Control::Jump] = 0;
                        r[forward] = 0;
                    }
                }
            }
            break;
        }
    }
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Monster,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("home_pos", &Monster::home_pos, optional),
        attr("home_left", &Monster::home_left, collapse_optional),
        attr("alert_phase", &Monster::alert_phase, optional),
        attr("alert_timer", &Monster::alert_timer, optional)
    ),
    init<&Monster::init>()
)

AYU_DESCRIBE(vf::MonsterMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &MonsterMind::target),
        attr("alert_sequence", &MonsterMind::alert_sequence, optional),
        attr("sight_range", &MonsterMind::sight_range),
        attr("attack_range", &MonsterMind::attack_range),
        attr("jump_range", &MonsterMind::jump_range, optional),
        attr("social_distance", &MonsterMind::social_distance),
        attr("ambush_distance", &MonsterMind::ambush_distance, optional)
    )
)
