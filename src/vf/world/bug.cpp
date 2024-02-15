#include "bug.h"
#include "../../dirt/ayu/reflection/describe.h"

namespace vf {


void Bug::Resident_before_step () {
    wings_timer += 1;
    if (wings_timer >= 6) wings_timer = 0;
    Walker::Resident_before_step();
}

Pose Bug::Walker_pose () {
    auto& poses = *data->poses;
    Pose r = Walker::Walker_pose();
     // Using the head segment for the wings
    r.head = wings_timer < 3 ? poses.fly[0].head : poses.fly[1].head;
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Bug,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
