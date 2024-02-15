#include "bug.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
} using namespace vf;

AYU_DESCRIBE(vf::Bug,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
