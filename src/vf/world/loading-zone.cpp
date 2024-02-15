#include "loading-zone.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/camera.h"
#include "../game/game.h"
#include "../game/state.h"
#include "verdant.h"

namespace vf {

LoadingZone::LoadingZone () {
    types = Types::LoadingZone;
    pos = {0, 0};
    hb.layers_1 = Layers::LoadingZone_Verdant;
    set_hitbox(hb);
}

void LoadingZone::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    expect(o.types & Types::Verdant);
    auto& state = current_game->state();
    if (state.transition) return;  // LoadingZone already scheduled
    state.transition = Transition(target_room, &o, target_pos, checkpoint_level);
    start_transition_effect(direction);
}

} using namespace vf;

AYU_DESCRIBE(vf::LoadingZone,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("bounds", ref_func<Rect>([](LoadingZone& v)->Rect&{ return v.hb.box; })),
        attr("target_room", &LoadingZone::target_room),
        attr("target_pos", &LoadingZone::target_pos),
        attr("direction", &LoadingZone::direction, optional),
        attr("checkpoint_level", &LoadingZone::checkpoint_level, optional)
    )
)
