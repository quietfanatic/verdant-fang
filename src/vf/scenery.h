#pragma once
#include "../dirt/glow/texture.h"
#include "room.h"

namespace vf {

struct Scenery : Resident {
    glow::Texture* tex;
    void Resident_draw () override;
};

}
