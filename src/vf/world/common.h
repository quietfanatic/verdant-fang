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
    Walker_Walker = 1 << 1,
    Weapon_Block = 1 << 2,
    Weapon_Walker = 1 << 3,
    Monster_Monster = 1 << 4,
};
} using _::Layers;

} // vf
