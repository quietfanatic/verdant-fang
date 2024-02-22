#include "indigo.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/state.h"
#include "door.h"
#include "math.h"
#include "verdant.h"

namespace vf {

Indigo::Indigo () {
    types |= Types::Indigo;
}

void Indigo::init () {
    if (!defined(home_pos)) home_pos = pos;
}

WalkerBusiness Indigo::Walker_business () {
    auto& id = static_cast<IndigoData&>(*data);
    if (state == IS::Capturing) {
        auto v = find_verdant();
        if (anim_timer == 0) {
            if (anim_phase == CP::MoveTarget) {
                v->set_state(VS::Captured);
                v->indigo = this;
                capture_initial_pos = v->pos;
                v->left = false;
            }
            else if (anim_phase == CP::MoveTargetPost) {
                v->pos = id.capture_target_pos;
            }
            else if (anim_phase == CP::MoveWeapon) {
                v->set_state(VS::CapturedWeaponTaken);
                capture_initial_pos = v->pos + v->Walker_pose().body->weapon;
            }
            else if (anim_phase == CP::HaveWeapon) {
                v->override_weapon_pos = id.capture_weapon_pos;
            }
            else if (anim_phase == CP::BreakWeapon) {
                v->set_state(VS::CapturedWeaponBroken);
            }
            else if (anim_phase == CP::DetachLimb0) {
                v->set_state(VS::CapturedLimbsDetached);
            }
            else if (anim_phase == CP::Leave) {
                if (back_door && !back_door->open) {
                    back_door->Activatable_activate();
                }
            }
            else if (anim_phase == CP::CloseDoor) {
                if (back_door && back_door->open) {
                    back_door->Activatable_activate();
                }
            }
        }
        if (anim_timer >= id.capturing_sequence[anim_phase]) {
            if (anim_phase == CP::CloseDoor) {
                set_state(IS::Bed);
                set_room(bedroom);
                pos = bed_pos;
                glasses_pos = bed_pos + Vec(36, 0);
                head_layers = 0b101;
                left = false;
                return WB::Frozen;
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
                if (anim_timer == 0) {
                    auto& vd = static_cast<VerdantData&>(*v->data);
                    if (vd.limb_detach_sound) vd.limb_detach_sound->play();
                }
                uint8 i = id.capture_limb_order[anim_phase - CP::DetachLimb0];
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
            else if (anim_phase > CP::TakeLimbs) {
                for (usize i = 0; i < 4; i++) {
                    v->limb_pos[i] = pos + id.capture_limb_offsets[i];
                }
            }
            anim_timer += 1;
        }
        return WB::Free;
    }
    else if (state == IS::Bed) {
        auto v = find_verdant();
        auto& poses = static_cast<IndigoPoses&>(*data->poses);
        if (anim_timer >= id.bed_cycle[anim_phase]) {
            anim_phase = !anim_phase;
            anim_timer = 1;
        }
        else anim_timer += 1;
        v->limb_pos[id.bed_use_limb] = pos + poses.bed[anim_phase].body->weapon;
        return WB::Frozen;
    }
    else if (state == IS::Bit) {
        auto v = find_verdant();
        auto& poses = static_cast<IndigoPoses&>(*data->poses);
        expect(anim_phase < 7);
        if (anim_timer >= id.bit_sequence[anim_phase]) {
            if (anim_phase == 6) {
                set_state(WS::Neutral);
            }
            anim_phase += 1;
            anim_timer = 0;
            return Walker_business();
        }
        else anim_timer += 1;
        v->limb_pos[id.bed_use_limb] = pos + poses.bit[anim_phase].body->weapon;
        return WB::Frozen;
    }
    return Walker::Walker_business();
}

void Indigo::Walker_set_hitboxes () {
    body_hb.box = data->body_box;
    set_hitbox(body_hb);
    if (state == IS::Bed) {
        damage_hb.box = data->damage_box;
        add_hitbox(damage_hb);
    }
}

Pose Indigo::Walker_pose () {
    auto& id = static_cast<IndigoData&>(*data);
    auto& poses = static_cast<IndigoPoses&>(*id.poses);
    if (state == IS::Capturing) {
        if (anim_phase >= CP::DetachLimb1 && anim_phase <= CP::TakeLimbs) {
            uint32 cycle_length = id.fingering_cycle[0] + id.fingering_cycle[1];
            return poses.capturing[
                (anim_timer % cycle_length) < id.fingering_cycle[0]
                    ? CP::DetachLimb1 : CP::DetachLimb2
            ];
        }
        else return poses.capturing[anim_phase];
    }
    else if (state == IS::Bed) {
        expect(anim_phase < 2);
        return poses.bed[anim_phase];
    }
    else if (state == IS::Bit) {
        expect(anim_phase < 7);
        return poses.bit[anim_phase];
    }
    else return Walker::Walker_pose();
}

void Indigo::Walker_draw_weapon (const Pose& pose) {
    auto& poses = static_cast<IndigoPoses&>(*data->poses);
    if (defined(glasses_pos)) {
        draw_frame(*poses.glasses, 1, glasses_pos);
    }
    Walker::Walker_draw_weapon(pose);
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
            if (me.front_door && me.front_door->open) {
                me.front_door->Activatable_activate();
            }
            goto next_alert_phase;
        }
    }
    else if (me.alert_phase == 1) {
        Vec goal = me.home_pos;
        if (me.state == IS::Capturing && me.back_door &&
            (me.anim_phase >= CP::Leave && me.anim_phase <= CP::CloseDoor)
        ) {
            me.left = false;
            goal = me.back_door->closed_pos + Vec(120, 8);
        }
        if (me.pos.x < goal.x - goal_tolerance.x) {
            r[Control::Right] = 1;
        }
        else if (me.pos.x > goal.x + goal_tolerance.x) {
            r[Control::Left] = 1;
        }
        if (me.pos.y < goal.y - goal_tolerance.y) {
            r[Control::Up] = 1;
        }
        if (me.pos.y > goal.y + goal_tolerance.y) {
            r[Control::Down] = 1;
        }
         // else just float down
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
        attr("capturing", &IndigoPoses::capturing),
        attr("bed", &IndigoPoses::bed),
        attr("glasses", &IndigoPoses::glasses),
        attr("bit", &IndigoPoses::bit)
    )
)

AYU_DESCRIBE(vf::IndigoData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("capture_target_pos", &IndigoData::capture_target_pos),
        attr("capture_weapon_pos", &IndigoData::capture_weapon_pos),
        attr("capture_limb_offsets", &IndigoData::capture_limb_offsets),
        attr("capture_limb_order", &IndigoData::capture_limb_order),
        attr("capturing_sequence", &IndigoData::capturing_sequence),
        attr("fingering_cycle", &IndigoData::fingering_cycle, optional),
        attr("bed_cycle", &IndigoData::bed_cycle, optional),
        attr("bed_use_limb", &IndigoData::bed_use_limb),
        attr("bit_sequence", &IndigoData::bit_sequence)
    )
);

AYU_DESCRIBE(vf::Indigo,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("alert_phase", &Indigo::alert_phase, optional),
        attr("alert_timer", &Indigo::alert_timer, optional),
        attr("home_pos", &Indigo::home_pos, optional),
        attr("capture_initial_pos", &Indigo::capture_initial_pos, optional),
        attr("front_door", &Indigo::front_door),
        attr("back_door", &Indigo::back_door),
        attr("bedroom", &Indigo::bedroom),
        attr("bed_pos", &Indigo::bed_pos),
        attr("glasses_pos", &Indigo::glasses_pos, optional)
    ),
    init<&Indigo::init>()
)

AYU_DESCRIBE(vf::IndigoMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &IndigoMind::target),
        attr("sight_range", &IndigoMind::sight_range),
        attr("goal_tolerance", &IndigoMind::goal_tolerance, optional)
    )
)
