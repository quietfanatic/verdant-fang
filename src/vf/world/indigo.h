#pragma once
#include "../game/controls.h"
#include "switch.h"
#include "walker.h"

namespace vf {
struct Door;
struct Verdant;

namespace IndigoState {
    constexpr WalkerState Capturing = WS::Custom + 0;
    constexpr WalkerState Bed = WS::Custom + 1;
    constexpr WalkerState Bit = WS::Custom + 2;
    constexpr WalkerState CapturingSnake = WS::Custom + 4;
    constexpr WalkerState Eaten = WS::Custom + 3;
    static_assert(Capturing == 6);
};
namespace IS = IndigoState;

namespace CapturingPhase {
    constexpr uint8 MoveTargetPre = 0;
    constexpr uint8 TrackTarget = 1;
    constexpr uint8 MoveTarget = 2;
    constexpr uint8 MoveTargetPost = 3;
    constexpr uint8 MoveTargetWait = 4;
    constexpr uint8 MoveWeapon = 5;
    constexpr uint8 HaveWeapon = 6;
    constexpr uint8 BreakWeaponPre = 7;
    constexpr uint8 BreakWeapon = 8;
    constexpr uint8 BreakWeaponPost = 9;
    constexpr uint8 BreakWeaponWait = 10;
    constexpr uint8 DetachLimbsPre = 11;
    constexpr uint8 DetachLimb0 = 12;
    constexpr uint8 DetachLimb1 = 13;
    constexpr uint8 DetachLimb2 = 14;
    constexpr uint8 DetachLimb3 = 15;
    constexpr uint8 TakeLimbs = 16;
    constexpr uint8 Leave = 17;
    constexpr uint8 CloseDoor = 18;
    constexpr uint8 N_Phases = 19;
};
namespace CP = CapturingPhase;

struct IndigoPoses : WalkerPoses {
    Frame* bubble [6];
    Frame* bubble_pop [3];
    Frame* bubble_boom [3];
    Pose capturing [CP::N_Phases];
    Pose bed [2];
    Frame* glasses;
    Pose bit [9];
    Pose capturing_snake [6];
    Frame* hat;
    Pose eaten [34];
};

struct IndigoData : WalkerData {
    float bubble_radius;
    float bubble_speeds [6];
    uint8 bubble_sequence [6];
    uint8 bubble_pop_sequence [3];
    float dodge_speed;
    Vec capture_target_pos;
    Vec capture_weapon_pos;
    Vec capture_limb_offsets [4];
    uint8 capture_limb_order [4];
    uint8 capturing_sequence [CP::N_Phases];
    uint8 fingering_cycle [2] = {0, 0};
    uint8 bed_cycle [2];
    uint8 bed_use_limb;
    uint8 bit_sequence [9];
     // 0 = preattack
     // 1 = track snake
     // 2 = move snake
     // 3 = postattack
     // 4 = waiting
     // 5 = preattack (time limit ignored, but this array entry used to
     // calculate sequence length)
    uint8 capturing_snake_sequence [6];
    Sound* bubble_pop_sound = null;
    Sound* dodge_sound = null;
};

struct IndigoBubble {
     // 0 = nonexistent
     // 1 = active
     // 2 = blowing up
    uint8 state = 0;
    uint8 phase = 0;
    uint32 timer = 0;
     // Absolute
    Vec pos = GNAN;
    float direction;
    float curve;
     // Relative to my pos
    Hitbox hb;
};

struct Indigo : Walker {
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
    uint8 attack_count = 0;
    uint8 home_index = 0;
    Vec home_pos [2] = {Vec(230, 72), Vec(90, 72)};
    IndigoBubble bubbles [6];
    Vec capture_initial_pos = GNAN;
     // Will be activated when I notice Verdant
    Door* front_door;
     // Will be activated twice when I leave (once to open, once to exit)
    Door* back_door;
     // Go here after CP::CloseDoor.
    Room* bedroom;
    Vec bed_pos;
    Vec glasses_pos = GNAN;
    Vec bedroom_limb_pos [4] = {GNAN, GNAN, GNAN, GNAN};
    Vec hat_pos = GNAN;
    Verdant* verdant = null;

    Indigo ();
    void emit_bubble (float angle);
    void go_to_bed ();
    WalkerBusiness Walker_business () override;
     // Create bubbles when attacking
    void Walker_move (const Controls&) override;
     // Disable damage hitbox in most states
    void Walker_set_hitboxes () override;
     // Bubble collision
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
    Pose Walker_pose () override;
     // For drawing glasses, hat, bubbles
    void Walker_draw_weapon (const Pose&) override;
};

struct IndigoMind : Mind {
    Verdant* target;
    float sight_range;
    Vec goal_tolerance = {2, 2};
    Controls Mind_think (Resident&) override;
};

Indigo* find_indigo ();

} // vf
