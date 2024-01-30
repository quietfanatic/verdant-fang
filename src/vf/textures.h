#pragma once
#include "../dirt/iri/iri.h"
#include "../dirt/ayu/resources/resource.h"

namespace vf {

constexpr iri::IRI textures_loc ("res:/vf/textures.ayu");
inline ayu::SharedResource& textures_res () {
    static ayu::SharedResource r (textures_loc);
    return r;
}

} // vf
