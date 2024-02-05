#pragma once

#include "../../dirt/control/command.h"
#include "../../dirt/control/input.h"
#include "common.h"
#include "controls.h"

namespace vf {

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
    Controls read_controls () const;
    UniqueArray<CommandBinding> commands;
};

} // vf
