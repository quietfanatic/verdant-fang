#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Activatable {
    virtual void Activatable_activate () = 0;
};

struct SwitchData;
struct Switch : Resident, Activatable {
    Hitbox hb;
    SwitchData* data;
    UniqueArray<Activatable*> activate;
    uint32 delay = 30;
    uint32 timer = 0;
    uint32 cooldown = 0;
    bool active = true;
    void init ();
    void Resident_before_step () override;
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
    void Resident_draw () override;
    void Activatable_activate () override;
};

} // vf
