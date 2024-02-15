#include "state.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/ayu/traversal/to-tree.h"
#include "../../dirt/control/command.h"
#include "camera.h"
#include "game.h"
#include "options.h"
#include "room.h"

namespace vf {

struct Scheduled {
    uint64 frame;
    control::Statement action;
};

 // If this returns false, don't step the room.
bool Transition::step (State& state) {
    if (until_exit) {
        if (!--until_exit) {
            state.current_room->exit();
            state.current_room = target_room;
            if (migrant) {
                migrant->set_room(state.current_room);
                migrant->set_pos(target_pos);
            }
            state.current_room->enter();
            swap_world_tex();
            auto& options = current_game->options();
            ayu::dump(checkpoint_level, options.frustration);
            if (checkpoint_level >= options.frustration) {
                state.save_checkpoint = true;
            }
             // fallthrough
        }
        else return true;
    }
    if (until_enter) {
        --until_enter;
        return false;
    }
     // Delete self
    state.transition = {};
    return true;
}

State::State () : rng(0) { }

void State::step () {
    bool should_step = true;
    if (transition) should_step = transition->step(*this);
    if (should_step) {
        if (save_checkpoint) {
            save_checkpoint = false;
             // Don't recursively nest checkpoints
            checkpoint = {};
            checkpoint = ayu::item_to_tree(this);
        }
        if (load_checkpoint) {
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
        attr("until_exit", &Transition::until_exit),
        attr("until_enter", &Transition::until_enter)
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
