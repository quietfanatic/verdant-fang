#pragma once

#include "../dirt/control/input.h"
#include "common.h"

namespace vf {

enum Action {
    Confirm,
    Back,
    Pause,
    Left,
    Right,
    Up,
    Down,
    Jump,
    Attack,
    Special,
    N_Actions
};

using Actions = std::array<bool, N_Actions>;

struct ActionBinding {
    control::Input input;
    Action action;
};

struct Settings {
    Actions get_actions () const;
    UniqueArray<ActionBinding> action_bindings;
};

} // vf
