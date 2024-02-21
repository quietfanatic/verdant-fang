#pragma once
#include "render.h"

namespace vf {

struct MenuDecoration {
    Frame* frame;
    Vec pos;
};

struct Menu {
    UniqueArray<MenuDecoration> decorations;
    void draw ();
};

} // vf
