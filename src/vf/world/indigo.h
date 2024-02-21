#pragma once
#include "../game/controls.h"
#include "switch.h"
#include "walker.h"

namespace vf {
struct Verdant;

namespace CapturePhase {
    constexpr uint8 Moving = 0;
    constexpr uint8 Waiting = 1;
    constexpr uint8 SpearTaken = 2;
    constexpr uint8 SpearRotate0 = 3;
    constexpr uint8 SpearRotate1 = 4;
    constexpr uint8 SpearWaiting = 5;
    constexpr uint8 SpearBroken = 6;
    constexpr uint8 SpearBrokenGlow = 7;
    constexpr uint8 SnakeBroken = 8;
    constexpr uint8 SnakeFall = 9;
    constexpr uint8 SnakeFloor0 = 10;
    constexpr uint8 SnakeFloor1 = 11;
    constexpr uint8 SnakeFloor2 = 12;
    constexpr uint8 SnakeFloor3 = 13;
    constexpr uint8 LimbDetach0 = 14;
    constexpr uint8 LimbDetach1 = 15;
    constexpr uint8 LimbDetach2 = 16;
    constexpr uint8 LimbDetach3 = 17;
    constexpr uint8 GoodbyeLimbs = 18;
    constexpr uint8 Falling = 19;  // No timer, ends on hitting floor
    constexpr uint8 Floor = 20;
    constexpr uint8 N_Phases = 21;
};
namespace CP = CapturePhase;

struct IndigoData : WalkerData {
    Vec stolen_limb_offsets [4];
};

struct Indigo : Walker {
    uint8 alert_phase = 0;
    uint8 alert_timer = 0;
    Vec stolen_limb_initial_pos [4];
    Indigo ();
    WalkerBusiness Walker_business () override;
};

struct IndigoMind : Mind {
    Verdant* target;
    float sight_range;
    Activatable* activate_on_sight = null;
    Controls Mind_think (Resident&) override;
};

} // vf
