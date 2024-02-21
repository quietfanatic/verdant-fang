#pragma once

#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/wind/active_loop.h"
#include "../../dirt/wind/window.h"
#include "common.h"

namespace vf {
struct Menu;

struct Game {
    wind::Window window;
    wind::ActiveLoop loop;
    UniqueArray<Menu*> menus;
    Menu* pause_menu;
    ayu::SharedResource settings_res;
    Settings& settings ();
    ayu::SharedResource options_res;
    Options& options ();
    ayu::SharedResource state_res;
    State& state ();
    Game ();
    ~Game ();
    void start ();
};

extern Game* current_game;

} // vf
