#include "options.h"

#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
} using namespace vf;

AYU_DESCRIBE(vf::Options,
    attrs(
        attr("frustration", &Options::frustration, optional)
    )
)
