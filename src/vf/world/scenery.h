#pragma once
#include "../../dirt/glow/texture.h"
#include "../game/room.h"

namespace vf {

struct Scenery : Resident {
    Scenery () { pos = {0, 0}; }
    glow::Texture* tex;
    void Resident_draw () override;
};

}
