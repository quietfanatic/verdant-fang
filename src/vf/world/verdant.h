#pragma once
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Verdant : Walker {
    Verdant ();

    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&);
};

} // vf
