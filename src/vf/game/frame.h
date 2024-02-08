#pragma once
#include "common.h"

namespace glow { struct Texture; }

namespace vf {

struct Frame {
    Vec offset;
    Rect bounds;
};

void draw_frame (
    Vec pos,
    const Frame&,
    const glow::Texture&,
    Vec scale = {1, 1},
    float z = 0
);

void draw_texture (
    const glow::Texture&,
    const Rect& world_rect,
    const Rect& tex_rect,
    float z = 0
);

void draw_frames ();

} // vf
