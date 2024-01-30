#include "settings.h"

#include <SDL2/SDL_events.h>
#include "../dirt/ayu/reflection/describe.h"

namespace vf {

Actions Settings::get_actions () const {
    Actions r = {};
    auto keyboard = SDL_GetKeyboardState(null);
    for (auto& binding : action_bindings) {
        if (input_currently_pressed(binding.input, keyboard)) {
            expect(binding.action < r.size());
            r[binding.action] = true;
        }
    }
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Action,
    values(
        value("confirm", Action::Confirm),
        value("back", Action::Back),
        value("pause", Action::Pause),
        value("left", Action::Left),
        value("right", Action::Right),
        value("up", Action::Up),
        value("down", Action::Down),
        value("jump", Action::Jump),
        value("attack", Action::Attack),
        value("special", Action::Special)
    )
)

AYU_DESCRIBE(vf::ActionBinding,
    elems(
        elem(&ActionBinding::input),
        elem(&ActionBinding::action)
    )
)

AYU_DESCRIBE(vf::Settings,
    attrs(
        attr("action_bindings", &Settings::action_bindings)
    )
)
