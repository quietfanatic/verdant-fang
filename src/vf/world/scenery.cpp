#include "scenery.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"
#include "../game/frame.h"

namespace vf {

Scenery::Scenery () { pos = {0, 0}; }

void Scenery::Resident_draw () {
    draw_frame(pos, {{0, 0}, bounds}, *tex);
}

} using namespace vf;

AYU_DESCRIBE(vf::Scenery,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("tex", &Scenery::tex),
        attr("bounds", &Scenery::bounds)
    )
)
