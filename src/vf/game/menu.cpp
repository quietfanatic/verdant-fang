#include "menu.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "game.h"
#include "sound.h"

namespace vf {

OpenMenu::OpenMenu (Menu* d) :
    data(d), current_index(data->default_index)
{
    if (data->music) {
        data->music->play();
    }
}

OpenMenu::~OpenMenu () {
    if (data && data->music) data->music->stop();
}

void OpenMenu::step (const Controls& controls) {
    frame_count += 1;
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

void MenuImage::MenuDrawable_draw (OpenMenu&, Vec pos, glow::RGBA8 tint) {
    draw_frame(*this, 0, pos, 10, {1, 1}, tint);
}

Vec MenuImage::MenuDrawable_cursor_pos (Vec pos) {
    return pos + Vec(bounds.l - 6, (bounds.t + bounds.b) / 2);
}

void MenuFang::MenuDrawable_draw (OpenMenu& om, Vec pos, glow::RGBA8) {
    uint32 len = 0;
    for (auto& phase : cycle) len += phase;
    uint32 timer = om.frame_count % len;
    uint8 layers;
    if (timer < cycle[0]) layers = 0b001;
    else {
        timer -= cycle[0];
        if (timer < cycle[1]) layers = 0b011;
        else {
            timer -= cycle[1];
            if (timer < cycle[2]) layers = 0b111;
            else layers = 0b011;
        }
    }
    draw_layers(frame, layers, pos, 10);
}

void MenuOptionBase::draw (OpenMenu& om, Vec pos, bool eq) {
    draw_frame(
        *this, 0, pos, 10, {1, 1},
        eq ? om.data->selected_tint : om.data->unselected_tint
    );
}

void OpenMenu::draw () {
    for (auto& deco : data->decorations) {
        deco.draw->MenuDrawable_draw(
            *this, deco.pos, data->decoration_tint
        );
    }
    for (usize i = 0; i < data->items.size(); i++) {
        data->items[i].draw->MenuDrawable_draw(
            *this, data->items[i].pos,
            current_index == i ? data->selected_tint : data->unselected_tint
        );
    }
    if (data->cursor && current_index < data->items.size()) {
        auto pos = data->items[current_index].draw->MenuDrawable_cursor_pos(
            data->items[current_index].pos
        );
        if (defined(pos)) {
            draw_frame(
                *data->cursor[(frame_count / 8) % data->cursor.size()], 0, pos
            );
        }
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

AYU_DESCRIBE(vf::MenuFang,
    attrs(
        attr("vf::MenuDrawable", base<MenuDrawable>(), include),
        attr("frame", &MenuFang::frame),
        attr("cycle", &MenuFang::cycle)
    )
)

AYU_DESCRIBE_TEMPLATE(
    AYU_DESCRIBE_TEMPLATE_PARAMS(class T),
    AYU_DESCRIBE_TEMPLATE_TYPE(vf::MenuOption<T>),
    desc::name([]{
        if constexpr (std::is_same_v<T, uint8>) {
            return StaticString("vf::MenuOption<uint8>");
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return StaticString("vf::MenuOption<bool>");
        }
        else static_assert((T*)null);
    }()),
    desc::elems(
        desc::elem(&MenuOption<T>::target),
        desc::elem(&MenuOption<T>::offset),
        desc::elem(&MenuOption<T>::bounds),
        desc::elem(&MenuOption<T>::option),
        desc::elem(&MenuOption<T>::value)
    ),
    desc::attrs(
        desc::attr("vf::MenuDrawable", desc::template base<MenuDrawable>(), desc::include),
        desc::attr("vf::Frame", desc::template base<Frame>(), desc::include),
        desc::attr("option", &MenuOption<T>::option),
        desc::attr("value", &MenuOption<T>::value)
    )
)

AYU_DESCRIBE_INSTANTIATE(MenuOption<uint8>)
AYU_DESCRIBE_INSTANTIATE(MenuOption<bool>)

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
        attr("on_back", &Menu::on_back, optional),
        attr("decoration_tint", &Menu::decoration_tint, optional),
        attr("selected_tint", &Menu::selected_tint, optional),
        attr("unselected_tint", &Menu::unselected_tint, optional),
        attr("default_index", &Menu::default_index, optional),
        attr("music", &Menu::music, optional),
        attr("allow_pause", &Menu::allow_pause, optional),
        attr("cursor", &Menu::cursor, optional)
    )
)
