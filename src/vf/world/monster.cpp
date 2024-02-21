#include "monster.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "math.h"
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
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    float victim_top = victim.pos.y + victim.damage_hb.box.t;
    float weapon_bottom = pos.y + weapon_offset.y +
        data->poses->hit[0].weapon->hitbox.b;
    bool high = victim_top - weapon_bottom < 17;
    uint8 decal_i = high ? 3 : 1;
    auto& victim_body = *victim.data->poses->damage.body;
    Vec decal_pos = victim.pos + victim.left_flip(victim_body.decals[decal_i]);
    float weapon_tip = pos.x + left_flip(
        weapon_offset.x + data->poses->hit[0].weapon->hitbox.r
    );
    float depth = left_flip(weapon_tip - decal_pos.x);

    victim.decal_type = high ? DecalType::SlashHigh : DecalType::SlashLow;
    victim.decal_index = decal_i;
    if (high || depth > 4) pending_weapon_layers = 0x5;
    else if (pending_weapon_layers == 0x1) pending_weapon_layers = 0x3;
    if (depth > 6) {
        victim.pos.x += left_flip(depth - 6);
    }
    else if (depth < 1) {
        pos.x += left_flip(1 - depth);
    }
    Walker::Walker_on_hit(hb, victim, o_hb);
}

Pose Monster::Walker_pose () {
    if (hide_phase >= 1 && hide_phase <= 3) {
        Pose r = Walker::Walker_pose();
        r.z = Z::Hiding;
        return r;
    }
    else return Walker::Walker_pose();
}

void Monster::Walker_draw_weapon (const Pose& pose) {
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
        set_state(WS::Neutral);
        pos = home_pos;
        vel = {0, 0};
        walk_start_x = pos.x;
        left = *home_left;
        alert_phase = 0;
        alert_timer = 0;
        hide_phase = 0;
        invincible = false;
    }
    Walker::Resident_on_exit();
}

static float predict (Walker& self, Walker& target, float self_acc, float time) {
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
    self.invincible = false;

     // Calculcate some distances
    float dist = target->pos.x - self.pos.x;
     // Predict where target will be when attack hitbox comes out.
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

    next_alert_phase:
    if (self.alert_phase == 0) {
        if (length(dist) < sight_range) {
            self.alert_phase = 1;
            goto next_alert_phase;
        }
    }
    else if (self.alert_phase == 1) {
        if (self.alert_timer >= alert_sequence[0]) {
            self.alert_phase = 2;
            self.alert_timer = 0;
            goto next_alert_phase;
        }
        else {
            self.alert_timer += 1;
             // Do nothing, haven't reacted yet
        }
    }
    else if (self.alert_phase == 2) {
        if (self.alert_timer == 0) {
             // Alert other monsters
            for (auto r : self.room->residents) {
                if (!(r->types & Types::Monster)) continue;
                auto& fren = static_cast<Monster&>(*r);
                if (fren.state == WS::Dead) continue;
                if (fren.alert_phase == 0) fren.alert_phase = 1;
            }
        }
        if (self.alert_timer >= alert_sequence[1]) {
            self.alert_phase = 3;
            self.alert_timer = 0;
            goto next_alert_phase;
        }
        else {
            self.alert_timer += 1;
             // Look at target
            if (dist < 0) r[backward] = 1;
        }
    }
    else if (self.alert_phase == 3) {
        if (defined(hiding_spot)) {
            next_hide_phase:
            if (self.hide_phase == 0) {
                if (self.pos.x < hiding_spot + 24) {
                    r[Control::Right] = 1;
                    return r;
                }
                else {
                    self.hide_phase = 1;
                    goto next_hide_phase;
                }
            }
            else if (self.hide_phase == 1) {
                if (self.pos.x > hiding_spot) {
                    r[Control::Left] = 1;
                    self.invincible = true;
                    return r;
                }
                else {
                    self.hide_phase = 2;
                    goto next_hide_phase;
                }
            }
            else if (self.hide_phase == 2) {
                if (self.left) {
                    r[Control::Right] = 1;
                }
                else if (dist > 48) {
                     // Don't come out until you're the only one left
                    bool others = false;
                    for (auto r : self.room->residents) {
                        if (!(r->types & Types::Monster)) continue;
                        auto& m = static_cast<Monster&>(*r);
                        if (&m == &self || m.state == WS::Dead) continue;
                        others = true; break;
                    }
                    if (!others) {
                        self.hide_phase = 3;
                        goto next_hide_phase;
                    }
                }
                self.invincible = true;
                return r;
            }
            else if (self.hide_phase == 3) {
                if (self.pos.x > hiding_spot + 20) {
                    self.hide_phase = 4;
                }
            }
        }
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
        for (auto other : self.room->residents) {
            if (other == &s || !(other->types & Types::Monster)) continue;
            auto& fren = static_cast<Monster&>(*other);
            if (fren.state == WS::Dead) continue; // :(
            if (fren.hide_phase == 1 || fren.hide_phase == 2) continue;
            auto dist = self.left_flip(fren.pos.x - self.pos.x);
            if (dist > 0 && dist < social_distance) {
                if (jump_dist && fren.left != self.left) {
                    r[Control::Jump] = 1;
                }
                else {
                    r[Control::Jump] = 0;
                    r[forward] = 0;
                }
                break;
            }
        }
         // If we're behind scenery, keep going right no matter what
        if (self.hide_phase == 3) {
            r[Control::Right] = 1;
            r[Control::Left] = 0;
        }
    }
    else never();
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Monster,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("home_pos", &Monster::home_pos, optional),
        attr("home_left", &Monster::home_left, collapse_optional),
        attr("alert_phase", &Monster::alert_phase, optional),
        attr("alert_timer", &Monster::alert_timer, optional),
        attr("hide_phase", &Monster::hide_phase, optional)
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
        attr("hiding_spot", &MonsterMind::hiding_spot, optional)
    )
)
