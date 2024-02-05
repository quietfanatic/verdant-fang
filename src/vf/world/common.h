#pragma once

namespace vf {

struct Block;
struct Fang;
struct Player;
struct Scenery;
struct Walker;

enum Layers : uint32 {
    Walker_Block = 1 << 0,
    Fang_Block = 1 << 1,
};

} // vf
