#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Activatable {
    virtual void Activatable_activate () = 0;
};

struct Switch : Resident {
    Hitbox hb;
    TexFrameSound* data;
    Activatable* target = null;
    uint32 delay = 30;
    uint32 timer = 0;
    uint32 cooldown = 0;
    void init ();
    void Resident_before_step () override;
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
    void Resident_draw () override;
};

} // vf
