#include "settings.h"

#include <SDL2/SDL_events.h>
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {

Controls Settings::read_controls () {
    Controls r = {};
    auto keyboard = SDL_GetKeyboardState(null);
    for (auto& binding : controls) {
        if (input_currently_pressed(binding.input, keyboard)) {
            expect(binding.control < r.size());
            r[binding.control] = true;
        }
    }
     // Kind of a weird hacky way to do this but the obvious solution (doing in
     // the above loop) doesn't work if there's more than one binding for the
     // same control.
    for (usize i = 0; i < Control::N_Controls; i++) {
        if (!r[i]) disable_while_held[i] = false;
        if (disable_while_held[i]) r[i] = false;
    }
    return r;
}

} using namespace vf;

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
        attr("audio_buffer", &Settings::audio_buffer, optional),
        attr("true_fullscreen", &Settings::true_fullscreen, optional),
        attr("controls", &Settings::controls),
        attr("commands", &Settings::commands)
    )
)
