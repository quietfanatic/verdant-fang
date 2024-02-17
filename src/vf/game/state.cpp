#include "state.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/ayu/resources/resource.h"
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
         // fallthrough
    }
    if (timer < exit_at) {
        timer += 1;
        set_transition_t(float(timer) / end_at);
        return true;
    }
    else if (timer == exit_at) {
        state.current_room->exit();
        state.current_room = target_room;
        if (migrant) {
            migrant->set_room(state.current_room);
            migrant->set_pos(target_pos);
        }
        state.current_room->enter();
        swap_world_tex();
        set_transition_side(false);
        if (set_checkpoint) state.save_checkpoint = true;
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
        if (save_checkpoint && !transition) {
            save_checkpoint = false;
             // Don't recursively nest checkpoints
            checkpoint = {};
             // Don't save transition state to checkpoint
            std::optional<Transition> save_tr = move(transition);
            transition = {};
            checkpoint = ayu::item_to_tree(this);
            transition = move(save_tr);
        }
        if (load_checkpoint && !transition) {
            load_checkpoint = false;
            if (checkpoint) {
                 // Preserve rng state so rng is different
                uint32 save_rng = rng_uint32;
                 // Don't overwrite current checkpoint
                ayu::Tree save_cp = move(*checkpoint);
                checkpoint = {};
                ayu::item_from_tree(this, save_cp);
                checkpoint = move(save_cp);
                rng_uint32 = save_rng;
                transition = {};
            }
            else {
                 // No checkpoint?  Uh...reload from initial world?
                load_initial();
            }
        }
        if (current_room) current_room->step();
    }
    current_frame += 1;
}

} using namespace vf;

AYU_DESCRIBE(vf::Transition,
    attrs(
        attr("target_room", &Transition::target_room),
        attr("migrant", &Transition::migrant),
        attr("target_pos", &Transition::target_pos),
        attr("set_checkpoint", &Transition::set_checkpoint),
        attr("exit_at", &Transition::exit_at),
        attr("enter_at", &Transition::enter_at),
        attr("end_at", &Transition::end_at),
        attr("timer", &Transition::timer)
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
