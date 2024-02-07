#include "decals.h"

#include "../../dirt/ayu/reflection/describe.h"

namespace vf {

void draw_decals (const Walker&, const Pose&) {
    return;
}

} using namespace vf;

AYU_DESCRIBE(vf::DecalData,
    attrs(
        attr("stab_tex", &DecalData::stab_tex),
        attr("stab_0", &DecalData::stab_0),
        attr("stab_1", &DecalData::stab_1),
        attr("stab_2", &DecalData::stab_2)
    )
)
