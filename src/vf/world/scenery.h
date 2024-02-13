#pragma once
#include "../../dirt/glow/texture.h"
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Scenery : Resident {
    Frame* data;
    float z = Z::BG;
    Vec scale = {1, 1};
    Scenery ();
    void Resident_draw () override;
};

}
