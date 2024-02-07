#pragma once
#include "common.h"
#include "../game/room.h"

namespace vf {

struct Block : Hitbox { };

struct Blocks : Resident {
    UniqueArray<Block> blocks;
    Blocks ();
    void init ();
    void Resident_draw () override;
};

} // vf
