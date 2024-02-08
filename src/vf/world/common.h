#pragma once
#include "../game/common.h"

namespace vf {

namespace Z {
    constexpr float BG = -200;
    constexpr float Dead = -100;
    constexpr float Blocks = 0;
    constexpr float Damage = 100;
    constexpr float Actor = 200;
    constexpr float Overlap = 300;
    constexpr float HeadOffset = -10;
    constexpr float DecalOffset = 10;
    constexpr float WeaponOffset = 20;
}

namespace Types {
    constexpr uint32 Walker = 1 << 0;
    constexpr uint32 Verdant = 1 << 1;
    constexpr uint32 Monster = 1 << 2;
    constexpr uint32 Transition = 1 << 3;
}

namespace Layers {
    constexpr uint32 Walker_Block = 1 << 0;
    constexpr uint32 Walker_Walker = 1 << 1;
    constexpr uint32 Weapon_Block = 1 << 2;
    constexpr uint32 Weapon_Walker = 1 << 3;
    constexpr uint32 Transition_Verdant = 1 << 4;
}

} // vf
