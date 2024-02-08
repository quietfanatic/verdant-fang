#include "transition.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"
#include "../game/game.h"
#include "verdant.h"

namespace vf {

Transition::Transition () {
    types = Types::Transition;
    pos = {0, 0};
    hb.layers_1 = Layers::Transition_Verdant;
    hitboxes = Slice<Hitbox>(&hb, 1);
}

void Transition::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    expect(o.types & Types::Verdant);
    auto& v = static_cast<Verdant&>(o);
    current_game->before_next_step.push_back([&v, this]{
        v.set_room(null);
        auto& game = *current_game;
        game.current_room->exit();
        game.current_room = target_room;
        game.current_room->enter();
        v.set_room(target_room);
        v.walk_start_x += target_pos.x - v.pos.x; // hack
        v.pos = target_pos;
        start_transition();
    });
}

} using namespace vf;

AYU_DESCRIBE(vf::Transition,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("bounds", ref_func<Rect>([](Transition& v)->Rect&{ return v.hb.box; })),
        attr("target_room", &Transition::target_room),
        attr("target_pos", &Transition::target_pos)
    )
)
