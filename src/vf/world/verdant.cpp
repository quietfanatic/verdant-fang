#include "verdant.h"

namespace vf {

Verdant::Verdant () { }

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
