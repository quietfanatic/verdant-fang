#pragma once
#include "../game/common.h"

namespace vf {

namespace Z {
    constexpr float BG = -600;
    constexpr float DoorBehind = -500;
    constexpr float Hiding = -400;
    constexpr float Semisolid = -300;
    constexpr float Dead = -200;
    constexpr float Door = -100;
    constexpr float Blocks = 0;
    constexpr float Switch = 100;
    constexpr float Damage = 200;
    constexpr float Actor = 300;
    constexpr float Overlap = 400;
    constexpr float HeadOffset = -10;
    constexpr float DecalOffset = 10;
    constexpr float WeaponOffset = 20;
}

namespace Types {
    constexpr uint32 Walker = 1 << 0;
    constexpr uint32 Verdant = 1 << 1;
    constexpr uint32 Monster = 1 << 2;
    constexpr uint32 LoadingZone = 1 << 3;
    constexpr uint32 Semisolid = 1 << 4;
    constexpr uint32 Door = 1 << 5;
    constexpr uint32 Scenery = 1 << 6;
}

namespace Layers {
    constexpr uint32 Walker_Solid = 1 << 0;
    constexpr uint32 Walker_Semisolid = 1 << 1;
    constexpr uint32 Walker_Walker = 1 << 2;
    constexpr uint32 Weapon_Solid = 1 << 3;
    constexpr uint32 Weapon_Walker = 1 << 4;
    constexpr uint32 LoadingZone_Verdant = 1 << 5;
    constexpr uint32 Switch_Weapon = 1 << 6;
}

} // vf
