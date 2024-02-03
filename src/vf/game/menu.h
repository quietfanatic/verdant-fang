#pragma once

namespace vf {

struct MenuButton {
    Texture* tex;
    Statement on_press;
    IRect margin;
};

} // vf
