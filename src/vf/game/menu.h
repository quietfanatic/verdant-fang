#pragma once
#include "../../dirt/control/command.h"
#include "../../dirt/glow/colors.h"
#include "controls.h"
#include "render.h"

namespace vf {
struct Menu;

struct MenuDrawable {
    virtual void MenuDrawable_draw (
        Menu* menu, Vec pos, glow::RGBA8 tint
    ) = 0;
};

struct MenuImage : MenuDrawable, Frame {
    void MenuDrawable_draw (Menu*, Vec, glow::RGBA8) override;
};

template <class T>
struct MenuOption : MenuDrawable {
    Frame frame;
    T* option;
    void MenuDrawable_draw (Menu*, Vec, glow::RGBA8) override;
};

struct MenuDecoration {
    MenuDrawable* draw;
    Vec pos;
};

struct MenuItem {
    MenuDrawable* draw;
    Vec pos;
    control::Statement on_press;
    control::Statement on_left;
    control::Statement on_right;
};

struct Menu {
    UniqueArray<MenuDecoration> decorations;
    UniqueArray<MenuItem> items;
    control::Statement on_back;
    glow::RGBA8 decoration_tint = {};
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
