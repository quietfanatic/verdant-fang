#pragma once

#include "../../dirt/control/command.h"
#include "../../dirt/control/input.h"
#include "common.h"

namespace vf {

enum Control {
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
    N_Controls,
};

using Controls = std::array<bool, N_Controls>;

struct ControlBinding {
    control::Input input;
    Control control;
};

struct CommandBinding {
    control::Input input;
    control::Statement command;
};

struct Settings {
    bool true_fullscreen = false;
    UniqueArray<ControlBinding> controls;
    Controls get_controls () const;
    UniqueArray<CommandBinding> commands;
};

} // vf
