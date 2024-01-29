#include "block.h"
#include "../dirt/ayu/reflection/describe.h"

using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include)
    )
)
