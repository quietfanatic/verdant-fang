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

void LoadingZone::trigger (Verdant& v) {
    auto& state = current_game->state();
    auto& options = current_game->options();
    if (state.transition) return;  // Transition already ongoing
    state.transition = Transition{
        .target_room = target_room,
        .migrant = &v,
        .target_pos = target_pos,
        .type = transition,
        .save_checkpoint = checkpoint_level >= options.frustration,
        .checkpoint_level = checkpoint_level,
        .checkpoint_transition_center = target_pos + v.visual_center(),
    };
}

void LoadingZone::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    expect(o.types & Types::Verdant);
    trigger(static_cast<Verdant&>(o));
}

void trigger_loading_zone_ (AnyString name) {
    if (!current_game) return;
    if (auto v = find_verdant()) {
        LoadingZone* zone = current_game->state_res["world"][name][1];
        zone->trigger(*v);
    }
}
control::Command trigger_loading_zone (trigger_loading_zone_, "trigger_loading_zone", "Activate a loading zone by its name in the world document");

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
