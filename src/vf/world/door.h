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
    uint8 close_after = 0;
    bool open_from_left = false;
    bool open_from_right = false;
    bool crush = false;
    bool troll = false;
    float detection_dist = 20;
    Door* no_troll_unless_open = null;
    Door* close_after_these [2] = {null, null};
    void init (); // Sets closed and open pos if they're not set
     // Sets state and plays appropriate sound
    void set_state (DoorState);
    void Resident_before_step () override;
    void Resident_draw () override;
     // Apply open_from_left|right
    void Resident_on_enter () override;
    void Resident_on_exit () override;
    void Activatable_activate () override;
};

} // vf
