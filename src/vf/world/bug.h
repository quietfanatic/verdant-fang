#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Bug : Walker {
    Vec home_pos = GNAN;
    std::optional<bool> home_left;
    uint8 wings_timer = 0;
     // 0: Nothing suspicious
     // 1: Noticed, not reacted
     // 2: Combat
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
    Vec roam_pos;
     // Counts down
    uint32 roam_timer = 0;
    Bug ();
    void init ();
     // Animate wings
    void Resident_before_step () override;
    Pose Walker_pose () override;
    void Resident_on_exit () override;
};

struct BugMind : Mind {
    Walker* target = null;
     // Unlike Monster, this is a 2D distance
    float sight_range;
     // Area the bug thinks its tail hitbox covers.  Does not include target's
     // hitbox (that's added in calculations later).
    Rect attack_area;
    uint8 alert_sequence = 10;
     // Where the bug can roam, in absolute room coordinates
    Rect roam_territory;
    IRange roam_interval;
     // How strict the bug adheres to its current roam position
    float roam_tolerance = 3;
     // Back up if we're too close to target
    float personal_space;
    Controls Mind_think (Resident&) override;
};

} // vf
