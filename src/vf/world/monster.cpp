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
    if (contains(Range(-sight_dist, -stop_dist), target)) {
        r[Control::Left] = 1;
    }
    else if (contains(Range(stop_dist, sight_dist), target)) {
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
        attr("stop_dist", &MonsterMind::stop_dist)
    )
)
