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
     // Keep revive counters separate from anim counters because with force
     // restart and limbo animations, you can revive in any state.
    uint8 revive_phase = 0;
    uint8 revive_timer = 0;

    Vec captured_start_pos;
    Vec limb_pos [4];

    Verdant ();

     // Relative to self.pos
    Vec visual_center ();

    void go_to_limbo ();

     // For handling custom hit animations
    WalkerBusiness Walker_business () override;
     // For moving during captured cutscene
    void Walker_move (const Controls&) override;
     // Allow getting crushed by door
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
     // Set decals on victim
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // For drawing damage forward frames
    Pose Walker_pose () override;
     // For handling extra drawing during cutscenes
    void Walker_draw_weapon (const Pose&) override;
     // Reset decals
    void Resident_on_exit () override;
};

Verdant* find_verdant ();

} // vf
