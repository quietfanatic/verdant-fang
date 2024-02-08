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
    victim.decal_type = DecalType::Slash;
    victim.decal_index = 1;
    Walker::Walker_on_hit(hb, victim, o_hb);
}

Controls MonsterMind::Mind_think (Resident& s) {
    Controls r {};
    if (!(s.types & Types::Monster)) return r;
    auto& self = static_cast<Monster&>(s);
    if (!target ||
        target->state == WS::Damage ||
        target->state == WS::Dead
    ) return r;
    auto dist = target->pos.x - self.pos.x;
     // Work with right-facing coordinates
    if (self.left) dist = -dist;
    auto forward = self.left ? Control::Left : Control::Right;
    auto backward = self.left ? Control::Right : Control::Left;

    if (dist < 0) {
        r[backward] = 1;
    }
    else if (dist < attack_dist) {
         // Don't hold preattack pose
        if (self.state != WS::Attack || self.anim_phase >= 3) {
            r[Control::Attack] = 1;
        }
    }
    else if (dist < jump_dist) {
        r[forward] = 1;
        r[Control::Jump] = 1;
    }
    else if (dist < sight_dist) {
        r[forward] = 1;
    }

    for (auto& other : self.room->residents) {
        if (&other == &s || !(other.types & Types::Monster)) continue;
        auto& fren = static_cast<Monster&>(other);
        if (fren.state == WS::Dead || fren.state == WS::Damage) continue; // :(
        auto dist = fren.pos.x - self.pos.x;
        if (self.left) dist = -dist;
        if (dist > 0 && dist < social_dist) {
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
        attr("sight_dist", &MonsterMind::sight_dist),
        attr("attack_dist", &MonsterMind::attack_dist),
        attr("jump_dist", &MonsterMind::jump_dist, optional),
        attr("social_dist", &MonsterMind::social_dist)
    )
)
