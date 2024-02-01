#include "scenery.h"

#include "../dirt/ayu/reflection/describe.h"
#include "camera.h"
#include "frame.h"

namespace vf {

void Scenery::Resident_draw () {
    draw_frame({{0, 0}, bounds}, *tex, pos);
}

} using namespace vf;

AYU_DESCRIBE(vf::Scenery,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("tex", &Scenery::tex),
        attr("bounds", &Scenery::bounds)
    )
)
