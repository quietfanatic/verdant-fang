#include "indigo.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "math.h"
#include "verdant.h"

namespace vf {

namespace IndigoState {
}
namespace IS = IndigoState;

Indigo::Indigo () {
    types |= Types::Indigo;
}

WalkerBusiness Indigo::Walker_business () {
    auto& id = static_cast<IndigoData&>(*data);
    if (auto v = find_verdant()) {
        if (v->state == VS::Captured && v->anim_phase == CP::GoodbyeLimbs) {
            auto& vd = static_cast<VerdantData&>(*v->data);
            float t = float(v->anim_timer) /
                      vd.captured_sequence[v->anim_phase];
            for (uint32 i = 0; i < 4; i++) {
                 // Most likely Verdant was declared in world.ayu before I
                 // was, so her anim timer was ticked before mine was.
                if (v->anim_timer == 1) {
                    stolen_limb_initial_pos[i] = v->limb_pos[i];
                }
                v->limb_pos[i] = lerp(
                    stolen_limb_initial_pos[i],
                    pos + id.stolen_limb_offsets[i],
                    ease_in_out_sin(t)
                );
            }
        }
    }
    return Walker::Walker_business();
}

Controls IndigoMind::Mind_think (Resident& s) {
    Controls r {};
    if (!(s.types & Types::Indigo)) return r;
    auto& me = static_cast<Indigo&>(s);
    if (!target) return r;
    next_alert_phase:
    if (me.alert_phase == 0) {
        if (distance2(target->pos.x, me.pos.x) < length2(sight_range)) {
            me.alert_phase = 1;
            me.alert_timer = 0;
            if (activate_on_sight) activate_on_sight->Activatable_activate();
            goto next_alert_phase;
        }
    }
    else if (me.alert_phase == 1) {
    }
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::IndigoData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("stolen_limb_offsets", &IndigoData::stolen_limb_offsets)
    )
);

AYU_DESCRIBE(vf::Indigo,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("alert_phase", &Indigo::alert_phase, optional),
        attr("alert_timer", &Indigo::alert_timer, optional),
        attr("stolen_limb_initial_pos", &Indigo::stolen_limb_initial_pos, optional)
    )
)

AYU_DESCRIBE(vf::IndigoMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &IndigoMind::target),
        attr("sight_range", &IndigoMind::sight_range),
        attr("activate_on_sight", &IndigoMind::activate_on_sight, optional)
    )
)
