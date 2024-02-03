#pragma once
#include "common.h"

namespace glow { struct Texture; }

namespace vf {

struct Frame {
    Vec offset;
    Rect bounds;
};

void draw_frame (Vec, const Frame&, const glow::Texture&, Vec scale = {1, 1});

} // vf
