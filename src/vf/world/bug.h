#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

namespace BugState {
    constexpr WalkerState Spit = WS::Custom + 0;
}
namespace BS = BugState;

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
     // These count down
    uint32 roam_timer = 0;
    uint32 spit_timer = 0;
     // 0 = nonexistent
     // 1 = moving
     // 2 = splashing
    uint8 projectile_state = 0;
    uint32 projectile_timer = 0;
    Vec projectile_pos = GNAN;
    Vec projectile_vel = GNAN;
    Hitbox projectile_hb;
    Bug ();
    void init ();
     // Handle BS::Spit
    WalkerBusiness Walker_business () override;
     // Spit
    void Walker_move (const Controls&) override;
     // Animate wings
    void Resident_before_step () override;
     // Projectile collision
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
     // Apply decals
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // Animate wings
    Pose Walker_pose () override;
     // Draw projectile
    void Resident_draw () override;
    void Resident_on_exit () override;
};

struct BugMind : Mind {
    Walker* target = null;
     // Unlike Monster, this is a 2D distance
    float sight_range;
    uint8 alert_sequence = 10;
     // Back up if we're too close to target
    float personal_space = 20;
     // Area the bug thinks its tail hitbox covers.  Does not include target's
     // hitbox (that's added in calculations later).
    Rect attack_area;
     // Where the bug can roam, in absolute room coordinates
    Rect roam_territory;
     // Min and max time to pick a new roam location
    IRange roam_interval;
     // How strict the bug adheres to its current roam position
    float roam_tolerance = 0;
     // Min and max time to spit.
    IRange spit_interval;
     // Don't spit unless speed is less than this
    float spit_stability_requirement = 3;
    Controls Mind_think (Resident&) override;
};

} // vf
