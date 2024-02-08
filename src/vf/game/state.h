#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "common.h"

namespace vf {

struct State {
    union {
        std::minstd_rand rng;
        uint32 rng_uint32;
    };
    ayu::Document world;
    Room* current_room = null;
    State () : rng(0) { }
};

} // vf
