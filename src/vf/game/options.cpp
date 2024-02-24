#include "options.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "game.h"

namespace vf {

void frustration_ (int diff) {
    if (!current_game) return;
    auto& options = current_game->options();
    options.frustration = clamp(options.frustration + diff, 1, 3);
    ayu::save(current_game->options_res);
}
control::Command frustration (frustration_, "frustration", "Alter frustration level (relatively)");

void enemy_difficulty_ (int diff) {
    if (!current_game) return;
    auto& options = current_game->options();
    options.enemy_difficulty = clamp(options.enemy_difficulty + diff, 1, 2);
    ayu::save(current_game->options_res);
}
control::Command enemy_difficulty (enemy_difficulty_, "enemy_difficulty", "Alter enemy difficulty level (relatively)");

void hide_blood_ (bool hide) {
    if (!current_game) return;
    auto& options = current_game->options();
    options.hide_blood = hide;
    ayu::save(current_game->options_res);
}
control::Command hide_blood (hide_blood_, "hide_blood", "Set whether to show or hide blood");

} using namespace vf;

AYU_DESCRIBE(vf::Options,
    attrs(
        attr("frustration", &Options::frustration, optional),
        attr("enemy_difficulty", &Options::enemy_difficulty, optional),
        attr("hide_blood", &Options::hide_blood, optional)
    )
)
