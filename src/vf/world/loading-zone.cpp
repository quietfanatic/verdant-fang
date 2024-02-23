#include "loading-zone.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/state.h"
#include "../game/options.h"
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
    auto& v = static_cast<Verdant&>(o);
    auto& state = current_game->state();
    auto& options = current_game->options();
    if (state.transition) return;  // Transition already ongoing
    state.transition = Transition{
        .target_room = target_room,
        .migrant = &o,
        .target_pos = target_pos,
        .type = transition,
        .save_checkpoint = checkpoint_level >= options.frustration,
        .checkpoint_level = checkpoint_level,
        .checkpoint_transition_center = target_pos + v.visual_center(),
    };
}

} using namespace vf;

AYU_DESCRIBE(vf::LoadingZone,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("bounds", ref_func<Rect>([](LoadingZone& v)->Rect&{ return v.hb.box; })),
        attr("target_room", &LoadingZone::target_room),
        attr("target_pos", &LoadingZone::target_pos),
        attr("transition", &LoadingZone::transition, optional),
        attr("checkpoint_level", &LoadingZone::checkpoint_level, optional)
    )
)
