#include "indigo.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
} using namespace vf;

AYU_DESCRIBE(vf::Indigo,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
