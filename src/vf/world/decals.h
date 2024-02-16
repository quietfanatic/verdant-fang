#pragma once
#include "../../dirt/glow/image-texture.h"
#include "../game/render.h"
#include "common.h"

namespace vf {

enum class DecalType : uint8 {
    None,
    Stab,
    SlashLow,
    SlashHigh
};

constexpr uint8 max_decals = 4;

struct Decal {
    vf::LayeredTexture tex;
    Frame dir_0 [4];
    Frame dir_1 [1];
    Frame dir_2 [7];
};

struct Paralyze {
    vf::LayeredTexture tex;
    Frame symbol [2];
};

struct DecalData {
    Decal stab;
    Decal slash_low;
    Decal slash_high;
    Paralyze paralyze;
};

struct Walker;
struct Pose;
void draw_decal (Walker&, const Pose&);

} // vf
