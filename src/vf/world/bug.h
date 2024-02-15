#pragma once
#include "walker.h"

namespace vf {

struct Bug : Walker {
    uint8 wings_timer = 0;
     // Animate wings
    void Resident_before_step () override;
    Pose Walker_pose () override;
};

} // vf
