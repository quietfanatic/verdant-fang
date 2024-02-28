#pragma once

#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/wind/active_loop.h"
#include "../../dirt/wind/window.h"
#include "common.h"
#include "controls.h"

namespace vf {
struct Menu;
struct OpenMenu;

struct Game {
    wind::Window window;
    wind::ActiveLoop loop;
    UniqueArray<OpenMenu> menus;
    Menu* start_menu;
    Menu* pause_menu;
    Menu* options_menu;
    Menu* end_menu;
    Controls menu_controls = {};
    bool turbo = false;
    ayu::SharedResource settings_res;
    Settings& settings ();
    ayu::SharedResource options_res;
    Options& options ();
    ayu::SharedResource state_res;
    State& state ();
    Game ();
    ~Game ();
    void start ();
    void reset ();
    [[noreturn]] void suspend ();
};

extern Game* current_game;

} // vf
