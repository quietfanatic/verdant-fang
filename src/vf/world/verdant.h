#pragma once
#include "../game/room.h"
#include "common.h"
#include "walker.h"

namespace vf {

struct Verdant;
inline Verdant* main_character = null;

struct Verdant : Walker {

    Verdant ();
    ~Verdant () { main_character = null; }

    void Resident_on_enter () override {
        expect(!main_character); main_character = this;
    }
    void Resident_on_exit () override {
        main_character = null;
    }
};

} // vf
