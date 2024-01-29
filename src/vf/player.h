#pragma once
#include "common.h"
#include "room.h"

namespace vf {

inline Player* the_player = null;

struct Player : Resident {
    void Resident_emerge () override {
        expect(!the_player); the_player = this;
    }
    void Resident_reclude () override {
        the_player = null;
    }
    ~Player () { the_player = null; }
};

} // vf
