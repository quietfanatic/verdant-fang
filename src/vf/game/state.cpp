#include "state.h"

#include <SDL2/SDL_mixer.h>
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/ayu/traversal/scan.h"
#include "../../dirt/ayu/traversal/to-tree.h"
#include "../../dirt/control/command.h"
#include "../../dirt/iri/iri.h"
#include "camera.h"
#include "game.h"
#include "options.h"
#include "room.h"

namespace vf {

 // If this returns false, don't step the room.
bool Transition::step (State& state) {
    if (state.current_music &&
        defined(start_music_volume) &&
        defined(end_music_volume)
    ) {
        Mix_VolumeMusic(
            round(lerp(start_music_volume,
                 end_music_volume,
                 float(timer) / end_at
            ) * 128)
        );
    }
    if (timer == 0) {
        set_transition_type(type);
        set_transition_side(true);
        if (load_checkpoint) {
            set_transition_center(state.checkpoint
                ? state.checkpoint->transition_center
                : camera_size / 2
            );
        }
         // fallthrough
    }
    if (timer < exit_at) {
        timer += 1;
        set_transition_t(float(timer) / end_at);
        return true;
    }
    else if (timer == exit_at) {
        if (target_room) {
            state.current_room->exit();
            state.current_room = target_room;
            if (migrant) {
                migrant->set_room(state.current_room);
                migrant->set_pos(target_pos);
            }
            state.current_room->enter();
        }
        swap_world_tex();
        set_transition_side(false);
        if (save_checkpoint) {
            ayu::KeepLocationCache klc;
            state.checkpoint = Checkpoint{
                .current_room = ayu::reference_to_location(state.current_room),
                .current_music = state.current_music,
                .checkpoint_level = checkpoint_level,
                .transition_center = checkpoint_transition_center,
                .world = ayu::item_to_tree(&state.world),
            };
        }
        if (load_checkpoint) {
            auto& frustration = current_game->options().frustration;
            if (state.checkpoint &&
                state.checkpoint->checkpoint_level >= frustration
            ) {
                ayu::item_from_tree(&state.world, state.checkpoint->world);
                 // Do this AFTER item_from_tree to reference the new world and
                 // not the old world.
                state.current_room = ayu::reference_from_location(
                    state.checkpoint->current_room
                );
                if (state.current_music != state.checkpoint->current_music) {
                    if (state.current_music &&
                        !state.checkpoint->current_music
                    ) {
                        state.current_music->stop();
                    }
                    state.current_music = state.checkpoint->current_music;
                    if (state.current_music) state.current_music->play();
                }
                if (state.checkpoint->checkpoint_level >= 3) {
                    state.lost_hardcore = false;
                }
            }
            else {
                 // No checkpoint?  Uh...reload from initial world?
                if (state.current_music) state.current_music->stop();
                state.load_initial();
            }
        }
         // fallthrough
    }
    if (timer < enter_at) {
        timer += 1;
        set_transition_t(float(timer) / end_at);
        return false;
    }
    else if (timer < end_at) {
        timer += 1;
        set_transition_t(float(timer) / end_at);
        return true;
    }
    else {
         // Delete self
        set_transition_t(GNAN);
        state.transition = {};
        return true;
    }
}

State::State () : rng(0) { }

void State::init () {
    if (current_music) {
         // SDL2_mixer doesn't provide any way of acquiring a lock, so
         // manipulating the music position is full of race conditions, yay!
        current_music->play();
        glow::require_sdl(Mix_SetMusicPosition(current_music_position) == 0);
    }
}

void State::load_initial () {
    ayu::SharedResource initial (iri::constant("res:/vf/world/world.ayu"));
    Resident* v = initial["verdant"][1];
    if (v) {
        current_room = v->room;
    }
    else current_room = initial["start"][1];
    world = move(initial->value().as<ayu::Document>());
    ayu::force_unload(initial);
    lost_hardcore = false;
}

void State::step () {
    if (current_music && defined(current_music->loop_end)) {
         // Race condition with audio thread because SDL_Mixer provides no way
         // to sync with it.  Also Mix_GetMusicPosition takes a Mix_Music* but
         // Mix_SetMusicPosition doesn't, go figure.
//        double pos = Mix_GetMusicPosition(current_music);
//        if (pos > current_music->loop_end) {
//            glow::require_sdl(Mix_SetMusicPosition(pos) >= 0);
//        }
//        current_music_position = pos;
    }
    bool should_step = true;
    if (transition) should_step = transition->step(*this);
    if (should_step) {
        if (current_room) current_room->step();
    }
    frame_count += 1;
}

void State::save_checkpoint (uint8 checkpoint_level, Vec transition_center) {
    //expect(!transition);
    transition = Transition{
        .save_checkpoint = true,
        .checkpoint_level = checkpoint_level,
        .checkpoint_transition_center = transition_center,
        .exit_at = 0,
        .enter_at = 0,
        .end_at = 0,
    };
}

void State::load_checkpoint () {
    //expect(!transition);
    transition = Transition{
        .type = TransitionType::ApertureOpen,
        .load_checkpoint = true,
        .exit_at = 0,
        .enter_at = 20,
        .end_at = 30,
        .start_music_volume = current_music ? current_music->volume / 2 : GNAN,
        .end_music_volume = current_music ? current_music->volume : GNAN,
    };
}

} using namespace vf;

AYU_DESCRIBE(vf::Transition,
    attrs(
        attr("target_room", &Transition::target_room),
        attr("migrant", &Transition::migrant),
        attr("target_pos", &Transition::target_pos),
        attr("type", &Transition::type),
        attr("save_checkpoint", &Transition::save_checkpoint),
        attr("load_checkpoint", &Transition::load_checkpoint),
        attr("checkpoint_level", &Transition::checkpoint_level),
        attr("checkpoint_transition_center", &Transition::checkpoint_transition_center),
        attr("exit_at", &Transition::exit_at),
        attr("enter_at", &Transition::enter_at),
        attr("end_at", &Transition::end_at),
        attr("timer", &Transition::timer),
        attr("start_music_volume", &Transition::start_music_volume),
        attr("end_music_volume", &Transition::end_music_volume)
    )
)

AYU_DESCRIBE(vf::Checkpoint,
    attrs(
        attr("current_room", &Checkpoint::current_room),
        attr("current_music", &Checkpoint::current_music),
        attr("checkpoint_level", &Checkpoint::checkpoint_level),
        attr("transition_center", &Checkpoint::transition_center),
        attr("world", &Checkpoint::world)
    )
)

AYU_DESCRIBE(vf::State,
    attrs(
        attr("rng", member(&State::rng_uint32, prefer_hex), optional),
        attr("frame_count", &State::frame_count, optional),
        attr("current_room", &State::current_room),
        attr("current_music", &State::current_music),
        attr("current_music_position", &State::current_music_position),
        attr("transition", &State::transition, collapse_optional),
        attr("world", &State::world),
        attr("checkpoint", &State::checkpoint, collapse_optional),
        attr("lost_hardcore", &State::lost_hardcore, optional)
    ),
     // This needs to happen after the music loads itself.
    init<&State::init>(-10)
);
