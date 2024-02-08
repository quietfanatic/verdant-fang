#include "state.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "camera.h"
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
    if (should_step && current_room) current_room->step(); 
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
        attr("world", &State::world)
    )
);
