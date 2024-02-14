#include "semisolid.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/render.h"

namespace vf {

void Semisolid::init () {
    types |= Types::Semisolid;
    hb.layers_2 = Layers::Walker_Semisolid;
    hb.box = Rect(data->bounds);
    set_hitbox(hb);
}

void Semisolid::Resident_draw () {
    draw_frame(*data, 0, pos, Z::Semisolid);
}

} using namespace vf;

AYU_DESCRIBE(vf::Semisolid,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Semisolid::data)
    ),
    init<&Semisolid::init>()
)
