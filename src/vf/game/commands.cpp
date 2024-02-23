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

void pause_or_unpause_ () {
    if (!current_game) return;
    if (current_game->menus &&
        current_game->menus[0].data == current_game->pause_menu
    ) {
        current_game->menus = {};
    }
    else {
        current_game->menus = {OpenMenu(current_game->pause_menu)};
    }
}
Command pause_or_unpause (pause_or_unpause_, "pause_or_unpause", "Pause game if playing game; Unpause game if paused");

void delete_state_ () {
    if (!current_game) return;
    ayu::remove_source(current_game->state_res->name());
}
Command delete_state (delete_state_, "delete_state", "Delete the saved game state");

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
void open_options_menu_ () {
    if (!current_game) return;
    if (current_game->menus &&
        current_game->menus.back().data == current_game->options_menu
    ) return;
    current_game->menus.push_back(OpenMenu(current_game->options_menu));
}
control::Command open_options_menu (open_options_menu_, "open_options_menu", "Open the options menu");

void close_menu_ () {
    if (!current_game || !current_game->menus) return;
    current_game->menus.pop_back();
}
control::Command close_menu (close_menu_, "close_menu", "Close the current menu");

} // vf
