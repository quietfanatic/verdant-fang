#pragma once
#include "common.h"
#include "frame.h"
#include "room.h"

namespace vf {

inline Player* the_player = null;

struct PlayerFrames {
    Frame standing;
};

struct Player : Resident {
    Vec vel;
    bool standing = false;
     // Temporary
    Block* floor;

    Player ();
    ~Player () { the_player = null; }
    void Resident_emerge () override {
        expect(!the_player); the_player = this;
    }
    void Resident_reclude () override {
        the_player = null;
    }
    void Resident_before_step () override;
    void Resident_collide (Resident&) override;
    void Resident_after_step () override;
    void Resident_draw () override;
};

} // vf
