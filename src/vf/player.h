#pragma once
#include "common.h"
#include "room.h"

namespace vf {

inline Player* the_player = null;

struct Player : Resident {
    Vec vel;
    Block* floor = null;
    Player ();
    void Resident_emerge () override {
        expect(!the_player); the_player = this;
    }
    void Resident_reclude () override {
        the_player = null;
    }
    void Resident_before_step () override;
    void Resident_collide (Resident&) override;
    void Resident_draw () override;
    ~Player () { the_player = null; }
};

} // vf
