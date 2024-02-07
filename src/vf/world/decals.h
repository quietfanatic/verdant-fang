#pragma once
#include "../../dirt/glow/image-texture.h"
#include "../game/frame.h"
#include "common.h"

namespace vf {

enum class DecalType : uint8 {
    None,
    Stab,
};

constexpr uint8 max_decals = 4;

struct DecalData {
    glow::PixelTexture stab_tex;
    Frame stab_0 [3];
    Frame stab_1 [1];
    Frame stab_2 [7];
};

struct Walker;
struct Pose;
void draw_decals (const Walker&, const Pose&);

} // vf
