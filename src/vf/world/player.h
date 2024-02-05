#pragma once
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Player;
inline Player* the_player = null;

struct Player : Walker {
    struct Fang : Resident {
        Fang ();
        void Resident_collide (Resident&) override;
    };
     // Temporary.  The attack hitbox is only active for one frame, and will be
     // deactivated before the frame is over, leaving no serializable state.
    Fang fang;

    Player ();
    ~Player () { the_player = null; }

    void Resident_emerge () override {
        expect(!the_player); the_player = this;
    }
    void Resident_reclude () override {
        the_player = null;
    }
     // Override these for weapon management
    void Resident_before_step () override;
    void fang_collide (Resident&);
    void Resident_after_step () override;
};

} // vf
