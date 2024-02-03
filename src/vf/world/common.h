#pragma once

namespace vf {

struct Block;
struct Player;
struct Scenery;

enum Layers : uint32 {
    Player_Block = 1 << 0,
};

} // vf
