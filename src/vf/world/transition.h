#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Transition : Resident {
    Hitbox hb;
    Room* target_room;
    Vec target_pos;
    Vec direction = {1, 0};
    Transition ();
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&);
};

} // vf
