#include "game.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../dirt/control/input.h"

namespace vf {

Game::Game () :
    window("Verdant Fang (testing)", {640, 360}),
    loop{
        .on_event = [](SDL_Event*){ return false; },
        .on_step = []{},
        .on_draw = []{},
    }
{ }

} using namespace vf;

#ifndef TAP_DISABLE_TESTS
#include "../dirt/tap/tap.h"

tap::TestSet tests ("vf/game", []{
    using namespace control;
    using namespace tap;
    Game game;
    int window_id = SDL_GetWindowID(game.window);
    send_input_as_event({.type = InputType::Key, .code = SDLK_ESCAPE}, window_id);
    game.loop.start();
    pass();
    done_testing();
});
#endif
