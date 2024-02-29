#include "indigo.h"
#include <numbers>
#include <random>
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/options.h"
#include "../game/state.h"
#include "door.h"
#include "math.h"
#include "verdant.h"

namespace vf {

Indigo::Indigo () {
    types |= Types::Indigo;
    for (auto& bubble : bubbles) {
        bubble.hb.layers_1 = Layers::Projectile_Walker
                           | Layers::Projectile_Solid
                           | Layers::Projectile_Weapon;
    }
}

void Indigo::go_to_bed () {
    set_state(IS::Bed);
    set_room(bedroom);
    pos = bed_pos;
    if (head_layers & 0b010) {
        glasses_pos = bed_pos + Vec(36, 0);
        head_layers &= ~0b010;
    }
    left = false;
    alert_phase = 5;
}

WalkerBusiness Indigo::Walker_business () {
    auto& id = static_cast<IndigoData&>(*data);
    uint8 n_bubbles = 0;
    for (auto& bubble : bubbles) if (bubble.state == 1) n_bubbles += 1;
    for (auto& bubble : bubbles) {
        if (bubble.state) {
            bubble.timer += 1;
            if (bubble.state == 1) {
                uint8 speed_i = attack_count == 1
                    ? 4 - n_bubbles : 6 - n_bubbles;
                if (speed_i >= 6) {
#ifndef NDEBUG
                    never();
#endif
                    speed_i = 5;
                }
                bubble.direction += bubble.curve;
                bubble.pos += id.bubble_speeds[speed_i] *
                    Vec(std::cos(bubble.direction), std::sin(bubble.direction));
            }
            else if (bubble.state == 2 || bubble.state == 3) {
                if (bubble.state == 2 && bubble.phase == 0 &&
                    bubble.timer == 1
                ) {
                    if (id.bubble_pop_sound) id.bubble_pop_sound->play();
                }
                if (bubble.timer >= id.bubble_pop_sequence[bubble.phase]) {
                    if (bubble.phase == 2) bubble.state = 0;
                    else {
                        bubble.phase += 1;
                        bubble.timer = 0;
                    }
                }
                else bubble.timer += 1;
            }
            else never();
            if (!contains(Rect(-20, -20, 340, 200), bubble.pos)) {
                bubble.state = 0;
            }
        }
    }
    if (state == WS::Dead) {
        if (anim_phase >= 8 && verdant && verdant->state == VS::SnakeCaptured) {
            verdant->set_state(VS::Snake);
            verdant->limb_layers &= ~0b1000;
        }
        else if (anim_phase == 3 && floor) {
            if (head_layers & 0b100) {
                hat_pos = pos + left_flip(Vec(18, 0));
                head_layers &= ~0b100;
            }
        }
    }
    else if (state == IS::Capturing) {
        if (!verdant) verdant = find_verdant();
        if (verdant->state == WS::Dead) return WB::Free;
        if (anim_timer == 0) {
            if (anim_phase == CP::TrackTarget) {
                if (id.attack_sound) id.attack_sound->play();
            }
            if (anim_phase == CP::MoveTarget) {
                verdant->set_state(VS::Captured);
                verdant->indigo = this;
                capture_initial_pos = verdant->pos;
                verdant->left = false;
            }
            else if (anim_phase == CP::MoveTargetPost) {
                verdant->pos = id.capture_target_pos;
            }
            else if (anim_phase == CP::MoveWeapon) {
                verdant->set_state(VS::CapturedWeaponTaken);
                capture_initial_pos = verdant->pos +
                                      verdant->Walker_pose().body->weapon;
            }
            else if (anim_phase == CP::HaveWeapon) {
                verdant->override_weapon_pos = id.capture_weapon_pos;
            }
            else if (anim_phase == CP::BreakWeapon) {
                verdant->set_state(VS::CapturedWeaponBroken);
            }
            else if (anim_phase == CP::DetachLimb0) {
                verdant->set_state(VS::CapturedLimbsDetached);
            }
            else if (anim_phase == CP::Leave) {
                if (back_door) back_door->set_state(DoorState::Open);
            }
            else if (anim_phase == CP::CloseDoor) {
                if (back_door) back_door->set_state(DoorState::Closed);
            }
        }
        if (anim_timer >= id.capturing_sequence[anim_phase]) {
            if (anim_phase == CP::CloseDoor) {
                go_to_bed();
                return WB::Frozen;
            }
            if (anim_phase == CP::MoveTargetWait &&
                (distance(pos, home_pos[home_index]) > 4 || length(vel.x) > 1)
            ) {
                 // If this happens I probably just dodged away from the
                 // target's captured pos, so wait until I settle down a little
                 // before advancing the cutscene.
            }
            else {
                anim_phase += 1;
                anim_timer = 0;
            }
        }
        else {
            if (anim_phase == CP::MoveTarget) {
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                verdant->pos = lerp(
                    capture_initial_pos,
                    id.capture_target_pos,
                    ease_in_out_sin(t)
                );
            }
            else if (anim_phase == CP::MoveWeapon) {
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                verdant->override_weapon_pos = lerp(
                    capture_initial_pos,
                    id.capture_weapon_pos,
                    ease_out_sin(t)
                );
            }
            else if (anim_phase >= CP::DetachLimb0 &&
                     anim_phase <= CP::DetachLimb3
            ) {
                if (anim_timer == 0) {
                    auto& vd = static_cast<VerdantData&>(*verdant->data);
                    if (vd.limb_detach_sound) vd.limb_detach_sound->play();
                }
                uint8 i = id.capture_limb_order[anim_phase - CP::DetachLimb0];
                auto& vp = static_cast<VerdantPoses&>(*verdant->data->poses);
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                verdant->limb_pos[i] = verdant->pos + lerp(
                    vp.captured_limbs[i]->attached,
                    vp.captured_limbs[i]->detached,
                    ease_out_cubic(t)
                );
            }
            else if (anim_phase == CP::TakeLimbs) {
                auto& vp = static_cast<VerdantPoses&>(*verdant->data->poses);
                float t = float(anim_timer) /
                          id.capturing_sequence[anim_phase];
                for (usize i = 0; i < 4; i++) {
                    verdant->limb_pos[i] = lerp(
                        verdant->pos + vp.captured_limbs[i]->detached,
                        pos + id.capture_limb_offsets[i],
                        ease_in_out_sin(t)
                    );
                }
            }
            else if (anim_phase > CP::TakeLimbs) {
                for (usize i = 0; i < 4; i++) {
                    verdant->limb_pos[i] = pos + id.capture_limb_offsets[i];
                }
            }
            anim_timer += 1;
        }
        return WB::Free;
    }
    else if (state == IS::Bed) {
        expect(anim_phase < 2);
        if (!verdant) verdant = find_verdant();
        auto& poses = static_cast<IndigoPoses&>(*data->poses);
        if (anim_timer >= id.bed_cycle[anim_phase]) {
            anim_phase = !anim_phase;
            anim_timer = 1;
        }
        else anim_timer += 1;
        for (int i = 0; i < 4; i++) {
            if (i == id.bed_use_limb) {
                verdant->limb_pos[i] =
                    pos + poses.bed[anim_phase].body->weapon;
            }
            else verdant->limb_pos[i] = bedroom_limb_pos[i];
        }
        if (current_game->options().hide_nudity) {
            verdant->limb_pos[id.bed_use_limb] =
                pos + poses.bed[0].body->weapon;
        }
        return WB::Frozen;
    }
    else if (state == IS::Bit) {
        auto& poses = static_cast<IndigoPoses&>(*data->poses);
        expect(anim_phase < 9);
        if (anim_timer >= id.bit_sequence[anim_phase]) {
            if (anim_phase == 8) {
                set_state(WS::Neutral);
                verdant->limb_pos[id.bed_use_limb] =
                    bedroom_limb_pos[id.bed_use_limb];
            }
            anim_phase += 1;
            anim_timer = 0;
            return Walker_business();
        }
        else anim_timer += 1;
        verdant->limb_pos[id.bed_use_limb] =
            pos + poses.bit[anim_phase].body->weapon;
        return WB::Frozen;
    }
    else if (state == IS::CapturingSnake) {
        expect(anim_phase < 6);
        if (anim_phase == 5) {
             // Stop, wait for poison
        }
        else if (anim_timer >= id.capturing_snake_sequence[anim_phase]) {
            anim_phase += 1;
            anim_timer = 0;
            if (anim_phase == 1) {
                if (id.attack_sound) id.attack_sound->play();
            }
            if (anim_phase == 2) capture_initial_pos = verdant->pos;
            return Walker_business();
        }
        else anim_timer += 1;
        if (anim_phase == 2) {
            verdant->set_state(VS::SnakeCaptured);
            verdant->left = false;
            verdant->pos = lerp(
                capture_initial_pos,
                id.capture_target_pos,
                ease_in_out_sin(
                    float(anim_timer) / id.capturing_snake_sequence[anim_phase]
                )
            );
        }
        else if (anim_phase == 3) {
            verdant->set_state(VS::SnakeCaptured);
            verdant->pos = id.capture_target_pos;
        }
        return WB::Free;
    }
    else if (state == IS::Eaten) {
         // Not really anything to do here, we're already hosed.
        return WB::Frozen;
    }
    return Walker::Walker_business();
}

void Indigo::Walker_move (const Controls& controls) {
    auto& id = static_cast<IndigoData&>(*data);
    if (business == WB::Free && controls[Control::Special] &&
        length(vel.x) < 1
    ) {
         // Dodge
        home_index = !home_index;
        vel.x = home_index ? -id.dodge_speed : id.dodge_speed;
        if (id.dodge_sound) id.dodge_sound->play();
    }
    if (business == WB::DoAttack) {
        auto& rng = current_game->state().rng;
        float seed_angle = std::uniform_real_distribution<float>(
            0, std::numbers::pi_v<float> / 2
        )(rng);
        uint32 seq_len = 0;
        for (auto& p : id.bubble_sequence) seq_len += p;
        auto emit_bubble = [&](uint8 i, float angle, float curve){
            bubbles[i].state = 1;
            bubbles[i].phase = 0;
            bubbles[i].timer = std::uniform_int_distribution<uint32>(
                0, seq_len - 1
            )(rng);
            bubbles[i].pos = pos + left_flip(
                data->poses->attack[2].body->weapon
            );
            if (left) angle = std::numbers::pi_v<float> - angle;
            bubbles[i].direction = angle;
            bubbles[i].curve = left_flip(curve);
        };
        if (attack_count == 0) {
            emit_bubble(0, seed_angle - 0.8f, -0.01f);
            emit_bubble(1, seed_angle - 0.4f, 0.01f);
            emit_bubble(2, seed_angle + 0.4f, -0.01f);
            emit_bubble(3, seed_angle + 0.8f, 0.01f);
            bubbles[4].state = 0;
            bubbles[5].state = 0;
        }
        else {
            emit_bubble(0, seed_angle - 0.8f, -0.01f);
            emit_bubble(1, seed_angle - 0.7f, 0.01f);
            emit_bubble(2, seed_angle - 0.1f, -0.01f);
            emit_bubble(3, seed_angle + 0.1f, 0.01f);
            emit_bubble(4, seed_angle + 0.7f, -0.01f);
            emit_bubble(5, seed_angle + 0.8f, 0.01f);
        }
        attack_count += 1;
        if (id.attack_sound) id.attack_sound->play();
    }
    Walker::Walker_move(controls);
}

void Indigo::Walker_set_hitboxes () {
    if (state == IS::Eaten) {
        clear_hitboxes();
        return;
    }
    body_hb.box = data->body_box;
    set_hitbox(body_hb);
    if (state == IS::Bed) {
        damage_hb.box = data->damage_box;
        add_hitbox(damage_hb);
    }
    auto& id = static_cast<IndigoData&>(*data);
    for (auto& bubble : bubbles) {
        if (bubble.state == 1) {
            bubble.hb.box = bubble.pos - pos + Rect(
                Vec(-id.bubble_radius), Vec(id.bubble_radius)
            );
            add_hitbox(bubble.hb);
        }
    }
}

void Indigo::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (hb.layers_1 & Layers::Projectile_Solid &&
        o_hb.layers_2 & Layers::Projectile_Solid
    ) {
        Rect here = pos + hb.box;
        Rect there = o.pos + o_hb.box;
        Rect overlap = here & there;
        auto& bubble = *(IndigoBubble*)(
            (char*)&hb - offsetof(IndigoBubble, hb)
        );
        if (width(overlap) < height(overlap)) {
            if (overlap.l == here.l && std::cos(bubble.direction) < 0) {
                bubble.pos.x += width(overlap);
                bubble.direction =
                    std::numbers::pi_v<float> - bubble.direction;
            }
            else if (overlap.r == here.r && std::cos(bubble.direction) > 0) {
                bubble.pos.x -= width(overlap);
                bubble.direction =
                    std::numbers::pi_v<float> - bubble.direction;
            }
        }
        else {
            if (overlap.b == here.b && std::sin(bubble.direction) < 0) {
                bubble.pos.y += height(overlap);
                bubble.direction = -bubble.direction;
            }
            else if (overlap.t == here.t && std::sin(bubble.direction) > 0) {
                bubble.pos.y -= height(overlap);
                bubble.direction = -bubble.direction;
            }
        }
    }
    else if (hb.layers_1 & Layers::Projectile_Walker &&
        o_hb.layers_2 & Layers::Projectile_Walker
    ) {
        auto& bubble = *(IndigoBubble*)(
            (char*)&hb - offsetof(IndigoBubble, hb)
        );
        auto& victim = static_cast<Walker&>(o);
        victim.set_state(WS::Dead);
        if (victim.data->attack_sound) victim.data->attack_sound->stop();
        if (victim.data->damage_sound) victim.data->damage_sound->play();
        bubble.state = 3;
        bubble.timer = 0;
    }
    else if (hb.layers_1 & Layers::Projectile_Weapon &&
        o_hb.layers_2 & Layers::Projectile_Weapon
    ) {
        auto& bubble = *(IndigoBubble*)(
            (char*)&hb - offsetof(IndigoBubble, hb)
        );
        bubble.state = 2;
        bubble.timer = 0;
    }
    else Walker::Resident_on_collide(hb, o, o_hb);
}

Pose Indigo::Walker_pose () {
    auto& id = static_cast<IndigoData&>(*data);
    auto& poses = static_cast<IndigoPoses&>(*id.poses);
    Pose r = {};
    if (state == IS::Capturing) {
        if (anim_phase >= CP::DetachLimb1 && anim_phase <= CP::TakeLimbs) {
            if (current_game->options().hide_nudity) {
                r = poses.capturing[CP::DetachLimb0];
            }
            else {
                uint32 cycle_length = id.fingering_cycle[0] +
                    id.fingering_cycle[1];
                r = poses.capturing[
                    (anim_timer % cycle_length) < id.fingering_cycle[0]
                        ? CP::DetachLimb1 : CP::DetachLimb2
                ];
            }
        }
        else r = poses.capturing[anim_phase];
    }
    else if (state == IS::Bed) {
        expect(anim_phase < 2);
        if (current_game->options().hide_nudity) {
            r = poses.bed[0];
        }
        else r = poses.bed[anim_phase];
    }
    else if (state == IS::Bit) {
        expect(anim_phase < 9);
        r = poses.bit[anim_phase];
    }
    else if (state == IS::CapturingSnake) {
        expect(anim_phase < 6);
        r = poses.capturing_snake[anim_phase];
    }
    else if (state == IS::Eaten) {
        if (anim_phase == 0) {
            expect(verdant && verdant->anim_phase < 34);
            r = poses.eaten[verdant->anim_phase];
        }
        else {
             // Goodbye
            return Pose{};
        }
    }
    else r = Walker::Walker_pose();
    if (r.body == poses.fly[0].body) {
        if (left_flip(vel.x) > 1) {
            r.body = poses.fly[1].body;
        }
        else if (left_flip(vel.x) < -1) {
            r.body = poses.fly[2].body;
        }
    }
    if (r.head == poses.fly[0].head) {
        if (left_flip(vel.x) > 1) {
            r.head = poses.fly[1].head;
        }
        else if (left_flip(vel.x) < -1) {
            r.head = poses.fly[2].head;
        }
    }
    return r;
}

void Indigo::Walker_draw_weapon (const Pose& pose) {
    auto& id = static_cast<IndigoData&>(*data);
    auto& poses = static_cast<IndigoPoses&>(*id.poses);
    if (defined(glasses_pos)) {
        draw_frame(*poses.glasses, 1, glasses_pos, Z::Dead - 10);
    }
    if (defined(hat_pos)) {
        draw_frame(*poses.hat, 2, hat_pos, Z::Dead - 10, {-1, 1});
    }
    uint32 seq_len = 0;
    for (auto& p : id.bubble_sequence) seq_len += p;
    for (auto& bubble : bubbles) {
        if (bubble.state) {
            Frame* frame = null;
            if (bubble.state == 1) {
                uint32 timer = bubble.timer % seq_len;
                for (int i = 0; i < 6; i++) {
                    if (timer < id.bubble_sequence[i]) {
                        frame = poses.bubble[i];
                        break;
                    }
                    timer -= id.bubble_sequence[i];
                }
            }
            else if (bubble.state == 2) {
                frame = poses.bubble_pop[bubble.phase];
            }
            else if (bubble.state == 3) {
                frame = poses.bubble_boom[bubble.phase];
            }
            expect(frame);
            draw_frame(*frame, 0, bubble.pos, Z::Projectile);
        }
    }
    if (state == IS::Capturing && anim_phase == CP::TrackTarget) {
        Vec track_pos = lerp(
            pos + left_flip(poses.attack[2].body->weapon),
            verdant->pos + verdant->visual_center(),
            float(anim_timer) / id.capturing_sequence[CP::TrackTarget]
        );
        draw_frame(*poses.bubble[0], 0, track_pos, Z::Projectile);
    }
    else if (state == IS::CapturingSnake && anim_phase == 1) {
        Vec track_pos = lerp(
            pos + left_flip(poses.attack[2].body->weapon),
            verdant->pos + verdant->visual_center(),
            float(anim_timer) / id.capturing_snake_sequence[1]
        );
        draw_frame(*poses.bubble[0], 0, track_pos, Z::Projectile);
    }
    Walker::Walker_draw_weapon(pose);
}

Controls IndigoMind::Mind_think (Resident& s) {
    Controls r {};
    if (!(s.types & Types::Indigo)) return r;
    if (!target) return r;
    auto& me = static_cast<Indigo&>(s);
    auto& id = static_cast<IndigoData&>(*me.data);
    next_alert_phase:
    Vec goal = me.home_pos[me.home_index];
    if (me.alert_phase == 0) {
        if (distance2(target->pos.x, me.pos.x) < length2(sight_range)) {
            me.alert_phase = 1;
            me.alert_timer = 0;
            if (me.front_door) me.front_door->set_state(DoorState::Closed);
            goto next_alert_phase;
        }
        else return r;
    }
    else if (me.alert_phase == 1) {
        if (me.alert_timer >= 120) {
            me.alert_phase = 2;
            me.alert_timer = 0;
        }
        else me.alert_timer += 1;
    }
    else if (me.alert_phase == 2) {
        int bubble_count = 0;
        for (auto& bubble : me.bubbles) {
            if (bubble.state) bubble_count++;
        }
        if (bubble_count == 0 && me.state != WS::Attack) {
            uint8 attack_limit = current_game->options().enemy_difficulty < 2
                ? 1 : 2;
            if (me.attack_count >= attack_limit) {
                me.alert_phase = 3;
                me.alert_timer = 0;
                if (current_game->state().current_music) {
                    glow::require_sdl(Mix_FadeOutMusic(120*1000/60));
                    current_game->state().current_music = null;
                }
                goto next_alert_phase;
            }
            else r[Control::Attack] = 1;
        }
    }
    else if (me.alert_phase == 3) {
        if (me.alert_timer >= 120 && target->state != WS::Dead) {
            me.alert_phase = 4;
            me.alert_timer = 0;
        }
        else me.alert_timer += 1;
    }
    else if (me.alert_phase == 4) {
        me.set_state(IS::Capturing);
        me.alert_phase = 5;
    }
    else if (me.alert_phase == 5) {
        if (me.poison_level) {
            goal = Vec(160, 80);
            if (me.state == WS::Neutral) {
                uint32 anim_len = 0;
                for (auto ph : id.capturing_snake_sequence) anim_len += ph;
                if (me.poison_timer <= anim_len) {
                    me.set_state(IS::CapturingSnake);
                }
            }
        }
        if (me.state == IS::Capturing && me.back_door &&
            (me.anim_phase >= CP::Leave && me.anim_phase <= CP::CloseDoor)
        ) {
            me.left = false;
            goal = me.back_door->closed_pos + Vec(120, 8);
        }
    }
    if (me.business == WB::Free) {
        if (me.state != IS::Capturing || me.anim_phase < CP::Leave) {
            me.left = target->pos.x < me.pos.x;
        }
         // Dodge if Verdant gets too close.  Not that it matters since I'm
         // normally intangible, but it's for the performance.
        if (distance2(target->pos, me.pos) < 36*36 &&
            length(me.vel.x) < 1
        ) {
            r[Control::Special] = 1;
        }
    }
     // If I hold a direction while going fast from dodging, I'll keep going
     // the same speed.  We should probably fix this in the movement logic, but
     // that's handled by Walker and we don't want to interfere with that, so
     // I'm just going to release the directional button to decelerate.
    if (me.pos.x < goal.x - goal_tolerance.x && me.vel.x < id.air_max) {
        r[Control::Right] = 1;
    }
    else if (me.pos.x > goal.x + goal_tolerance.x && me.vel.x > -id.air_max) {
        r[Control::Left] = 1;
    }
    else if (me.vel.x < 0.01) {
        r[Control::Right] = 1;
    }
    else if (me.vel.x > 0.01) {
        r[Control::Left] = 1;
    }
    if (me.pos.y < goal.y - goal_tolerance.y) {
        r[Control::Up] = 1;
    }
    else if (me.pos.y > goal.y + goal_tolerance.y) {
        r[Control::Down] = 1;
    }
    else if (me.vel.y < -0.5) {
        r[Control::Up] = 1;
    }
     // else just float down
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

void put_indigo_to_bed_ () {
    if (auto i = find_indigo()) {
        i->go_to_bed();
    }
}
control::Command put_indigo_to_bed (put_indigo_to_bed_, "put_indigo_to_bed");

} using namespace vf;

AYU_DESCRIBE(vf::IndigoPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("bubble", &IndigoPoses::bubble),
        attr("bubble_pop", &IndigoPoses::bubble_pop),
        attr("bubble_boom", &IndigoPoses::bubble_boom),
        attr("capturing", &IndigoPoses::capturing),
        attr("bed", &IndigoPoses::bed),
        attr("glasses", &IndigoPoses::glasses),
        attr("bit", &IndigoPoses::bit),
        attr("capturing_snake", &IndigoPoses::capturing_snake),
        attr("hat", &IndigoPoses::hat),
        attr("eaten", &IndigoPoses::eaten)
    )
)

AYU_DESCRIBE(vf::IndigoData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("bubble_radius", &IndigoData::bubble_radius),
        attr("bubble_speeds", &IndigoData::bubble_speeds),
        attr("bubble_sequence", &IndigoData::bubble_sequence),
        attr("bubble_pop_sequence", &IndigoData::bubble_pop_sequence),
        attr("dodge_speed", &IndigoData::dodge_speed),
        attr("capture_target_pos", &IndigoData::capture_target_pos),
        attr("capture_weapon_pos", &IndigoData::capture_weapon_pos),
        attr("capture_limb_offsets", &IndigoData::capture_limb_offsets),
        attr("capture_limb_order", &IndigoData::capture_limb_order),
        attr("capturing_sequence", &IndigoData::capturing_sequence),
        attr("fingering_cycle", &IndigoData::fingering_cycle, optional),
        attr("bed_cycle", &IndigoData::bed_cycle, optional),
        attr("bed_use_limb", &IndigoData::bed_use_limb),
        attr("bit_sequence", &IndigoData::bit_sequence),
        attr("capturing_snake_sequence", &IndigoData::capturing_snake_sequence),
        attr("bubble_pop_sound", &IndigoData::bubble_pop_sound, optional),
        attr("dodge_sound", &IndigoData::dodge_sound, optional)
    )
);

AYU_DESCRIBE(vf::IndigoBubble,
    attrs(
        attr("state", &IndigoBubble::state),
        attr("phase", &IndigoBubble::phase),
        attr("timer", &IndigoBubble::timer),
        attr("pos", &IndigoBubble::pos),
        attr("direction", &IndigoBubble::direction),
        attr("curve", &IndigoBubble::curve)
    )
)

AYU_DESCRIBE(vf::Indigo,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("alert_phase", &Indigo::alert_phase, optional),
        attr("alert_timer", &Indigo::alert_timer, optional),
        attr("attack_count", &Indigo::attack_count, optional),
        attr("home_index", &Indigo::home_index, optional),
        attr("home_pos", &Indigo::home_pos, optional),
        attr("bubbles", &Indigo::bubbles, optional),
        attr("capture_initial_pos", &Indigo::capture_initial_pos, optional),
        attr("front_door", &Indigo::front_door),
        attr("back_door", &Indigo::back_door),
        attr("bedroom", &Indigo::bedroom),
        attr("bed_pos", &Indigo::bed_pos),
        attr("glasses_pos", &Indigo::glasses_pos, optional),
        attr("bedroom_limb_pos", &Indigo::bedroom_limb_pos, optional),
        attr("hat_pos", &Indigo::hat_pos, optional),
        attr("verdant", &Indigo::verdant, optional)
    )
)

AYU_DESCRIBE(vf::IndigoMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &IndigoMind::target),
        attr("sight_range", &IndigoMind::sight_range),
        attr("goal_tolerance", &IndigoMind::goal_tolerance, optional)
    )
)
