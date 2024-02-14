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

static void on_draw (Game& game) {
    begin_camera();
    auto room = game.state().current_room;
    if (room) room->draw();
    end_camera();
    SDL_GL_SwapWindow(game.window);
}

Game::Game () :
    window("Verdant Fang (testing)", size(window_viewport)),
    loop{
        .on_event = [this](SDL_Event* e){ return on_event(*this, e); },
        .on_step = [this]{ state().step(); },
        .on_draw = [this]{ on_draw(*this); },
    },
    settings_res(iri::constant("data:/settings.ayu")),
    options_res(iri::constant("data:/options.ayu")),
    state_res(iri::constant("data:/state.ayu"))
{
    expect(!current_game);
    current_game = this;

    ayu::ResourceTransaction tr;

    if (ayu::source_exists(settings_res->name())) {
        ayu::load(settings_res);
    }
    else {
         // Copy file instead of renaming resource to keep the comments.
        fs::copy_file(
            ayu::resource_filename(
                iri::constant("res:/vf/game/settings-initial.ayu")
            ),
            ayu::resource_filename(settings_res->name())
        );
        ayu::load(settings_res);
    }

    if (ayu::source_exists(options_res->name())) {
        ayu::load(options_res);
    }
    else {
        auto initial = ayu::SharedResource(
            iri::constant("res:/vf/game/options-initial.ayu")
        );
        ayu::load(initial);
        ayu::rename(initial, options_res);
        ayu::save(options_res);
    }

    if (ayu::source_exists(state_res->name())) {
        ayu::load(state_res);
    }
    else {
        ayu::SharedResource world (iri::constant("res:/vf/world/world.ayu"));
        state_res->set_value(ayu::Dynamic::make<State>());
        auto& state = state_res->get_value().as<State>();
        state.current_room = world["start"][1];
        state.world = move(world->value().as<ayu::Document>());
        ayu::force_unload(world);
         // Don't save state yet
    }
}

Game::~Game () { current_game = null; }

Settings& Game::settings () {
    return settings_res->value().as_known<Settings>();
}

Options& Game::options () {
    return state_res->value().as_known<Options>();
}

State& Game::state () {
    return state_res->value().as_known<State>();
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
#include "../world/common.h"

tap::TestSet tests ("vf/game", []{
    using namespace control;
    using namespace tap;
    fs::remove(ayu::resource_filename(iri::constant("data:/state.ayu")));
    Game game;
    auto room = game.state().current_room;
    ok(room->find_with_types(Types::Verdant), "Initial state has player");
    int window_id = SDL_GetWindowID(game.window);
    send_input_as_event({.type = InputType::Key, .code = SDLK_ESCAPE}, window_id);
    game.loop.start();
    pass();
    done_testing();
});
#endif
