#include "menu.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {

void Menu::draw () {
    for (auto& deco : decorations) {
        draw_frame(*deco.frame, 0, deco.pos);
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::MenuDecoration,
    elems(
        elem(&MenuDecoration::frame),
        elem(&MenuDecoration::pos)
    )
)

AYU_DESCRIBE(vf::Menu,
    attrs(
        attr("decorations", &Menu::decorations, optional)
    )
)
