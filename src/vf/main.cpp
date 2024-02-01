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
    ayu::FolderResourceScheme res_scheme ("res", uni::cat(base, + "res"));
    ayu::FolderResourceScheme data_scheme ("data", UniqueString(base));
    SDL_free(base);
    tap::allow_testing(argc, argv);

    try {
        Game game;
        game.start();
    }
    catch (std::exception& e) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Sorry, the program crashed.",
            cat("Uncaught exception: ", e.what()).c_str(),
            null
        );
        throw;
    }
    return 0;
}
