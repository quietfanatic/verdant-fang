#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "common.h"

namespace control { struct Statement; }

namespace vf {

struct Scheduled;
struct State {
    union {
        std::minstd_rand rng;
        uint32 rng_uint32;
    };
    uint64 current_frame = 0;
    Room* current_room = null;
    UniqueArray<Scheduled> scheduled;
    ayu::Document world;

    State ();
    void step ();
     // If delay is 0, runs action at the BEGINNING of the NEXT frame.  And 1 is
     // the frame after that.
    void schedule_after (uint32 delay_frames, control::Statement&&);
};

} // vf
