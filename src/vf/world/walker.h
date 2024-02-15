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
 //    [WalkerState]
 //          |
 //    Walker_business()  Mind_think()
 //          |               |
 //    [WalkerBusiness]  [Controls]
 //              \        /
 //       fixed movement routines
 //                  |
 //           [pos, hitboxes]

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
    constexpr WalkerState Dead = 4;
    constexpr WalkerState Custom = 5;
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
    float z = Z::Actor;
    bool damage_overlap = false;
    uint8 body_layers = 0xff;
};

struct WalkerPoses {
    Pose stand;
    Pose crouch;
     // Six frame walk cycle
    Pose walk [6];
     // vel positive, vel neutral, and two vel negative alternating.
    Pose jump [4];
     // Neutral, forward, backward
    Pose fly [3];
     // Occupied, interruptible
    Pose land [2];
     // Preattack (before hold), preattack (hold), attack active, attack
     // inactive, postattack occupied, postattack interruptible
    Pose attack [6];
    Pose crouch_attack [6];
    Pose hit [2];
    Pose damage;
    Pose damagefall;
    Pose dead [1];
};

enum class WalkerFlavor {
    Strawberry,
    Lemon
};
using WF = WalkerFlavor;

struct WalkerData {
    Rect body_box;
    Rect crouch_body_box;
    Rect dead_body_box;
    Rect damage_box;
    Rect crouch_damage_box;
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
    float gravity_fly;
    bool can_fly;
    uint8 drop_duration;
     // [0] is occupied.
     // [1] is interruptible
    uint8 land_sequence [2];
     // [0] is preattack.  Phase will not advance to 1 until the attack button
     // is released.
     // [1] is preattack.  Attack is imminent.
     // [2] is when the attack hitbox is active.
     // [3] is after the attack hitbox but the attack frame is still visible.
     // [4] is postattack, occupied.
     // [5] is postattack, interruptible.
    uint8 attack_sequence [6];
     // Length of time to freeze when hitting a target.  Phases differ only in
     // animation.
    uint8 hit_sequence [2];
     // [0] + [1] is freeze frames during hit.  These should match opponent's
     // hit_sequence so you're both frozen for the same time.  pose = damage
     // [2] is before starting to fall.  pose = damage
     // [3] is minimum time to fall before landing.  Phase won't advance to 4
     // until floor is true.  pose = damagefall
     // [4]..[10] are laying down, varying only in decals.  pose = dead
    uint8 dead_sequence [11];
     // When crouching in midair, lift feet this many pixels
    uint8 jump_crouch_lift;
     // Applied between phases 2 and 3
    uint8 dead_floor_lift;
     // Allow jumping and attacking if the button has been held this many frames
    uint8 hold_buffer;
     // For animation
    float walk_cycle_dist;
    float fall_cycle_dist;
    float jump_end_vel;
    float fall_start_vel;
    Sound* step_sound [5];
    Sound* land_sound;
    Sound* attack_sound;
    Sound* crouch_attack_sound;
    Sound* hit_solid_sound;
    Sound* hit_soft_sound;
    WalkerPoses* poses;
    DecalData* decals;
    WalkerFlavor flavor;
};

struct Walker : Resident {
    WalkerData* data = null;
    Mind* mind = null;
    Vec vel;
    bool left = false;
    bool crouch = false;
    bool fly = false;
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
     // Bitset, each bit corresponding to a layer in the body LayeredTexture.
    uint8 body_layers = 0x1;
    uint8 head_layers = 0x1;
    uint8 weapon_layers = 0x1;
     // For the rare case where two walkers kill eachother on the same frame,
     // show them in hit[0] instead of dead[0]
    bool mutual_kill = false;

     // Everything below here is only used within one frame, so don't serialize
     // any of it.
    bool invincible = false;
     // Process recoil after collisions so we don't stack multiple recoils
    bool do_recoil = false;
     // Allow one hit sound to override another
    Sound* hit_sound;
    Resident* new_floor;
    Hitbox body_hb;
    Hitbox damage_hb;
    Hitbox weapon_hb;

    WalkerBusiness business;

    Walker ();

    void set_state (WalkerState);
    uint8 walk_frame ();
    uint8 jump_frame ();

     // Land on solid object
    void set_floor (Resident& o);

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
     // Handle posing for custom states here, otherwise supercall.
    virtual Pose Walker_pose ();
     // Override drawing weapon.  Currently only used for making Fang hover
     // during the transformation scene.
    virtual void Walker_draw_weapon (const Pose& p);

     // Useful
    float left_flip (float v) const { return left ? -v : v; }
    Vec left_flip (Vec v) const { return left ? Vec(-v.x, v.y) : v; }
};

} // vf
