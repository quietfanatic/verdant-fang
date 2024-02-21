#include "indigo.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/state.h"
#include "math.h"
#include "verdant.h"

namespace vf {

Indigo::Indigo () {
    types |= Types::Indigo;
}

WalkerBusiness Indigo::Walker_business () {
    auto& id = static_cast<IndigoData&>(*data);
    if (state == IS::Capturing) {
        auto v = find_verdant();
        if (anim_timer == 0) {
            if (anim_phase == CP::MoveTarget) {
                v->state = VS::Captured;
                v->capturer = this;
                capture_initial_pos = v->pos;
                v->left = false;
            }
            else if (anim_phase == CP::MoveTargetPost) {
                v->pos = id.capture_target_pos;
            }
            else if (anim_phase == CP::MoveWeapon) {
                v->state = VS::CapturedWeaponTaken;
                capture_initial_pos = v->pos + v->Walker_pose().body->weapon;
            }
            else if (anim_phase == CP::HaveWeapon) {
                v->override_weapon_pos = id.capture_weapon_pos;
            }
            else if (anim_phase == CP::BreakWeapon) {
                v->state = VS::CapturedWeaponBroken;
            }
        }
        if (anim_timer >= id.capturing_sequence[anim_phase]) {
            if (anim_phase == CP::Leave) {
                set_state(WS::Neutral);
                 // TODO: change rooms
                return Walker_business();
            }
            anim_phase += 1;
            anim_timer = 0;
        }
        else {
            if (anim_phase == CP::MoveTarget) {
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                v->pos = lerp(
                    capture_initial_pos,
                    id.capture_target_pos,
                    ease_in_out_sin(t)
                );
            }
            else if (anim_phase == CP::MoveWeapon) {
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                v->override_weapon_pos = lerp(
                    capture_initial_pos,
                    id.capture_weapon_pos,
                    ease_out_sin(t)
                );
            }
            else if (anim_phase >= CP::DetachLimb0 &&
                     anim_phase <= CP::DetachLimb3
            ) {
                uint8 i = id.capture_limb_order[anim_phase = CP::DetachLimb0];
                auto& vp = static_cast<VerdantPoses&>(*v->data->poses);
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                v->limb_pos[i] = v->pos + lerp(
                    vp.captured_limbs[i]->attached,
                    vp.captured_limbs[i]->detached,
                    ease_out_cubic(t)
                );
            }
            else if (anim_phase == CP::TakeLimbs) {
                auto& vp = static_cast<VerdantPoses&>(*v->data->poses);
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                for (usize i = 0; i < 4; i++) {
                    v->limb_pos[i] = lerp(
                        v->pos + vp.captured_limbs[i]->detached,
                        pos + id.capture_limb_offsets[i],
                        ease_in_out_sin(t)
                    );
                }
            }
            anim_timer += 1;
        }
        return WB::Occupied;
    }
    return Walker::Walker_business();
}

Pose Indigo::Walker_pose () {
    if (state == IS::Capturing) {
        auto& poses = static_cast<IndigoPoses&>(*data->poses);
        return poses.capturing[anim_phase];
    }
    else return Walker::Walker_pose();
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

Indigo* find_indigo () {
    if (auto game = current_game)
    if (auto room = game->state().current_room)
    if (auto i = room->find_with_types(Types::Indigo)) {
        return static_cast<Indigo*>(i);
    }
    return null;
}

void do_captured_sequence_ () {
    if (auto i = find_indigo()) {
        i->set_state(IS::Capturing);
    }
}
control::Command do_captured_sequence (do_captured_sequence_, "do_captured_sequence");

} using namespace vf;

AYU_DESCRIBE(vf::IndigoPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("capturing", &IndigoPoses::capturing)
    )
)

AYU_DESCRIBE(vf::IndigoData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("capture_target_pos", &IndigoData::capture_target_pos),
        attr("capture_weapon_pos", &IndigoData::capture_weapon_pos),
        attr("capture_limb_offsets", &IndigoData::capture_limb_offsets),
        attr("capture_limb_order", &IndigoData::capture_limb_order),
        attr("capturing_sequence", &IndigoData::capturing_sequence)
    )
);

AYU_DESCRIBE(vf::Indigo,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("alert_phase", &Indigo::alert_phase, optional),
        attr("alert_timer", &Indigo::alert_timer, optional),
        attr("capture_initial_pos", &Indigo::capture_initial_pos, optional)
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
