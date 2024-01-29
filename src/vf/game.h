#pragma once

#include "../dirt/wind/active_loop.h"
#include "../dirt/wind/window.h"
#include "common.h"

namespace vf {

struct Game {
    wind::Window window;
    wind::ActiveLoop loop;
    Game ();
};

} // vf
