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
        if (current_index > max) current_index = max;
    }
    else if (controls[Control::Down] == 1) {
        current_index += 1;
        uint32 max = data->items ? data->items.size() - 1 : 0;
        if (current_index > max) current_index = max;
    }
    else if (controls[Control::Confirm] == 1) {
        if (current_index < data->items.size()) {
            data->items[current_index].on_press();
        }
    }
    else if (controls[Control::Back] == 1) {
        if (data->close_on_back) {
             // This probably destroys this.
            current_game->menus.pop_back();
        }
    }
}

void OpenMenu::draw () {
    for (auto& deco : data->decorations) {
        draw_frame(*deco.frame, 0, deco.pos);
    }
    for (usize i = 0; i < data->items.size(); i++) {
        draw_frame(
            *data->items[i].frame, 0, data->items[i].pos, 10, {1, 1},
            current_index == i ? data->selected_tint : data->unselected_tint
        );
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::MenuDecoration,
    elems(
        elem(&MenuDecoration::frame),
        elem(&MenuDecoration::pos)
    )
)

AYU_DESCRIBE(vf::MenuButton,
    attrs(
        attr("frame", &MenuButton::frame),
        attr("pos", &MenuButton::pos),
        attr("on_press", &MenuButton::on_press)
    )
)

AYU_DESCRIBE(vf::Menu,
    attrs(
        attr("decorations", &Menu::decorations, optional),
        attr("items", &Menu::items),
        attr("selected_tint", &Menu::selected_tint, optional),
        attr("unselected_tint", &Menu::unselected_tint, optional),
        attr("default_index", &Menu::default_index, optional),
        attr("close_on_back", &Menu::close_on_back, optional)
    )
)
