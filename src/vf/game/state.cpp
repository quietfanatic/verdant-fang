#include "state.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/ayu/traversal/scan.h"
#include "../../dirt/ayu/traversal/to-tree.h"
#include "../../dirt/control/command.h"
#include "../../dirt/iri/iri.h"
#include "camera.h"
#include "game.h"
#include "room.h"

namespace vf {

 // If this returns false, don't step the room.
bool Transition::step (State& state) {
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
                .transition_center = checkpoint_transition_center,
                .world = ayu::item_to_tree(&state.world),
            };
        }
        if (load_checkpoint) {
            if (state.checkpoint) {
                ayu::item_from_tree(&state.world, state.checkpoint->world);
                 // Do this AFTER item_from_tree to reference the new world and
                 // not the old world.
                state.current_room = ayu::reference_from_location(
                    state.checkpoint->current_room
                );
            }
            else {
                 // No checkpoint?  Uh...reload from initial world?
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

void State::load_initial () {
    ayu::SharedResource initial (iri::constant("res:/vf/world/world.ayu"));
    Resident* v = initial["verdant"][1];
    if (v) {
        current_room = v->room;
    }
    else current_room = initial["start"][1];
    world = move(initial->value().as<ayu::Document>());
    ayu::force_unload(initial);
}

void State::step () {
    bool should_step = true;
    if (transition) should_step = transition->step(*this);
    if (should_step) {
        if (current_room) current_room->step();
    }
    current_frame += 1;
}

void State::save_checkpoint (Vec transition_center) {
    expect(!transition);
    transition = Transition{
        .save_checkpoint = true,
        .checkpoint_transition_center = transition_center,
        .exit_at = 0,
        .enter_at = 0,
        .end_at = 0,
    };
}

void State::load_checkpoint () {
    expect(!transition);
    transition = Transition{
        .type = TransitionType::ApertureOpen,
        .load_checkpoint = true,
        .exit_at = 0,
        .enter_at = 20,
        .end_at = 30
    };
}

} using namespace vf;

AYU_DESCRIBE(vf::Transition,
    attrs(
        attr("target_room", &Transition::target_room),
        attr("migrant", &Transition::migrant),
        attr("target_pos", &Transition::target_pos),
        attr("save_checkpoint", &Transition::save_checkpoint),
        attr("load_checkpoint", &Transition::load_checkpoint),
        attr("exit_at", &Transition::exit_at),
        attr("enter_at", &Transition::enter_at),
        attr("end_at", &Transition::end_at),
        attr("timer", &Transition::timer)
    )
)

AYU_DESCRIBE(vf::Checkpoint,
    attrs(
        attr("current_room", &Checkpoint::current_room),
        attr("transition_center", &Checkpoint::transition_center),
        attr("world", &Checkpoint::world)
    )
)

AYU_DESCRIBE(vf::State,
    attrs(
        attr("rng", member(&State::rng_uint32, prefer_hex), optional),
        attr("current_frame", &State::current_frame, optional),
        attr("current_room", &State::current_room),
        attr("transition", &State::transition, collapse_optional),
        attr("world", &State::world),
        attr("checkpoint", &State::checkpoint, collapse_optional)
    )
);
