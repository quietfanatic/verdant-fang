#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "camera.h"
#include "common.h"

namespace control { struct Statement; }

namespace vf {

struct Transition {
    Room* target_room;
    Resident* migrant;
    Vec target_pos = GNAN;
    TransitionType type = TransitionType::WipeLeft;
    bool set_checkpoint = false;
    uint32 exit_at = 8;
    uint32 enter_at = 24;
    uint32 end_at = 30;
    uint32 timer = 0;

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

    std::optional<ayu::Tree> checkpoint;

     // Temporary, don't save
    bool save_checkpoint = false;
    bool load_checkpoint = false;

    State ();
    void step ();
};

} // vf
