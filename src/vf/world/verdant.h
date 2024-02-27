#pragma once
#include "../game/controls.h"
#include "../game/room.h"
#include "common.h"
#include "indigo.h"
#include "walker.h"

namespace vf {
struct Door;

namespace VS {
    constexpr WalkerState PreTransform = WS::Custom + 0;
    constexpr WalkerState Transform = WS::Custom + 1;
    constexpr WalkerState FangHelp = WS::Custom + 2;
    constexpr WalkerState Captured = WS::Custom + 3;
    constexpr WalkerState CapturedWeaponTaken = WS::Custom + 4;
    constexpr WalkerState CapturedWeaponBroken = WS::Custom + 5;
    constexpr WalkerState CapturedLimbsDetached = WS::Custom + 6;
    constexpr WalkerState Limbless = WS::Custom + 7;
    constexpr WalkerState Inch = WS::Custom + 8;
    constexpr WalkerState Snakify = WS::Custom + 9;
    constexpr WalkerState Snake = WS::Custom + 10;
    constexpr WalkerState SnakeAttack = WS::Custom + 11;
    constexpr WalkerState SnakeBite = WS::Custom + 12;
    constexpr WalkerState SnakeCaptured = WS::Custom + 13;
    constexpr WalkerState SnakeEat = WS::Custom + 14;
    constexpr WalkerState Desnakify = WS::Custom + 15;
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

namespace DesnakifyPhase {
    constexpr uint8 FullGlow0 = 0;
    constexpr uint8 FullGlow1 = 1;
    constexpr uint8 FullGlow2 = 2;
    constexpr uint8 FullScreenGlow0 = 3;
    constexpr uint8 FullScreenGlow1 = 4;
    constexpr uint8 FullScreenGlow2 = 5;
    constexpr uint8 FangRevived0 = 6;
    constexpr uint8 FangRevived1 = 7;
    constexpr uint8 FangRevived2 = 8;
    constexpr uint8 FangFloats0 = 9;
    constexpr uint8 FangFloats1 = 10;
    constexpr uint8 FangFloats2 = 11;
    constexpr uint8 FangGlow0 = 12;
    constexpr uint8 FangGlow1 = 13;
    constexpr uint8 FangGlow2 = 14;
    constexpr uint8 FangScreenGlow0 = 15;
    constexpr uint8 FangScreenGlow1 = 16;
    constexpr uint8 FangScreenGlow2 = 17;
    constexpr uint8 BackToHuman0 = 18;
    constexpr uint8 BackToHuman1 = 19;
    constexpr uint8 BackToHuman2 = 20;
    constexpr uint8 TakeFang0 = 21;
    constexpr uint8 TakeFang1 = 22;
    constexpr uint8 HoldFang = 23;
    constexpr uint8 WalkAway = 24;
    constexpr uint8 End = 25;
    constexpr uint8 N_Phases = 26;
};
namespace DP = DesnakifyPhase;

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
    Pose captured;
    Pose captured_damage;
    Pose weapon_taken [3];
    Pose weapon_broken [9];
    Pose limbs_detached;
    Pose limbs_taken;
    Pose limbless_fall;
    Pose limbless;
    LimbFrame* captured_limbs [4];
    Pose inch [3];
    Frame* floor_clothes;
    Pose snake_stand;
    Pose snake_walk [4];
    Pose snake_attack [6];
    Pose snake_bite [9];
    Pose snake_captured;
    Pose eat [34];
    Pose desnakify [DP::N_Phases];
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
    uint8 weapon_taken_sequence [3];
    uint8 weapon_broken_sequence [8];
    float fang_gravity;
    float fang_dead_y;
     // For drawing decals, repeated for each limb
    uint8 limb_detach_sequence [3];
    uint8 limbless_sequence;
     // Indexed by anim_phase-1
    uint8 inch_sequence [2];
     // 0 1 2 = self and fang glow
     // 3 4 5 = screen glow
     // 6 7 = screen glow fades
    uint8 snakify_sequence [8];
    Rect snake_box;
    Rect snake_attack_box;
    float snake_acc;
    float snake_max;
    float snake_dec;
    float snake_walk_cycle_dist;
    float snake_jump_vel;
     // Matches attack_sequence in meaning
    uint8 snake_attack_sequence [6];
    Vec snake_attack_vel;
    uint8 snake_tongue_cycle [5];
    Music* music_after_transform = null;
    Music* music_after_snakify = null;
    Music* music_after_desnakify = null;
     // Indexed by Indigo's anim_phase
     // Dunno if storing this here is right but we're doing it
    Vec bite_indigo_offsets [9];
    Vec bite_release_vel;
    uint8 eat_sequence [34];
    uint8 desnakify_sequence [DP::N_Phases - 1];
    Vec desnakify_fang_pos [2];
    Vec desnakify_limb_offsets [4];
    Sound* unstab_sound = null;
    Sound* revive_sound = null;
    Sound* spear_break_sound = null;
    Sound* snake_death_sound = null;
    Sound* limb_detach_sound = null;
    Sound* snake_bite_sound = null;
    RandomSound* snake_eat_sound = null;
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
    uint8 limb_layers = 1;
    uint8 floor_clothes_layers = 0;
    Vec limb_pos [4] = {GNAN, GNAN, GNAN, GNAN};
    Vec limb_initial_pos [4] = {GNAN, GNAN, GNAN, GNAN};
    Vec floor_decal_pos = GNAN;
    Room* floor_clothes_room = null;
    Vec floor_clothes_pos = GNAN;
    glow::RGBA8 limb_tint = {};
    Indigo* indigo = null;
    float fang_vel_y = GNAN;
    uint32 tongue_timer = 0;
    Door* desnakify_leaving_door = null;

    Verdant ();

     // Relative to self.pos
    Vec visual_center ();

    void go_to_limbo ();
    void revive ();
    void animate_tongue ();

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
