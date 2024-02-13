#include "scenery.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"
#include "../game/render.h"

namespace vf {

Scenery::Scenery () {
    types |= Types::Scenery;
}

void Scenery::Resident_draw () {
    draw_frame(*data, 0, pos, z, scale);
}

} using namespace vf;

AYU_DESCRIBE(vf::Scenery,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Scenery::data),
        attr("z", &Scenery::z, optional),
        attr("scale", &Scenery::scale, optional)
    )
)
