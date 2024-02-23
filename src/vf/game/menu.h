#pragma once
#include "../../dirt/control/command.h"
#include "../../dirt/glow/colors.h"
#include "controls.h"
#include "render.h"

namespace vf {

struct MenuDecoration {
    Frame* frame;
    Vec pos;
};

struct MenuButton {
    Frame* frame;
    Vec pos;
    control::Statement on_press;
    control::Statement on_left;
    control::Statement on_right;
};

struct Menu {
    UniqueArray<MenuDecoration> decorations;
    UniqueArray<MenuButton> items;
    control::Statement on_back;
    glow::RGBA8 selected_tint = {};
    glow::RGBA8 unselected_tint = {};
    uint32 default_index = 0;
};

struct OpenMenu {
    Menu* data;
    usize current_index;
    OpenMenu (Menu*);
    void step (const Controls&);
    void draw ();
};

} // vf
