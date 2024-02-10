#pragma once
#include "common.h"
namespace glow { struct PixelTexture; struct Texture; }

namespace vf {

struct LayeredTexture {
    UniqueArray<glow::PixelTexture> layers;
     // If a frame is given with an offset that doesn't have any non-transparent
     // pixels nearby, search in this direction for non-transparent pixels
     // (default up).
    IVec search_direction = {0, 1};
};

struct Frame {
    LayeredTexture* target;
    IVec offset;
     // If bounds is not specified ([0 0 0 0]), searches the target textures for
     // a rectangle surrounded by transparent pixels.
    IRect bounds;
    void init ();
};

void draw_frame (
    const Frame&,
    uint32 layer,
    Vec pos,
    float z = 0,
    Vec scale = {1, 1}
);

void draw_texture (
    const glow::Texture&,
    const Rect& world_rect,
    const Rect& tex_rect,
    float z = 0
);

void draw_frames ();

} // vf
