#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Block : Resident {
    Hitbox hb;
    Block ();
};

} // vf
