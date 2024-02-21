#include "controls.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "game.h"
#include "settings.h"
#include "state.h"

namespace vf {

Controls Player::Mind_think (Resident&) {
    auto inputs = current_game->settings().read_controls();
    auto& rng = current_game->state().rng;
    for (usize i = 0; i < Control::N_Controls; i++) {
        if (inputs[i]) {
            rng();
            if (controls[i] < 255) controls[i] += 1;
        }
        else controls[i] = 0;
    }
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
