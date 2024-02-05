#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Monster : Walker {
    Monster ();
};

struct MonsterMind : Mind {
    float sight_dist;
    float stop_dist;
    Controls Mind_think (Resident& r) override;
};

} // vf
