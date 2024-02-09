#include "monster.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "verdant.h"

namespace vf {

Monster::Monster () {
    types |= Types::Monster;
}

void Monster::Walker_on_hit (
    const Hitbox& hb, Walker& victim, const Hitbox& o_hb
) {
    victim.left = !left;
    bool high = pos.y - victim.pos.y > 8;
    uint8 decal_i = high ? 3 : 1;
    Vec weapon_offset = data->poses->attack[1].body->weapon;
    auto& victim_body = *victim.data->poses->damage[0].body;
    float weapon_tip = pos.x + left_flip(
        weapon_offset.x + data->poses->attack[1].weapon->hitbox.r
    );
    Vec decal_pos = victim.pos + victim.left_flip(victim_body.decals[decal_i]);
    float depth = left_flip(weapon_tip - decal_pos.x);
    ayu::dump(depth);
    victim.decal_type = high ? DecalType::SlashHigh : DecalType::SlashLow;
    victim.decal_index = decal_i;
    if (high || depth > 4) weapon_state = 2;
    else if (weapon_state < 1) weapon_state = 1;
    if (depth > 6) {
        victim.pos.x += left_flip(depth - 6);
    }
    Walker::Walker_on_hit(hb, victim, o_hb);
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
    auto& target_phys = target.data->phys;
    auto predicted_x = quadratic_until_stop(
        self.pos.x, self.vel.x,
        self.left ? -self_acc : self_acc,
        time
    );
     // Assume target will attack and decelerate.
    float target_dec =
        target.floor ? target_phys.coast_dec : target_phys.air_dec;
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
    if (!target ||
        target->state == WS::Damage ||
        target->state == WS::Dead
    ) return r;
    auto& phys = self.data->phys;

     // Calculcate some distances
    float dist = target->pos.x - self.pos.x;
     // Predict where target will be when attack animation finishes.
    float attack_dist = predict(
        self, *target,
        self.floor ? -phys.coast_dec : -phys.air_dec,
        phys.attack_sequence[0]
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

    if (dist < 0) {
        r[backward] = 1;
    }
    else if (attack_dist < attack_range) {
        if (
         // Don't attack too early when jumping unless the player is above.
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
    else if (dist < sight_range) {
        r[forward] = 1;
    }

    for (auto& other : self.room->residents) {
        if (&other == &s || !(other.types & Types::Monster)) continue;
        auto& fren = static_cast<Monster&>(other);
        if (fren.state == WS::Dead || fren.state == WS::Damage) continue; // :(
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
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Monster,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)

AYU_DESCRIBE(vf::MonsterMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &MonsterMind::target),
        attr("sight_range", &MonsterMind::sight_range),
        attr("attack_range", &MonsterMind::attack_range),
        attr("jump_range", &MonsterMind::jump_range, optional),
        attr("social_distance", &MonsterMind::social_distance)
    )
)
