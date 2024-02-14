#pragma once
#include "common.h"
#include "../game/room.h"

namespace vf {

struct Block {
    Vec pos;
    Hitbox hb;
};

struct Blocks : Resident {
    Frame* data;
    UniqueArray<Block> blocks;
    Blocks ();
    void init ();
    void Resident_draw () override;
};

} // vf
