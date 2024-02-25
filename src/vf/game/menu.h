#pragma once
#include "../../dirt/control/command.h"
#include "../../dirt/glow/colors.h"
#include "controls.h"
#include "render.h"

namespace vf {
struct Menu;
struct Music;
struct OpenMenu;

struct MenuDrawable {
    virtual void MenuDrawable_draw (
        OpenMenu&, Vec pos, glow::RGBA8 tint
    ) = 0;
};

struct MenuImage : MenuDrawable, Frame {
    void MenuDrawable_draw (OpenMenu&, Vec, glow::RGBA8) override;
};

struct MenuFang : MenuDrawable {
    Frame frame;
    uint8 cycle [4];
    void MenuDrawable_draw (OpenMenu&, Vec, glow::RGBA8) override;
};

struct MenuOptionBase : Frame {
    uint32* option;
    uint32 value;
    void draw (OpenMenu&, Vec, bool);
};

template <class T>
struct MenuOption : MenuDrawable, MenuOptionBase {
    T* option;
    T value;
    void MenuDrawable_draw (OpenMenu& om, Vec pos, glow::RGBA8) override {
        draw(om, pos, option && *option == value);
    }
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
    Music* music = null;
};

struct OpenMenu {
    Menu* data;
    uint32 current_index;
    uint32 frame_count = 0;
    OpenMenu (Menu*);
    ~OpenMenu ();
    OpenMenu (const OpenMenu&) = delete;
    OpenMenu (OpenMenu&& o) : data(o.data), current_index(o.current_index) {
        o.data = null;
    }
    void step (const Controls&);
    void draw ();
};

} // vf
