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
    uint32 audio_buffer = 512;
    bool true_fullscreen = false;
    UniqueArray<ControlBinding> controls;
     // Only returns 0 and 1, do counting in Mind
    Controls read_controls ();
    UniqueArray<CommandBinding> commands;

     // Kind of a hack to make it so that pressing a button to close a menu
     // doesn't cause the game to take actions on the same button press.
    Controls disable_while_held = {};
};

} // vf
