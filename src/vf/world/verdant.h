#pragma once
#include "../game/controls.h"
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Verdant : Walker {
    bool damage_forward = false;
     // Total transformation time, for hair animation
    uint32 transform_timer = 0;
    Room* limbo = null;

    Verdant ();

     // For handling custom hit animations
    WalkerBusiness Walker_business () override;
     // Allow getting crushed by door
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
     // Set decals on victim
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // For drawing damage forward frames
    Pose Walker_pose () override;
     // For making Fang hover during transformation
    void Walker_draw_weapon (const Pose&) override;
     // Reset decals
    void Resident_on_exit () override;
};

Verdant* find_verdant ();

} // vf
