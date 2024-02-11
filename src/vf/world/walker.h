#pragma once

#include "../../dirt/glow/image-texture.h"
#include "../game/controls.h"
#include "../game/render.h"
#include "../game/sound.h"
#include "../game/room.h"
#include "common.h"
#include "decals.h"

namespace vf {

 // The data flow for the walker class looks like this.
 //
 //    WalkerState
 //         |
 //    WalkerBusiness  Controls
 //              \      /
 //              movement

 // WalkerState indicates roughly what state the character is in between frames.
 // Each state has an associated anim_phase and anim_timer, and what values are
 // valid for those depend on the state.  This does not include whether the
 // walker is grounded (this is covered by .floor) or whether the walker is
 // crouching (.crouch).
using WalkerState = uint8;
namespace WS {
    constexpr WalkerState Neutral = 0;
    constexpr WalkerState Land = 1;
    constexpr WalkerState Attack = 2;
    constexpr WalkerState Hit = 3;
    constexpr WalkerState Damage = 4;
    constexpr WalkerState Dead = 5;
    constexpr WalkerState Custom = 6;
}

 // WalkerBusiness indicates how "busy" this character is, which encompasses
 // whether they can move, whether they can attack or be attacked, etc.
enum class WalkerBusiness {
     // Do whatever you want
    Free,
     // Do whatever you want, but set state to Neutral first
    Interruptible,
     // You can't do anything except release the attack button to advance attack
     // animation.
    HoldAttack,
     // You can't do anything, but the attack hitbox will be active.
    DoAttack,
     // You can't do anything.
    Occupied,
     // You are stuck in place.  You can't even fall.
    Frozen,
};
using WB = WalkerBusiness;

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
    Pose attack [6];
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
    float gravity_damage;
    uint8 drop_duration;
    uint8 land_sequence [2];
     // [0] is preattack.  Phase will not advance to 1 until the attack button
     // is released.
     // [1] is preattack.  Attack is imminent.
     // [2] is when the attack hitbox is active.
     // [3] is after the attack hitbox but the attack frame is still visible.
     // [4] is postattack, occupied.
     // [5] is postattack, interruptible.
    uint8 attack_sequence [6];
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
    DecalData* decals;
    WalkerPoses* poses;
    WalkerSfx sfx;
};

struct Walker : Resident {
    WalkerData* data = null;
    Mind* mind = null;
    Vec vel;
    bool left = false;
    bool crouch = false;
    WalkerState state = WS::Neutral;
     // Valid values of this depend on state.
    uint8 anim_phase = 0;
     // This ranges from 0 to n where n is probably a *_sequence array element
     // in the WalkerData.  0 typically means the animation hasn't been
     // processed at all yet, so the normal range for anim_timer is 1..n, thus
     // performing the animation for n frames.  If you set the state in
     // Resident_before_move, you should recurse to Walker_business.  This way,
     // animation limits of 0 will be properly respected and skip the animation
     // phase entirely.
    uint32 anim_timer = 0;

     // More gravity a few frames after releasing jump
    uint32 drop_timer = 0;
    Resident* floor = null;
     // For animations
    float walk_start_x = GNAN;
    float fall_start_y = GNAN;
    DecalType decal_type = {};
    uint8 decal_index = -1;
    uint8 weapon_state = 0;

     // Everything below here is only used within one frame, so don't serialize
     // it.
    Sound* hit_sound;
    Resident* new_floor;
     // body, damage, weapon
    Hitbox hbs [3];

    WalkerBusiness business;

    Walker ();

    void set_state (WalkerState);
    uint8 walk_frame ();
    uint8 jump_frame ();

     // Hit solid object
    void recoil ();

    void Resident_set_pos (Vec) override;
    void Resident_before_step ();
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
    void Resident_after_step () override;
    void Resident_draw () override;
    void Resident_on_exit () override;
     // Customization points.
     // Handle custom states here, and supercall otherwise.
    virtual WalkerBusiness Walker_business ();
     // You can supercall this or not.
    virtual void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&);

     // Useful
    float left_flip (float v) const { return left ? -v : v; }
    Vec left_flip (Vec v) const { return left ? Vec(-v.x, v.y) : v; }
};

} // vf
