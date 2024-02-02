#pragma once
#include "common.h"

namespace glow { struct Texture; }

namespace vf {

struct Frame {
    Vec offset;
    Rect bounds;
};

void draw_frame (Vec, const Frame&, const glow::Texture&, BVec flip = {});

} // vf
