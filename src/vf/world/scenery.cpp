#include "scenery.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"
#include "../game/render.h"

namespace vf {

Scenery::Scenery () { pos = {0, 0}; }

void Scenery::Resident_draw () {
    draw_texture(
        *tex, bounds, {0, 0, length(width(bounds)), length(height(bounds))},
        Z::BG
    );
}

} using namespace vf;

AYU_DESCRIBE(vf::Scenery,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("tex", &Scenery::tex),
        attr("bounds", &Scenery::bounds)
    )
)
