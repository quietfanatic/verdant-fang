#include "state.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {
} using namespace vf;

AYU_DESCRIBE(vf::State,
    attrs(
        attr("rng", member(&State::rng_uint32, prefer_hex), optional),
        attr("current_room", &State::current_room),
        attr("world", &State::world)
    )
);
