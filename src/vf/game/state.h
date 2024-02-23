#pragma once
#include <random>
#include "../../dirt/ayu/resources/document.h"
#include "../../dirt/ayu/resources/global.h"
#include "camera.h"
#include "common.h"
#include "sound.h"

namespace control { struct Statement; }

namespace vf {

struct Transition {
    Room* target_room = null;
    Resident* migrant = null;
    Vec target_pos = GNAN;
    TransitionType type = TransitionType::WipeLeft;
    bool save_checkpoint = false;
    bool load_checkpoint = false;
    uint32 checkpoint_level = 2;
    Vec checkpoint_transition_center = camera_size / 2;
    uint32 exit_at = 6;
    uint32 enter_at = 20;
    uint32 end_at = 30;
    uint32 timer = 0;
    float start_music_volume = GNAN;
    float end_music_volume = GNAN;

    bool step (State&);
};

 // Checkpoints only save the world and current room.  They don't save rng, so
 // if you die the rng will be different.
struct Checkpoint {
    ayu::SharedLocation current_room;
    Music* current_music;
    uint32 checkpoint_level;
    Vec transition_center;
    ayu::Tree world;
};

struct State {
    union {
        std::minstd_rand rng;
        uint32 rng_uint32;
    };
    uint64 current_frame = 0;
    Room* current_room = null;
    Music* current_music = null;
    double current_music_position = 0;
    std::optional<Transition> transition;

    ayu::Document world;

    std::optional<Checkpoint> checkpoint;

    State ();
     // Starts playing music when loading a saved State.  You don't need to
     // bother with this if you're manually constructing a State.
    void init ();
    void load_initial ();
    void step ();

     // Both of these require that there is no ongoing transition.
    void save_checkpoint (Vec transition_center = camera_size / 2);
     // This starts a 30-frame aperture close transition.
    void load_checkpoint ();
};

} // vf
