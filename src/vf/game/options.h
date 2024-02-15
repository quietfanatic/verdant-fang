#pragma once
#include "common.h"

namespace vf {

struct Options {
     // 1 = low, lots of checkpoints
     // 2 = medium, some checkpoints
     // 3 = high, no checkpoints
    uint8 frustration = 2;
};

} // vf
