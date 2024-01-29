#include "player.h"
#include "../dirt/ayu/reflection/describe.h"

using namespace vf;

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Resident", base<Resident>(), include)
    )
)
