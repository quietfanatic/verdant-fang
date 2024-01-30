#pragma once
#include "../dirt/iri/iri.h"
#include "../dirt/ayu/resources/resource.h"

namespace vf {

inline ayu::SharedResource& textures_res () {
    static ayu::SharedResource r (iri::constant("res:/vf/textures.ayu"));
    return r;
}

} // vf
