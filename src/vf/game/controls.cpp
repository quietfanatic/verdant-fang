#include "controls.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "game.h"
#include "settings.h"
#include "state.h"

namespace vf {

void accumulate_controls (Controls& acc, const Controls& in) {
    for (usize i = 0; i < Control::N_Controls; i++) {
        if (in[i]) {
            if (acc[i] < 255) acc[i] += 1;
        }
        else acc[i] = 0;
    }
}

Controls Player::Mind_think (Resident&) {
    auto inputs = current_game->settings().read_controls();
     // Burn rng values when buttons are pressed in a non-sophisticated way.
    auto& rng = current_game->state().rng;
    for (auto& in : inputs) if (in) rng();
    accumulate_controls(controls, inputs);
    return controls;
}

} using namespace vf;

AYU_DESCRIBE(vf::Control,
    values(
        value("confirm", Control::Confirm),
        value("back", Control::Back),
        value("left", Control::Left),
        value("right", Control::Right),
        value("up", Control::Up),
        value("down", Control::Down),
        value("jump", Control::Jump),
        value("attack", Control::Attack),
        value("special", Control::Special)
    )
)

 // Abstract but we need to include it for castability
AYU_DESCRIBE(vf::Mind,
    attrs()
)

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("controls", &Player::controls, optional)
    )
)
