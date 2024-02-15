#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Bug : Walker {
    uint8 wings_timer = 0;
     // Animate wings
    void Resident_before_step () override;
    Pose Walker_pose () override;
};

struct BugMind : Mind {
    Walker* target = null;
    float sight_range;
    float attack_range;
    Controls Mind_think (Resident&) override;
};

} // vf
