#pragma once
#include "common.h"
#include "block.h"

namespace vf {

struct Falling {
    Rect body;  // relative to pos
    Vec pos;
    Vec vel;
    Block* floor;
};

} // vf
