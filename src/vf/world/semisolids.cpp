#include "semisolids.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/render.h"
#include "blocks.h"

namespace vf {

Semisolids::Semisolids () { pos = {0, 0}; }

void Semisolids::init () {
    types |= Types::Semisolid;
    clear_hitboxes();
    for (auto& b : blocks) {
        b.hb.layers_2 = Layers::Walker_Semisolid;
        b.hb.box = b.pos + Rect(data->bounds);
        add_hitbox(b.hb);
    }
}

void Semisolids::Resident_draw () {
    for (auto& b : blocks) {
        draw_frame(*data, 0, b.pos, Z::Semisolid);
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Semisolids,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Semisolids::data),
        attr("blocks", &Semisolids::blocks)
    ),
    init<&Semisolids::init>()
)
