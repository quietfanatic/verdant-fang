#pragma once
#include "../game/controls.h"
#include "switch.h"
#include "walker.h"

namespace vf {
struct Verdant;

namespace IndigoState {
    constexpr WalkerState Capturing = WS::Custom + 0;
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
    constexpr uint8 DetachLimb0 = 10;
    constexpr uint8 DetachLimb1 = 11;
    constexpr uint8 DetachLimb2 = 12;
    constexpr uint8 DetachLimb3 = 13;
    constexpr uint8 TakeLimbs = 14;
    constexpr uint8 Leave = 15;
    constexpr uint8 N_Phases = 16;
};
namespace CP = CapturingPhase;

struct IndigoPoses : WalkerPoses {
    Pose capturing [CP::N_Phases];
};

struct IndigoData : WalkerData {
    Vec capture_target_pos;
    Vec capture_weapon_pos;
    Vec capture_limb_offsets [4];
    uint8 capture_limb_order [4];
    uint8 capturing_sequence [CP::N_Phases];
};

struct Indigo : Walker {
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
    Vec capture_initial_pos = GNAN;
    Indigo ();
    WalkerBusiness Walker_business () override;
    Pose Walker_pose () override;
};

struct IndigoMind : Mind {
    Verdant* target;
    float sight_range;
    Activatable* activate_on_sight = null;
    Controls Mind_think (Resident&) override;
};

Indigo* find_indigo ();

} // vf
