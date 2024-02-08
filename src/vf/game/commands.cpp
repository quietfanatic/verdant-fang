#include <SDL2/SDL_video.h>
#include "../../dirt/control/command.h"
#include "../../dirt/glow/common.h"
#include "game.h"
#include "settings.h"

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
    ayu::ResourceTransaction tr;
    ayu::reload(current_game->state_res);
}
Command load_state (load_state_, "load_state", "Load game state");

} // vf
