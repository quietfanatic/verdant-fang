#pragma once
#include "../game/common.h"

namespace vf {

namespace Z {
    constexpr float BG = -100;
    constexpr float Dead = 0;
    constexpr float Blocks = 50;
    constexpr float Alive = 100;
}

namespace Types {
    constexpr uint32 Walker = 1 << 0;
    constexpr uint32 Verdant = 1 << 1;
    constexpr uint32 Monster = 1 << 2;
}

namespace Layers {
    constexpr uint32 Walker_Block = 1 << 0;
    constexpr uint32 Walker_Walker = 1 << 1;
    constexpr uint32 Weapon_Block = 1 << 2;
    constexpr uint32 Weapon_Walker = 1 << 3;
}

} // vf
