#pragma once

#include "../dirt/ayu/resources/resource.h"
#include "../dirt/wind/active_loop.h"
#include "../dirt/wind/window.h"
#include "common.h"

namespace vf {

struct Game {
    wind::Window window;
    wind::ActiveLoop loop;
    ayu::SharedResource state;
    Game ();
};

} // vf
