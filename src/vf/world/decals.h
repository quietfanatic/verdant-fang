#pragma once
#include "../../dirt/glow/image-texture.h"
#include "../game/frame.h"
#include "common.h"

namespace vf {

enum class DecalType : uint8 {
    None,
    Stab,
    Slash,
};

constexpr uint8 max_decals = 4;

struct Decal {
    glow::PixelTexture tex;
    Frame dir_0 [3];
    Frame dir_1 [1];
    Frame dir_2 [7];
};

struct DecalData {
    Decal stab;
    Decal slash;
};

struct Walker;
struct Pose;
void draw_decal (const Walker&, const Pose&);

} // vf
