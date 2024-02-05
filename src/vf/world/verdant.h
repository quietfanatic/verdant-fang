#pragma once
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Verdant;
inline Verdant* main_character = null;

struct Verdant : Walker {
    struct Fang : Resident {
        Fang ();
        void Resident_collide (Resident&) override;
    };
     // Temporary.  The attack hitbox is only active for one frame, and will be
     // deactivated before the frame is over, leaving no serializable state.
    Fang fang;

    Verdant ();
    ~Verdant () { main_character = null; }

    void Resident_emerge () override {
        expect(!main_character); main_character = this;
    }
    void Resident_reclude () override {
        main_character = null;
    }
     // Override these for weapon management
    void Resident_before_step () override;
    void fang_collide (Resident&);
    void Resident_after_step () override;
};

} // vf
