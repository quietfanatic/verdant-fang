#pragma once
#include "../game/room.h"
#include "common.h"

namespace vf {

struct Player;
inline Player* the_player = null;

enum class PlayerState {
    Neutral,
    Attack,
};
using PS = PlayerState;

struct PlayerData;

struct Player : Resident {
    PlayerData* data = null;
    Vec vel;
    bool left = false;
    PlayerState state = PS::Neutral;
    uint8 anim_phase = 0;
    uint32 anim_timer = 0;
    float walk_start_x = GNAN;

    Block* floor = null;
     // Temporary
    Block* new_floor;

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
