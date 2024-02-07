#pragma once
#include "../game/common.h"

namespace vf {

namespace Z {
    constexpr float BG = -300;
    constexpr float Dead = -200;
    constexpr float DeadDecal = -100;
    constexpr float Blocks = 0;
    constexpr float Damage = 100;
    constexpr float Alive = 200;
    constexpr float DamageOverlap = 300;
    constexpr float DamageDecal = 400;
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
