#pragma once

#include "../dirt/ayu/resources/resource.h"
#include "../dirt/wind/active_loop.h"
#include "../dirt/wind/window.h"
#include "common.h"

namespace vf {

struct Game {
    wind::Window window;
    wind::ActiveLoop loop;
    ayu::SharedResource settings_res;
    ayu::SharedResource state;
    Settings& settings ();
    Room* current_room;
    Game ();
    ~Game ();
    void start ();
};

extern Game* current_game;

} // vf
