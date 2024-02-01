#include "scenery.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/texture-program.h"
#include "camera.h"

namespace vf {

void Scenery::Resident_draw () {
    glow::draw_texture(*tex, world_to_screen(bounds + pos));
}

} using namespace vf;

AYU_DESCRIBE(vf::Scenery,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("tex", &Scenery::tex),
        attr("bounds", &Scenery::bounds)
    )
)
