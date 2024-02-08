#include "state.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "room.h"

namespace vf {

struct Scheduled {
    uint64 frame;
    control::Statement action;
};

State::State () : rng(0) { }

void State::step () {
    while (scheduled && scheduled[0].frame == current_frame) {
        auto action = move(scheduled[0].action);
        scheduled.erase(usize(0));
        action();
    }
    if (!frozen && current_room) current_room->step();
    current_frame += 1;
}

void State::schedule_after (
    uint32 delay_frames, control::Statement&& action
) {
    uint64 frame = current_frame + delay_frames + 1;
    for (auto& s : scheduled) {
        if (frame < s.frame) {
            scheduled.insert(&s, Scheduled(frame, move(action)));
            return;
        }
    }
    scheduled.emplace_back(frame, move(action));
}

} using namespace vf;

AYU_DESCRIBE(vf::Scheduled,
    attrs(
        attr("frame", &Scheduled::frame),
        attr("action", &Scheduled::action)
    )
)

AYU_DESCRIBE(vf::State,
    attrs(
        attr("rng", member(&State::rng_uint32, prefer_hex), optional),
        attr("current_frame", &State::current_frame, optional),
        attr("current_room", &State::current_room),
        attr("scheduled", &State::scheduled, optional),
        attr("frozen", &State::frozen, optional),
        attr("world", &State::world)
    )
);
