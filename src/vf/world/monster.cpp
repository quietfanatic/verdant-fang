#include "monster.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "verdant.h"

namespace vf {

Monster::Monster () {
    layers_1 |= Layers::Monster_Verdant;
}

Controls MonsterMind::Mind_think (Resident& s) {
    expect(s.layers_1 & Layers::Monster_Verdant);
    auto& self = static_cast<Monster&>(s);
    Controls r {};
    auto target = main_character->pos.x - self.pos.x;
    if (contains(Range(-sight_dist, -attack_dist), target)) {
        r[Control::Left] = 1;
    }
    else if (contains(Range(-attack_dist, attack_dist), target)) {
        if (target < 0 && !self.left) {
            r[Control::Left] = 1;
        }
        else if (target > 0 && self.left) {
            r[Control::Right] = 1;
        }
        if (self.state != WS::Attack || self.anim_phase >= 3) {
            r[Control::Attack] = 1;
        }
    }
    else if (contains(Range(attack_dist, sight_dist), target)) {
        r[Control::Right] = 1;
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
        attr("sight_dist", &MonsterMind::sight_dist),
        attr("attack_dist", &MonsterMind::attack_dist)
    )
)
