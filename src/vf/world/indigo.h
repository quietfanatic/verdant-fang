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
    constexpr WalkerState Eaten = WS::Custom + 3;
    static_assert(Capturing == 6);
};
namespace IS = IndigoState;

namespace CapturingPhase {
    constexpr uint8 MoveTargetPre = 0;
    constexpr uint8 MoveTarget = 1;
    constexpr uint8 MoveTargetPost = 2;
    constexpr uint8 MoveTargetWait = 3;
    constexpr uint8 MoveWeapon = 4;
    constexpr uint8 HaveWeapon = 5;
    constexpr uint8 BreakWeaponPre = 6;
    constexpr uint8 BreakWeapon = 7;
    constexpr uint8 BreakWeaponPost = 8;
    constexpr uint8 BreakWeaponWait = 9;
    constexpr uint8 DetachLimbsPre = 10;
    constexpr uint8 DetachLimb0 = 11;
    constexpr uint8 DetachLimb1 = 12;
    constexpr uint8 DetachLimb2 = 13;
    constexpr uint8 DetachLimb3 = 14;
    constexpr uint8 TakeLimbs = 15;
    constexpr uint8 Leave = 16;
    constexpr uint8 CloseDoor = 17;
    constexpr uint8 N_Phases = 18;
};
namespace CP = CapturingPhase;

struct IndigoPoses : WalkerPoses {
    Pose capturing [CP::N_Phases];
    Pose bed [2];
    Frame* glasses;
    Pose bit [7];
    Pose eaten [34];
};

struct IndigoData : WalkerData {
    Vec capture_target_pos;
    Vec capture_weapon_pos;
    Vec capture_limb_offsets [4];
    uint8 capture_limb_order [4];
    uint8 capturing_sequence [CP::N_Phases];
    uint8 fingering_cycle [2] = {0, 0};
    uint8 bed_cycle [2];
    uint8 bed_use_limb;
    uint8 bit_sequence [7];
};

struct Indigo : Walker {
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
    Vec capture_initial_pos = GNAN;
    Vec home_pos = GNAN;
     // Will be activated when I notice Verdant
    Door* front_door;
     // Will be activated twice when I leave (once to open, once to exit)
    Door* back_door;
     // Go here after CP::CloseDoor.
    Room* bedroom;
    Vec bed_pos;
    Vec glasses_pos = GNAN;
    Verdant* verdant = null;

    Indigo ();
    void init ();
    WalkerBusiness Walker_business () override;
     // Disable damage hitbox in most states
    void Walker_set_hitboxes () override;
    Pose Walker_pose () override;
     // For drawing glasses separately
    void Walker_draw_weapon (const Pose&) override;
};

struct IndigoMind : Mind {
    Verdant* target;
    float sight_range;
    Vec goal_tolerance = {2, 4};
    Controls Mind_think (Resident&) override;
};

Indigo* find_indigo ();

} // vf
