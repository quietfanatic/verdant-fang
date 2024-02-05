#pragma once

#include "../../dirt/glow/file-image.h"
#include "../../dirt/glow/image-texture.h"
#include "../game/controls.h"
#include "../game/frame.h"
#include "../game/sound.h"
#include "../game/room.h"
#include "common.h"

namespace vf {

enum class WalkerState {
    Neutral,
    Crouch,
    Land,
    Attack,
};
using WS = WalkerState;

struct BodyFrame : Frame {
    Vec head;
    Vec weapon = GNAN;
};

struct BodyFrames {
    BodyFrame stand;
    BodyFrame crouch;
    BodyFrame walk [6];
    BodyFrame fall1;
    BodyFrame land;
    BodyFrame attack [3];
};

struct HeadFrames {
    Frame neutral;
    Frame wave [3];
    Frame fall [2];
    Frame back;
};

struct Pose {
    BodyFrame* body = null;;
    Frame* head = null;
};

struct Poses {
    Pose stand;
    Pose crouch;
    Pose walk [6];
    Pose jump [4];
    Pose land [2];
    Pose attack [4];
};

struct WalkerPhys {
    float ground_acc;
    float ground_max;
    float ground_dec;
    float coast_dec;
    float air_acc;
    float air_max;
    float air_dec;
    float jump_vel;
    float jump_end_vel;
    float fall_start_vel;
    float gravity_jump;
    float gravity_fall;
    float gravity_drop;
    uint8 drop_duration;
    uint8 attack_sequence [4];
    uint8 land_sequence [2];
    uint8 hold_buffer;
};

struct WalkerData {
    WalkerPhys phys;
    glow::FileImage img;
    glow::PixelTexture body_tex;
    glow::PixelTexture head_tex;
    BodyFrames bodies;
    HeadFrames heads;
    Poses poses;
    Sound step_sfx [5];
    Sound land_sfx;
    Sound attack_sfx;
    Sound hit_solid_sfx;
};

struct Walker : Resident {
    WalkerData* data = null;
    Mind* mind = null;
    Vec vel;
    bool left = false;
    WalkerState state = WS::Neutral;
    uint8 anim_phase = 0;
    uint32 anim_timer = 0;
    uint32 drop_timer = 0;
     // More gravity a few frames after releasing jump
     // For animations
    float walk_start_x = GNAN;
    float fall_start_y = GNAN;

    Block* floor = null;
     // Temporary
    Block* new_floor;

    Walker ();

    void set_state (WalkerState);
    uint8 walk_frame ();
    uint8 jump_frame ();

    void Resident_before_step ();
    void Resident_collide (Resident& other) override;
    void Resident_after_step () override;
    void Resident_draw () override;
};

} // vf
