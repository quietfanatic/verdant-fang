#pragma once

#include "../../dirt/glow/image-texture.h"
#include "../game/controls.h"
#include "../game/frame.h"
#include "../game/sound.h"
#include "../game/room.h"
#include "common.h"
#include "decals.h"

namespace vf {

enum class WalkerState {
    Neutral,
    Crouch,
    Land,
    Attack,
    Hit,
    Damage,
    Dead,
};
using WS = WalkerState;

struct BodyFrame : Frame {
    Vec head;
    Vec weapon = GNAN;
    Vec decals [4] = {GNAN, GNAN, GNAN, GNAN};
    uint8 decal_dirs [4] = {0, 0, 0, 0};
};

struct WeaponFrame : Frame {
    Rect hitbox;
};

struct Pose {
    BodyFrame* body = null;
    Frame* head = null;
    WeaponFrame* weapon = null;
};

struct WalkerPoses {
    Pose stand;
    Pose crouch;
    Pose walk [6];
    Pose jump [4];
    Pose land [2];
    Pose attack [4];
    Pose damage [2];
    Pose dead;
};

struct WalkerPhys {
    Rect body_box;
    Rect damage_box;
    float ground_acc;
    float ground_max;
    float ground_dec;
    float coast_dec;
    float air_acc;
    float air_max;
    float air_dec;
    float jump_vel;
    float gravity_jump;
    float gravity_fall;
    float gravity_drop;
    uint8 drop_duration;
    uint8 land_sequence [2];
    uint8 attack_sequence [4];
    uint8 hit_sequence;
     // [0] + [1] should match opponent's hit_sequence.  TODO: link them
     // [2] is before starting to fall.
     // [3] is minimum time to fall before landing.
    uint8 damage_sequence [4];
    uint8 dead_sequence [7];
    uint8 jump_crouch_lift;
    uint8 dead_floor_lift;
    uint8 hold_buffer;
     // For animation
    float walk_cycle_dist;
    float fall_cycle_dist;
    float jump_end_vel;
    float fall_start_vel;
};

struct WalkerSfx {
    Sound* step [5];
    Sound* land;
    Sound* attack;
    Sound* hit_solid;
    Sound* hit_soft;
    Sound* unhit = null;
};

struct WalkerData {
    WalkerPhys phys;
    glow::PixelTexture* body_tex;
    glow::PixelTexture* head_tex;
    glow::PixelTexture* weapon_tex;
    DecalData* decals;
    WalkerPoses* poses;
    WalkerSfx sfx;
};

struct Walker : Resident {
    WalkerData* data = null;
    Mind* mind = null;
    Vec vel;
    bool left = false;
    WalkerState state = WS::Neutral;
    uint8 anim_phase = 0;
    uint32 anim_timer = 0;
     // More gravity a few frames after releasing jump
    uint32 drop_timer = 0;
     // For animations
    float walk_start_x = GNAN;
    float fall_start_y = GNAN;
    DecalType decal_type = {};
    uint8 decal_index = -1;

    Resident* floor = null;
     // Temporary
    Resident* new_floor;
     // body, damage, weapon
    Hitbox hbs [3];

    enum class Business {
        Free,
        Interruptible,
        Occupied,
        Frozen
    };
    using B = Business;
    Business business;

    Walker ();

    void set_state (WalkerState);
    uint8 walk_frame ();
    uint8 jump_frame ();

    void Resident_before_step ();
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
    void Resident_after_step () override;
    void Resident_draw () override;
     // Customization points.
     // You can supercall this or not.
    virtual void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&);
};

} // vf
