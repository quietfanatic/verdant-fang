#include "common.h"
#include "../game/room.h"

namespace vf {

struct Block; // blocks.h

struct Semisolids : Resident {
    Frame* data;
    UniqueArray<Block> blocks;
    Semisolids ();
    void init ();
    void Resident_draw () override;
};

} // vf
