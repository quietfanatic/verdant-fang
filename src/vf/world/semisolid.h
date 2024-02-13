#include "common.h"
#include "../game/room.h"

namespace vf {

struct Semisolid : Resident {
    Frame* data;
    Hitbox hb;
    void init ();
    void Resident_draw () override;
};

} // vf
