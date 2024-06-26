#pragma once
#include "../game/common.h"

namespace vf {

 // Keep these in sync with the declarations at the top of *.ayu
namespace Z {
    constexpr float BG = -700;
    constexpr float DoorBehind = -600;
    constexpr float Hiding = -500;
    constexpr float Semisolid = -400;
    constexpr float Dead = -300;
    constexpr float DeadPlayer = -200;
    constexpr float Switch = -100;
    constexpr float Door = 0;
    constexpr float Blocks = 100;
    constexpr float Damage = 200;
    constexpr float Actor = 300;
    constexpr float Overlap = 400;
    constexpr float Projectile = 500;
    constexpr float Light = 600;
    constexpr float Symbol = 700;
    constexpr float ScreenEffect = 800;
    constexpr float Debug = 1000000;
    constexpr float HeadOffset = -10;
    constexpr float WingsOffset = 10;
    constexpr float DecalOffset = 20;
    constexpr float WeaponOffset = 30;
}

 // If a Resident has the given flag in its types, you can static_cast it to the
 // same-named type.
namespace Types {
    constexpr uint32 Walker = 1 << 0;
    constexpr uint32 Verdant = 1 << 1;
    constexpr uint32 Monster = 1 << 2;
    constexpr uint32 LoadingZone = 1 << 3;
    constexpr uint32 Semisolid = 1 << 4;
    constexpr uint32 Door = 1 << 5;
    constexpr uint32 Scenery = 1 << 6;
    constexpr uint32 Bug = 1 << 7;
    constexpr uint32 Indigo = 1 << 8;
}

 // Flags that determine which hitboxes interact with each other
namespace Layers {
    constexpr uint32 Walker_Solid = 1 << 0;
    constexpr uint32 Walker_Semisolid = 1 << 1;
    constexpr uint32 Walker_Walker = 1 << 2;
    constexpr uint32 Weapon_Solid = 1 << 3;
    constexpr uint32 Weapon_Walker = 1 << 4;
    constexpr uint32 LoadingZone_Verdant = 1 << 5;
    constexpr uint32 Switch_Weapon = 1 << 6;
     // Not all projectiles need to have all of these
    constexpr uint32 Projectile_Walker = 1 << 7;
    constexpr uint32 Projectile_Solid = 1 << 8;
    constexpr uint32 Projectile_Weapon = 1 << 9;
}

} // vf
