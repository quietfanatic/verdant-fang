#pragma once

namespace vf {

struct Block;
struct Fang;
struct Player;
struct Scenery;
struct Walker;

namespace _ {
enum Layers : uint32 {
    Walker_Block = 1 << 0,
    Weapon_Block = 1 << 1,
    Weapon_Walker = 1 << 2,
    Monster_Monster = 1 << 3,
};
} using _::Layers;

} // vf
