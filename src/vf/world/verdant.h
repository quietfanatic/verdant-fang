#pragma once
#include "../game/controls.h"
#include "../game/room.h"
#include "common.h"
#include "indigo.h"
#include "walker.h"

namespace vf {

namespace VS {
    constexpr WalkerState PreTransform = WS::Custom + 0;
    constexpr WalkerState Transform = WS::Custom + 1;
    constexpr WalkerState FangHelp = WS::Custom + 2;
    constexpr WalkerState Captured = WS::Custom + 3;
    constexpr WalkerState Inch = WS::Custom + 4;
    constexpr WalkerState Snakify = WS::Custom + 5;
    constexpr WalkerState Snake = WS::Custom + 6;
     // Update world.ayu if this changes
    static_assert(PreTransform == 6);
};

namespace TransformPhase {
    constexpr uint8 Holding = 0;
    constexpr uint8 Lifting = 1;
    constexpr uint8 Waiting = 2;
    constexpr uint8 Dissolving0 = 3;
    constexpr uint8 Dissolving1 = 4;
    constexpr uint8 Dissolving2 = 5;
    constexpr uint8 Naked = 6;
    constexpr uint8 Assembling0 = 7;
    constexpr uint8 Assembling1 = 8;
    constexpr uint8 Assembling2 = 9;
    constexpr uint8 Settling0 = 10;
    constexpr uint8 Settling1 = 11;
    constexpr uint8 Settling2 = 12;
    constexpr uint8 Receiving = 13;
    constexpr uint8 N_Phases = 14;
};
namespace TP = TransformPhase;

struct LimbFrame : Frame {
     // Relative to body pos
    Vec attached;
    Vec detached;
    float z_offset;
};

struct VerdantPoses : WalkerPoses {
    Pose walk_hold [6];
    Pose transform [TP::N_Phases];
    Pose damagef;
    Pose damagefallf;
    Pose downf;
    Pose deadf [7];
    Frame* fang_help [6];
    Pose captured [CP::N_Phases];
    LimbFrame* captured_limbs [4];
    Pose inch [3];
    Pose snake_stand;
    Pose snake_walk [4];
};

struct CutsceneSound {
    uint8 phase;
    uint8 timer;
    Sound* sound;
};

struct VerdantData : WalkerData {
    Vec center;
    Vec dead_center;
    Vec dead_center_forward;
    uint8 transform_sequence [TP::N_Phases];
    Vec transform_pos;
    glow::RGBA8 transform_magic_color;
    CutsceneSound transform_sounds [3];
     // Phases are:
     // 0 = spear form
     // 1 = spear form glow
     // 2 = snake form long
     // 3 = snake form uncurling
     // 4 = snake form normal (still rising)
     // 5 = snake_form_normal (timer stops)
    uint8 fang_help_sequence [5];
    uint8 revive_sequence [5];
    glow::RGBA8 revive_tint [6];
    uint8 captured_sequence [CP::N_Phases];
    uint8 captured_limb_phases [4];
    Vec captured_pos;
    Vec captured_fang_pos_high;
    Vec captured_fang_pos_low;
     // Indexed by anim_phase-1
    uint8 inch_sequence [2];
     // 0 1 2 = self and fang glow
     // 3 4 5 = screen glow
     // 6 7 = screen glow fades
    uint8 snakify_sequence [8];
    Rect snake_box;
    float snake_acc;
    float snake_max;
    float snake_dec;
    float snake_walk_cycle_dist;
    float snake_jump_vel;
    Music* music_after_transform = null;
    Sound* unstab_sound = null;
    Sound* revive_sound = null;
    Sound* spear_break_sound = null;
    Sound* snake_death_sound = null;
    Sound* limb_detach_sound = null;
};

struct Verdant : Walker {
    bool damage_forward = false;
     // Total transformation time, for hair animation
    uint32 transform_timer = 0;
    Room* limbo = null;
     // Keep revive counters separate from anim counters because with force
     // restart and limbo animations, you can revive in any state.
    uint8 revive_phase = 0;
    uint8 revive_timer = 0;

    Vec captured_initial_pos;
    Vec limb_pos [4];

    Verdant ();

     // Relative to self.pos
    Vec visual_center ();

    void go_to_limbo ();

     // For handling custom hit animations
    WalkerBusiness Walker_business () override;
     // For moving during captured cutscene
    void Walker_move (const Controls&) override;
     // For VS::Snake hitbox
    void Walker_set_hitboxes () override;
     // Allow getting crushed by door
    void Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) override;
     // Set decals on victim
    void Walker_on_hit (const Hitbox&, Walker&, const Hitbox&) override;
     // For drawing damage forward frames
    Pose Walker_pose () override;
     // For handling extra drawing during cutscenes
    void Walker_draw_weapon (const Pose&) override;
     // Reset decals
    void Resident_on_exit () override;
};

 // Overrides movement during pre-transformation cutscene
struct VerdantMind : Mind {
    Mind* next;
    Controls Mind_think (Resident&) override;
};

Verdant* find_verdant ();

} // vf
