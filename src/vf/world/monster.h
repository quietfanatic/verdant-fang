#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Monster : Walker {
    Monster ();
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
};

struct MonsterMind : Mind {
    Walker* target = null;
    float sight_range;
    float attack_range;
    float jump_range = 0;
    float social_distance;
    Controls Mind_think (Resident& r) override;
};

} // vf
