#include "game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../../dirt/iri/iri.h"
#include "../../dirt/control/input.h"
#include "camera.h"
#include "menu.h"
#include "options.h"
#include "room.h"
#include "settings.h"
#include "state.h"

namespace vf {

static bool on_event (Game& game, SDL_Event* event) {
    switch (event->type) {
         // TODO: handle SDL_QUIT
        case SDL_KEYDOWN: {
            SDL_ShowCursor(SDL_DISABLE);
            auto input = control::input_from_event(event);
            for (auto& binding : game.settings().commands) {
                if (input_matches_binding(input, binding.input)) {
                    if (binding.command) {
                        try {
                            binding.command();
                        }
                        catch (std::exception& e) {
                            SDL_ShowSimpleMessageBox(
                                SDL_MESSAGEBOX_ERROR,
                                "Exception while executing hotkey command.",
                                cat(e.what()).c_str(),
                                game.window
                            );
                        }
                    }
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

static void on_step (Game& game) {
     // Read menu controls even if there isn't a menu active, so that if we open
     // a menu while holding a button, the menu doesn't think that button was
     // just pressed.  Also this does want to be separate from player controls,
     // otherwise the player could take different actions by pausing the game
     // (they'd be able to make an input 1 on consecutive frames, which should
     // be impossible).
    accumulate_controls(game.menu_controls, game.settings().read_controls());
    if (game.menus) game.menus.back().step(game.menu_controls);
    else {
        game.state().step();
        if (game.turbo) game.state().step();
    }
}

static void on_draw (Game& game) {
    begin_camera();
    auto room = game.state().current_room;
    if (room) room->draw();
    end_camera();
    if (room && game.menus) {
         // Fade out game if both game and menu are shown
        draw_rectangle(Rect(Vec(0, 0), camera_size), 0x00000080);
    }
    if (game.menus) game.menus.back().draw();
    finish_frame();
    SDL_GL_SwapWindow(game.window);
}

Game::Game () :
    window("Verdant Fang", size(window_viewport)),
    loop{
        .on_event = [this](SDL_Event* e){ return on_event(*this, e); },
        .on_step = [this]{ on_step(*this); },
        .on_draw = [this]{ on_draw(*this); },
    },
    settings_res(iri::constant("save:/settings.ayu")),
    options_res(iri::constant("save:/options.ayu")),
    state_res(iri::constant("save:/state.ayu"))
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

    ayu::SharedResource menus_res (iri::constant("res:/vf/game/menus.ayu"));
    ayu::global(&start_menu);
    start_menu = menus_res["start_menu"][1];
    ayu::global(&pause_menu);
    pause_menu = menus_res["pause_menu"][1];
    ayu::global(&options_menu);
    options_menu = menus_res["options_menu"][1];
    ayu::global(&end_menu);
    end_menu = menus_res["end_menu"][1];

    if (ayu::source_exists(state_res->name())) {
        ayu::load(state_res);
        menus.emplace_back(pause_menu);
    }
    else {
        state_res->set_value(ayu::Dynamic::make<State>());
        auto& state = state_res->get_value().as<State>();
        state.load_initial();
        menus.emplace_back(start_menu);
    }
}

Game::~Game () {
    ayu::unregister_global(&options_menu);
    ayu::unregister_global(&pause_menu);
    current_game = null;
}

Settings& Game::settings () {
    return settings_res->value().as_known<Settings>();
}

Options& Game::options () {
    return options_res->value().as_known<Options>();
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

void Game::reset () {
     // Clear everything including transitions and stuff
    state_res->set_value(ayu::Dynamic::make<State>());
    auto& state = state_res->get_value().as<State>();
    state.load_initial();
    menus = {};
    menus.push_back(OpenMenu(start_menu));
}

bool Game::hardcore () {
    return options().frustration >= 3 && options().enemy_difficulty >= 2 &&
        (!state().checkpoint || state().checkpoint->checkpoint_level >= 3) &&
        !state().lost_hardcore;
}

void Game::suspend () {
    ayu::save(current_game->state_res);
    exit(0);
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
    fs::remove(ayu::resource_filename(iri::constant("save:/state.ayu")));
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
