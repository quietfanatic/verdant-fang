#pragma once
#include "../../dirt/glow/image-texture.h"
#include "common.h"

namespace vf {

struct TextureLayer : glow::PixelTexture {
    float z_offset = 0;
};

struct LayeredTexture {
    UniqueArray<TextureLayer> layers;
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
    Vec scale = {1, 1},
    glow::RGBA8 tint = {},
    float fade = 1
);

inline void draw_layers (
    const Frame& frame,
    uint32 layers,
    Vec pos,
    float z = 0,
    Vec scale = {1, 1},
    glow::RGBA8 tint = {}
) {
    for (usize i = 0; i < 32 && i < frame.target->layers.size(); i++) {
        if (layers & (1 << i)) {
            draw_frame(frame, i, pos, z, scale, tint);
        }
    }
}

inline void draw_all_layers (
    const Frame& frame,
    Vec pos,
    float z = 0,
    Vec scale = {1, 1},
    glow::RGBA8 tint = {}
) {
    for (usize i = 0; i < frame.target->layers.size(); i++) {
        draw_frame(frame, i, pos, z, scale, tint);
    }
}

void draw_texture (
    const glow::Texture&,
    const Rect& world_rect,
    const Rect& tex_rect,
    float z = 0,
    glow::RGBA8 tint = {},
    float fade = 1
);

void draw_rectangle (
    const Rect& world_rect,
    glow::RGBA8 color,
    float z = 0
);

void commit_draws ();

} // vf
