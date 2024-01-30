#pragma once
#include "../dirt/iri/iri.h"
#include "../dirt/ayu/resources/resource.h"

namespace vf {

inline ayu::SharedResource& assets () {
    static ayu::SharedResource r (iri::constant("res:/vf/assets.ayu"));
    return r;
}

} // vf
