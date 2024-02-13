#pragma once
#include "../game/controls.h"
#include "walker.h"

namespace vf {

struct Monster : Walker {
    Vec home_pos = GNAN;
    std::optional<bool> home_left;
     // Don't show blood on the axe until pulling it back
    uint8 delay_weapon_layers = 1;
     // 0: Not alerted
     // 1: Before reacting
     // 2: Before moving
     // 3: Combat
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
     // Shift z behind scenery
    bool hiding = false;
    Monster ();
    void init (); 
     // Draw decals
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // Apply delayed weapon layers
    void Walker_draw_weapon (const Pose&) override;
    void Resident_on_exit () override;
};

struct MonsterMind : Mind {
    Walker* target = null;
    float sight_range;
    float attack_range;
    float jump_range = 0;
    float social_distance;
    float ambush_distance = GINF;
    uint8 alert_sequence [2] = {10, 20};
    Controls Mind_think (Resident&) override;
};

} // vf
