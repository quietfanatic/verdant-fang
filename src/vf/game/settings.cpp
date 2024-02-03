#include "settings.h"

#include <SDL2/SDL_events.h>
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {

Controls Settings::get_controls () const {
    Controls r = {};
    auto keyboard = SDL_GetKeyboardState(null);
    for (auto& binding : controls) {
        if (input_currently_pressed(binding.input, keyboard)) {
            expect(binding.control < r.size());
            r[binding.control] = true;
        }
    }
    return r;
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

AYU_DESCRIBE(vf::ControlBinding,
    elems(
        elem(&ControlBinding::input),
        elem(&ControlBinding::control)
    )
)

AYU_DESCRIBE(vf::CommandBinding,
    elems(
        elem(&CommandBinding::input),
        elem(&CommandBinding::command)
    )
)

AYU_DESCRIBE(vf::Settings,
    attrs(
        attr("true_fullscreen", &Settings::true_fullscreen),
        attr("controls", &Settings::controls),
        attr("commands", &Settings::commands)
    )
)
