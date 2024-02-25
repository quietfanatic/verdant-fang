#pragma once
#include "../game/room.h"
#include "common.h"
#include "switch.h"

namespace vf {

enum class DoorState {
    Closed,
    Stuck,
    Open
};

struct DoorData;
struct Door : Resident, Activatable {
    Hitbox hb;
    DoorData* data;
    Vec closed_pos = GNAN;
    Vec stuck_pos = GNAN;
    Vec open_pos = GNAN;
    DoorState state = DoorState::Closed;
    DoorState closed_activate = DoorState::Open;
    DoorState stuck_activate = DoorState::Open;
    DoorState open_activate = DoorState::Closed;
    uint8 open_after = 0;
    bool crush = false;
    bool troll = false;
    float troll_dist = 16;
    Door* no_troll_unless_open = null;
    void init (); // Sets closed and open pos if they're not set
     // Sets state and plays appropriate sound
    void set_state (DoorState);
    void Resident_before_step () override;
    void Resident_draw () override;
    void Resident_on_exit () override;
    void Activatable_activate () override;
};

} // vf
