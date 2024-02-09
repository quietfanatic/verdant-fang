#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Monster : Walker {
    Vec home_pos = GNAN;
    std::optional<bool> home_left;
    Monster ();
    void init ();
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
    void Resident_on_exit () override;
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
