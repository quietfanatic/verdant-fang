#include <SDL2/SDL_video.h>
#include "../../dirt/control/command.h"
#include "../../dirt/glow/common.h"
#include "game.h"
#include "menu.h"
#include "settings.h"
#include "state.h"

namespace vf {
using namespace control;

void fullscreen_ () {
    if (!current_game) return;
    auto flags = SDL_GetWindowFlags(current_game->window);
    if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        flags = 0;
    }
    else if (current_game->settings().true_fullscreen) {
        int display = SDL_GetWindowDisplayIndex(current_game->window);
        glow::require_sdl(display >= 0);
        SDL_DisplayMode mode = {};
        mode.format = SDL_PIXELFORMAT_RGBX8888;
        mode.w = 1280;
        mode.h = 720;
        mode.refresh_rate = 60;
        glow::require_sdl(SDL_GetClosestDisplayMode(display, &mode, &mode));
        glow::require_sdl(!SDL_SetWindowDisplayMode(current_game->window, &mode));
        flags = SDL_WINDOW_FULLSCREEN;
    }
    else {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL_SetWindowFullscreen(current_game->window, flags);
}
Command fullscreen (fullscreen_, "fullscreen", "Toggle window fullscreen state");

void save_state_ () {
    if (!current_game) return;
    ayu::save(current_game->state_res);
}
Command save_state (save_state_, "save_state", "Save game state");

void load_state_ () {
    if (!current_game) return;
    if (!ayu::source_exists(current_game->state_res->name())) return;
    ayu::ResourceTransaction tr;
    ayu::reload(current_game->state_res);
}
Command load_state (load_state_, "load_state", "Load game state");

void delete_state_ () {
    if (!current_game) return;
    ayu::remove_source(current_game->state_res->name());
}
Command delete_state (delete_state_, "delete_state", "Delete the saved game state");

void reset_state_ () {
    if (!current_game) return;
     // Clear everything including transitions and stuff
    current_game->state_res->set_value(ayu::Dynamic::make<State>());
    auto& state = current_game->state_res->get_value().as<State>();
    state.load_initial();
}
Command reset_state (reset_state_, "reset_state", "Reset game state to initial state wthout affecting saved state.");

void exit_program_ () {
    exit(0);
}
Command exit_program (exit_program_, "exit_program", "Exit the program immediately regardless of whether a state is saved.");

void ignore_held_controls_ () {
    if (!current_game) return;
    auto& settings = current_game->settings();
    settings.disable_while_held = settings.read_controls();
}
Command ignore_held_controls (ignore_held_controls_, "ignore_held_controls", "If any controls are currently being held, ignore them until they are released.");

 // Currently having pointers inside commands is broken, so we can't have a
 // generic open_menu command that takes a Menu* as an argument.
void open_main_menu_ () {
    if (!current_game) return;
    if (current_game->menus &&
        current_game->menus.back().data == current_game->main_menu
    ) return;
    current_game->menus.emplace_back(current_game->main_menu);
}
control::Command open_main_menu (open_main_menu_, "open_main_menu", "Open the main menu");

void pause_or_unpause_ () {
    if (!current_game) return;
    if (!current_game->menus) {
        current_game->menus.emplace_back(current_game->pause_menu);
    }
    else for (auto om = current_game->menus.rbegin();
        om != current_game->menus.rend();
        om++
    ) {
        if (om->data->allow_pause) {
            current_game->menus.emplace_back(current_game->pause_menu);
            break;
        }
        if (om->data == current_game->pause_menu) {
            current_game->menus.shrink(&*om - current_game->menus.begin());
            break;
        }
    }
}
Command pause_or_unpause (pause_or_unpause_, "pause_or_unpause", "Pause game if playing game; Unpause game if paused");

void open_options_menu_ () {
    if (!current_game) return;
    if (current_game->menus &&
        current_game->menus.back().data == current_game->options_menu
    ) return;
    current_game->menus.emplace_back(current_game->options_menu);
}
control::Command open_options_menu (open_options_menu_, "open_options_menu", "Open the options menu");

void close_menu_ () {
    if (!current_game || !current_game->menus) return;
    current_game->menus.pop_back();
}
control::Command close_menu (close_menu_, "close_menu", "Close the current menu");

void close_all_menus_ () {
    if (!current_game) return;
    current_game->menus = {};
}
control::Command close_all_menus (close_all_menus_, "close_all_menus", "Close all menus");

void toggle_turbo_ () {
    if (!current_game) return;
    current_game->turbo = !current_game->turbo;
}
control::Command toggle_turbo (toggle_turbo_, "toggle_turbo", "Make game run at double speed");

} // vf
