#include "options.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "game.h"

namespace vf {

void frustration_ (int diff) {
    if (!current_game) return;
    auto& options = current_game->options();
    options.frustration += diff;
    options.frustration = clamp(options.frustration + diff, 1, 3);
}
control::Command frustration (frustration_, "frustration", "Set frustration level, 1..3");

void blood_ (bool) {
}
control::Command blood (blood_, "blood", "Set whether to show or hide blood");

} using namespace vf;

AYU_DESCRIBE(vf::Options,
    attrs(
        attr("frustration", &Options::frustration, optional)
    )
)
