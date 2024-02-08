#include "transition.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/camera.h"
#include "../game/game.h"
#include "../game/state.h"
#include "verdant.h"

namespace vf {

static void unfreeze_ () {
    current_game->state().frozen = false;
}
static control::Command unfreeze (unfreeze_, "vf::Transition::unfreeze");

static void change_room_ (Transition* self, Verdant* v) {
    self->triggered = false;
    v->set_room(null);
    auto& state = current_game->state();
    state.current_room->exit();
    state.current_room = self->target_room;
    state.current_room->enter();
    v->set_room(self->target_room);
     // Hack to unbreak walking sfx
    v->walk_start_x += self->target_pos.x - v->pos.x;
    v->pos = self->target_pos;
    swap_world_tex();
    state.frozen = true;
    state.schedule_after(
        wipe_duration - 10, control::Statement(&unfreeze)
    );
}
static control::Command change_room (
    change_room_, "vf::Transition::change_room"
);

Transition::Transition () {
    types = Types::Transition;
    pos = {0, 0};
    hb.layers_1 = Layers::Transition_Verdant;
    hitboxes = Slice<Hitbox>(&hb, 1);
}

void Transition::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    expect(o.types & Types::Verdant);
    auto& v = static_cast<Verdant&>(o);
    if (!triggered) {
        triggered = true;
        start_transition(direction);
        current_game->state().schedule_after(
            10, control::Statement(&change_room, this, &v)
        );
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Transition,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("bounds", ref_func<Rect>([](Transition& v)->Rect&{ return v.hb.box; })),
        attr("target_room", &Transition::target_room),
        attr("target_pos", &Transition::target_pos),
        attr("direction", &Transition::direction, optional),
        attr("triggered", &Transition::triggered, optional)
    )
)
