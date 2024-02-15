#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct LoadingZone : Resident {
    Hitbox hb;
    Room* target_room;
    Vec target_pos;
    Vec direction = {1, 0};
     // Trigger a checkpoint if frustration is at most this value.
    uint8 checkpoint_level = 0;
    LoadingZone ();
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&);
};

} // vf
