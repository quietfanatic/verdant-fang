#pragma once

namespace vf {

struct Block;
struct Fang;
struct Player;
struct Scenery;

enum Layers : uint32 {
    Player_Block = 1 << 0,
    Fang_Block = 1 << 1,
};

} // vf
