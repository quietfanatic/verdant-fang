#include "menu.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "game.h"

namespace vf {

OpenMenu::OpenMenu (Menu* d) :
    data(d), current_index(data->default_index)
{ }

void OpenMenu::step (const Controls& controls) {
     // Only allow one input at once to avoid misinputs
    uint32 count = (controls[Control::Up] == 1)
                 + (controls[Control::Down] == 1)
                 + (controls[Control::Left] == 1)
                 + (controls[Control::Right] == 1)
                 + (controls[Control::Confirm] == 1)
                 + (controls[Control::Back] == 1);
    if (count != 1) return;
    if (controls[Control::Up] == 1) {
        current_index -= 1;
        uint32 max = data->items ? data->items.size() - 1 : 0;
        if (current_index > max) current_index = 0;
    }
    else if (controls[Control::Down] == 1) {
        current_index += 1;
        uint32 max = data->items ? data->items.size() - 1 : 0;
        if (current_index > max) current_index = max;
    }
    else if (controls[Control::Left] == 1) {
        if (current_index < data->items.size()) {
            if (auto& stmt = data->items[current_index].on_left) {
                stmt();
            }
        }
    }
    else if (controls[Control::Right] == 1) {
        if (current_index < data->items.size()) {
            if (auto& stmt = data->items[current_index].on_right) {
                stmt();
            }
        }
    }
    else if (controls[Control::Confirm] == 1) {
        if (current_index < data->items.size()) {
            if (auto& stmt = data->items[current_index].on_press) {
                stmt();
            }
        }
    }
    else if (controls[Control::Back] == 1) {
         // This may destroy this OpenMenu, so don't do anything more.
        if (data->on_back) data->on_back();
    }
}

void MenuImage::MenuDrawable_draw (Menu*, Vec pos, glow::RGBA8 tint) {
    draw_frame(*this, 0, pos, 10, {1, 1}, tint);
}

void OpenMenu::draw () {
    for (auto& deco : data->decorations) {
        deco.draw->MenuDrawable_draw(
            data, deco.pos, data->decoration_tint
        );
    }
    for (usize i = 0; i < data->items.size(); i++) {
        data->items[i].draw->MenuDrawable_draw(
            data, data->items[i].pos,
            current_index == i ? data->selected_tint : data->unselected_tint
        );
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::MenuDrawable,
    attrs()
)

AYU_DESCRIBE(vf::MenuImage,
    elems(
        elem(&MenuImage::target),
        elem(&MenuImage::offset),
        elem(&MenuImage::bounds)
    ),
    attrs(
        attr("vf::MenuDrawable", base<MenuDrawable>(), include),
        attr("vf::Frame", base<Frame>(), include)
    )
)

AYU_DESCRIBE(vf::MenuDecoration,
    elems(
        elem(&MenuDecoration::draw),
        elem(&MenuDecoration::pos)
    )
)

AYU_DESCRIBE(vf::MenuItem,
    attrs(
        attr("draw", &MenuItem::draw),
        attr("pos", &MenuItem::pos),
        attr("on_press", &MenuItem::on_press, optional),
        attr("on_left", &MenuItem::on_left, optional),
        attr("on_right", &MenuItem::on_right, optional)
    )
)

AYU_DESCRIBE(vf::Menu,
    attrs(
        attr("decorations", &Menu::decorations, optional),
        attr("items", &Menu::items),
        attr("decoration_tint", &Menu::decoration_tint, optional),
        attr("selected_tint", &Menu::selected_tint, optional),
        attr("unselected_tint", &Menu::unselected_tint, optional),
        attr("default_index", &Menu::default_index, optional),
        attr("on_back", &Menu::on_back, optional)
    )
)
