#include <SDL2/SDL.h>
#include "../dirt/ayu/resources/scheme.h"
#include "../dirt/glow/common.h"
#include "../dirt/tap/tap.h"
#include "common.h"
#include "game.h"

using namespace uni;
using namespace vf;

int main (int argc, char** argv) {
    char* base = glow::require_sdl(SDL_GetBasePath());
    ayu::FileResourceScheme res_scheme ("res", uni::cat(base, + "res"));
    ayu::FileResourceScheme data_scheme ("data", UniqueString(base));
    free(base);
    tap::allow_testing(argc, argv);

    Game game;
    game.start();
    return 0;
}
