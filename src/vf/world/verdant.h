#pragma once
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Verdant : Walker {
    bool damage_forward = true;

    Verdant ();

     // For handling custom hit animations
    WalkerBusiness Walker_business () override;
     // Allow getting crushed by door
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
     // Set decals on victim
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // For drawing damage forward frames
    Pose Walker_pose () override;
};

} // vf
