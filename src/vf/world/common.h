#pragma once

namespace vf {

struct Block;
struct Fang;
struct Player;
struct Scenery;
struct Walker;

namespace _ {
enum Types : uint32 {
    Walker = 1 << 0,
    Verdant = 1 << 1,
    Monster = 1 << 2,
};
} using _::Types;

namespace _ {
enum Layers : uint32 {
    Walker_Block = 1 << 0,
    Walker_Walker = 1 << 1,
    Weapon_Block = 1 << 2,
    Weapon_Walker = 1 << 3,
};
} using _::Layers;

} // vf
