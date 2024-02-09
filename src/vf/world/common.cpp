#include "common.h"

#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
} using namespace vf;

AYU_DESCRIBE(vf::TexAndFrame,
    attrs(
        attr("tex", &TexAndFrame::tex),
        attr("frame", &TexAndFrame::frame)
    )
)
