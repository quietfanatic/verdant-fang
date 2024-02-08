#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "common.h"

namespace control { struct Statement; }

namespace vf {

struct Transition {
    Room* target_room;
    Resident* migrant;
    Vec target_pos = GNAN;
    uint32 until_exit = 8;
    uint32 until_enter = 24;

    bool step (State&);
};

struct State {
    union {
        std::minstd_rand rng;
        uint32 rng_uint32;
    };
    uint64 current_frame = 0;
    Room* current_room = null;
    std::optional<Transition> transition;

    ayu::Document world;

    State ();
    void step ();
};

} // vf
