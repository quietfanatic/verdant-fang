#pragma once
#include "../../dirt/glow/texture.h"
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Scenery : Resident {
    Scenery ();
    glow::Texture* tex;
    Rect bounds;
    void Resident_draw () override;
};

}
