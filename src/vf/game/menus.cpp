#include "menus.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
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
