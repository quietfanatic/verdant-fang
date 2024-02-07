#pragma once
#include "../../dirt/glow/texture.h"
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Scenery : Resident {
    Scenery ();
    Rect bounds;
    glow::Texture* tex;
    void Resident_draw () override;
};

}
