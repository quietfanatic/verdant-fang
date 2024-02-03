#include "game.h"

#include <filesystem>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../dirt/iri/iri.h"
#include "../dirt/control/input.h"
#include "camera.h"
#include "room.h"
#include "settings.h"

namespace vf {

constexpr iri::IRI initial_settings_loc ("res:/vf/initial-settings.ayu");
constexpr iri::IRI settings_loc ("data:/settings.ayu");
constexpr iri::IRI initial_state_loc ("res:/vf/initial-state.ayu");
constexpr iri::IRI state_loc ("data:/state.ayu");

static void on_draw (Game& game) {
    begin_camera();
    if (game.current_room) game.current_room->draw();
    end_camera();
    SDL_GL_SwapWindow(game.window);
}

static bool on_event (Game& game, SDL_Event* event) {
    switch (event->type) {
        case SDL_KEYDOWN: {
            auto input = control::input_from_event(event);
            for (auto& binding : game.settings().commands) {
                if (input_matches_binding(input, binding.input)) {
                    if (binding.command) binding.command();
                    return true;
                }
            }
            return false;
        }
        case SDL_WINDOWEVENT: {
            if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                window_size_changed({event->window.data1, event->window.data2});
                return true;
            }
            else return false;
        }
        default: return false;
    }
}

Game::Game () :
    window("Verdant Fang (testing)", size(window_viewport)),
    loop{
        .on_event = [this](SDL_Event* e){ return on_event(*this, e); },
        .on_step = [this]{ if (current_room) current_room->step(); },
        .on_draw = [this]{ on_draw(*this); },
    },
    settings_res(settings_loc),
    state(state_loc)
{
    expect(!current_game);
    current_game = this;
    if (!ayu::source_exists(settings_loc)) {
        fs::copy_file(
            ayu::resource_filename(initial_settings_loc),
            ayu::resource_filename(settings_loc)
        );
    }
    if (!ayu::source_exists(state_loc)) {
        fs::copy_file(
            ayu::resource_filename(initial_state_loc),
            ayu::resource_filename(state_loc)
        );
    }
    ayu::load({settings_res, state});
    current_room = state["start"][1];
    expect(current_room->residents);
    current_room->enter();
}

Game::~Game () { current_game = null; }

Settings& Game::settings () {
    return settings_res->value().as_known<Settings>();
}

void Game::start () {
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_ShowWindow(window);
    SDL_GL_SetSwapInterval(1);
    glow::init();
    loop.start();
}

Game* current_game = null;

} using namespace vf;

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"
#include "player.h"

tap::TestSet tests ("vf/game", []{
    using namespace control;
    using namespace tap;
    fs::remove(ayu::resource_filename(state_loc));
    Game game;
    ok(the_player, "Initial state has player");
    int window_id = SDL_GetWindowID(game.window);
    send_input_as_event({.type = InputType::Key, .code = SDLK_ESCAPE}, window_id);
    game.loop.start();
    pass();
    done_testing();
});
#endif
