#pragma once
#include "../game/controls.h"
#include "switch.h"
#include "walker.h"

namespace vf {
struct Verdant;

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
