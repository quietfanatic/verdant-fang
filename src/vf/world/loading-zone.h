#pragma once
#include "../game/camera.h"
#include "../game/room.h"
#include "common.h"

namespace vf {
struct Verdant;

struct LoadingZone : Resident {
    Hitbox hb;
    Room* target_room;
    Vec target_pos;
    TransitionType transition = TransitionType::WipeLeft;
     // Trigger a checkpoint if frustration is at most this value.
    uint8 checkpoint_level = 0;
    LoadingZone ();
    void trigger (Verdant&);
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&);
};

} // vf
