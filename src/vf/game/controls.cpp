#include "controls.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "game.h"
#include "settings.h"

namespace vf {

Controls PlayerMind::Mind_think (Resident&) {
    return current_game->settings().read_controls();
}

} using namespace vf;

AYU_DESCRIBE(vf::Control,
    values(
        value("confirm", Control::Confirm),
        value("back", Control::Back),
        value("pause", Control::Pause),
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

AYU_DESCRIBE(vf::PlayerMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include)
    )
)
