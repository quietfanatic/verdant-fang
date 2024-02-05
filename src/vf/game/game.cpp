#include "game.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../../dirt/iri/iri.h"
#include "../../dirt/control/input.h"
#include "camera.h"
#include "room.h"
#include "settings.h"
#include "state.h"

namespace vf {

static void on_draw (Game& game) {
    begin_camera();
    if (game.current_room) game.current_room->draw();
    end_camera();
    SDL_GL_SwapWindow(game.window);
}

static bool on_event (Game& game, SDL_Event* event) {
    switch (event->type) {
        case SDL_KEYDOWN: {
            SDL_ShowCursor(SDL_DISABLE);
            auto input = control::input_from_event(event);
            for (auto& binding : game.settings().commands) {
                if (input_matches_binding(input, binding.input)) {
                    if (binding.command) binding.command();
                    return true;
                }
            }
            return false;
        }
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEWHEEL: {
            SDL_ShowCursor(SDL_ENABLE);
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
    settings_res(iri::constant("data:/settings.ayu")),
    state_res(iri::constant("data:/state.ayu"))
{
    expect(!current_game);
    current_game = this;
    if (ayu::source_exists(settings_res->name())) {
        ayu::load(settings_res);
    }
    else {
        auto initial = ayu::SharedResource(
            iri::constant("res:/vf/game/initial-settings.ayu")
        );
        ayu::load(initial);
        ayu::rename(initial, settings_res);
        ayu::save(settings_res);
    }
    if (ayu::source_exists(state_res->name())) {
        ayu::load(state_res);
    }
    else {
        ayu::SharedResource world (iri::constant("res:/vf/world/world.ayu"));
        state_res->set_value(ayu::Dynamic::make<State>());
        state_res->get_value().as<State>().world = move(
            world->value().as<ayu::Document>()
        );
        ayu::force_unload(world);
         // Don't save state yet
    }
    current_room = state_res["world"]["start"][1];
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
#include <filesystem>
#include "../../dirt/tap/tap.h"
#include "../world/player.h"

tap::TestSet tests ("vf/game", []{
    using namespace control;
    using namespace tap;
    fs::remove(ayu::resource_filename(iri::constant("data:/state.ayu")));
    Game game;
    ok(the_player, "Initial state has player");
    int window_id = SDL_GetWindowID(game.window);
    send_input_as_event({.type = InputType::Key, .code = SDLK_ESCAPE}, window_id);
    game.loop.start();
    pass();
    done_testing();
});
#endif
