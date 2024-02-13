#pragma once
#include "../game/room.h"
#include "common.h"
#include "switch.h"

namespace vf {

struct DoorData;
struct Door : Resident, Activatable {
    Hitbox hb;
    DoorData* data;
    Vec closed_pos = GNAN;
    Vec open_pos = GNAN;
    bool open = false;
    bool crush = false;
    void init (); // Sets closed and open pos if they're not set
    void Resident_before_step () override;
    void Resident_draw () override;
    void Resident_on_exit () override;
    void Activatable_activate () override;
};

} // vf
