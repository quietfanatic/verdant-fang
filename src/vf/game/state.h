#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "common.h"

namespace vf {

inline State* current_state = null;

struct State {
    union {
        std::minstd_rand rng;
        uint32 rng_uint32;
    };
    ayu::Document world;
    State () : rng(0) {
        expect(!current_state);
        ayu::global(&current_state);
        current_state = this;
    }
    ~State () {
        current_state = null;
        ayu::unregister_global(&current_state);
    }
};

} // vf
