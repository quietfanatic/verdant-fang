#include "verdant.h"

#include "../../dirt/control/command.h"
#include "../game/camera.h"
#include "../game/options.h"
#include "../game/game.h"
#include "../game/state.h"
#include "door.h"
#include "indigo.h"
#include "math.h"

namespace vf {

Verdant::Verdant () {
    types |= Types::Verdant;
    body_hb.layers_2 |= Layers::LoadingZone_Verdant;
    body_layers = 0x7;
}

Vec Verdant::visual_center () {
    auto& vd = static_cast<VerdantData&>(*data);
    auto& poses = static_cast<VerdantPoses&>(*vd.poses);
    Pose pose = Walker_pose();
    if (pose.body == poses.dead[0].body) {
        return left_flip(vd.dead_center);
    }
    else if (pose.body == poses.deadf[0].body) {
        return left_flip(vd.dead_center_forward);
    }
    else {
        return left_flip(vd.center);
    }
}

void Verdant::go_to_limbo () {
    auto& state = current_game->state();
    if (limbo && room != limbo && !state.transition) {
         // Horrible hard-coded values, too busy to do this properly
        Vec target = pos;
        Vec focus = target + visual_center();
        if (focus.x < 36) {
            target.x += 36 - focus.x;
            focus.x = 36;
        }
        if (focus.x > 284) {
            target.x += 284 - focus.x;
            focus.x = 284;
        }
        if (focus.y < 17) {
            target.y += 17 - focus.y;
            focus.y = 17;
        }
        if (focus.y > 140) {
            target.y += 140 - focus.y;
            focus.y = 140;
        }
        state.transition = Transition{
            .target_room = limbo,
            .migrant = this,
            .target_pos = target,
            .type = TransitionType::ApertureClose,
            .exit_at = 0,
            .enter_at = 0,
            .start_music_volume = state.current_music
                ? state.current_music->volume
                : GNAN,
            .end_music_volume = state.current_music
                ? state.current_music->volume / 2
                : GNAN,
        };
        set_transition_center(focus);
    }
}

void Verdant::animate_tongue () {
    auto& vd = static_cast<VerdantData&>(*data);
    uint32 acc = 0;
    for (uint32 i = 0; i < 5; i++) {
        acc += vd.snake_tongue_cycle[i];
        if (tongue_timer < acc) {
            switch (i) {
                case 0: case 4: body_layers = 0b1; break;
                case 1: case 3: body_layers = 0b11; break;
                case 2: body_layers = 0b111; break;
            }
            break;
        }
        else if (i == 4) tongue_timer = 0;
    }
    tongue_timer += 1;
}

WalkerBusiness Verdant::Walker_business () {
    auto& vd = static_cast<VerdantData&>(*data);
     // Treat revive animation separately from other stuff
    if (revive_phase) {
        auto& gs = current_game->state();
        if (!gs.transition) {
            expect(revive_phase < 7);
            if (revive_phase == 1 && revive_timer == 0) {
                if (vd.revive_sound) vd.revive_sound->play();
            }
            if (revive_phase == 6) {
                gs.load_checkpoint();
                 // Don't worry about resetting stuff.  After loading the
                 // checkpoint, this earthly body will be no more.
            }
            else if (revive_timer >= vd.revive_sequence[revive_phase-1]) {
                revive_phase += 1;
                revive_timer = 0;
                return Walker_business();
            }
            else revive_timer += 1;
            body_tint = vd.revive_tint[revive_phase-1];
            weapon_tint = vd.revive_tint[revive_phase-1];
        }
        return WB::Frozen;
    }
     // Clear this when no longer relevant
    if (state != WS::Stun && state != WS::Dead && state != VS::FangHelp) {
        damage_forward = false;
    }

    if (state == VS::PreTransform) {
        expect(anim_phase == 0);
        if (pos.x >= vd.transform_pos.x) {
            set_state(VS::Transform);
            transform_timer = 0;
            return Walker_business();
        }
        else return WB::Free;
    }
    else if (state == VS::Transform) {
        transform_timer += 1;
        expect(anim_phase < TP::N_Phases);
        for (auto& ts : vd.transform_sounds) {
            if (anim_phase == ts.phase && anim_timer == ts.timer) {
                ts.sound->play();
            }
        }
        if (anim_timer >= vd.transform_sequence[anim_phase]) {
            if (anim_phase == TP::Receiving) {
                transform_timer = 0;
                set_state(WS::Neutral);
                auto& state = current_game->state();
                if (vd.music_after_transform) {
                    state.current_music = vd.music_after_transform;
                    state.current_music->play();
                }
                state.save_checkpoint(3, pos + visual_center());
            }
            else {
                anim_phase += 1;
                anim_timer = 0;
            }
            return Walker_business();
        }
        else {
            anim_timer += 1;
            return anim_phase == 0 ? WB::Occupied : WB::Frozen;
        }
    }
    else if (state == WS::Hit) {
        if (anim_phase == 1 && anim_timer == data->hit_sequence[1]) {
            if (vd.unstab_sound) vd.unstab_sound->play();
        }
    }
    else if (state == WS::Stun) {
        if (poison_level == 2 && anim_timer == 0 && paralyze_symbol_timer && floor) {
            damage_forward = true;
            if (floor) pos.y += data->dead_floor_lift;
        }
    }
    else if (state == WS::Dead) {
        if (poison_level == 3 && anim_phase == 0 && anim_timer == 0) {
            damage_forward = true;
        }
        if (anim_phase == 10) {
            if (room != limbo) {
                go_to_limbo();
            }
            else {
                set_state(VS::FangHelp);
                return Walker_business();
            }
        }
    }
    else if (state == VS::FangHelp) {
        expect(anim_phase < 6);
        if (anim_phase == 5 || current_game->state().transition) {
             // Do nothing
        }
        else if (anim_timer >= vd.fang_help_sequence[anim_phase]) {
            anim_phase += 1;
            anim_timer = 0;
            return Walker_business();
        }
        else anim_timer += 1;
        return WB::Frozen;
    }
    else if (state == VS::Captured) {
        body_layers |= 0b1000;
        limb_layers = body_layers;
        for (uint i = 0; i < 4; i++) {
            auto& poses = static_cast<VerdantPoses&>(*data->poses);
            limb_pos[i] = pos + poses.captured_limbs[i]->attached;
        }
        return WB::Frozen;
    }
    else if (state == VS::CapturedWeaponTaken) {
        weapon_layers = 0b1001;
        if (anim_phase >= 2) {
             // animation ends
        }
        else if (anim_timer >= vd.weapon_taken_sequence[anim_phase]) {
            anim_phase += 1;
            anim_timer = 0;
            if (anim_phase == 1) override_weapon_scale = {-1, 1};
            return Walker_business();
        }
        else anim_timer += 1;
        return WB::Frozen;
    }
    else if (state == VS::CapturedWeaponBroken) {
        weapon_tint = anim_phase == 1 ? vd.transform_magic_color : 0;
        weapon_layers = anim_phase == 0 ? 0b1001 : 0b11;
        if (current_game->options().hide_blood) {
            weapon_layers &= ~0b10;
        }
        if (anim_timer == 0) {
            if (anim_phase == 0) {
                if (vd.spear_break_sound) vd.spear_break_sound->play();
            }
            else if (anim_phase == 2) {
                if (vd.snake_death_sound) vd.snake_death_sound->play();
            }
        }
        if (anim_phase == 8) {
            // Stop timer
        }
        else if (anim_phase == 3) {
             // Fang falls down
            fang_vel_y -= vd.fang_gravity;
            override_weapon_pos.y += fang_vel_y;
            if (override_weapon_pos.y < vd.fang_dead_y) {
                override_weapon_pos.y = vd.fang_dead_y;
                anim_phase += 1;
                anim_timer = 0;
                return Walker_business();
            }
        }
        else if (anim_timer >= vd.weapon_broken_sequence[anim_phase]) {
            anim_phase += 1;
            anim_timer = 0;
            if (anim_phase == 3) {
                fang_vel_y = 0;
            }
            return Walker_business();
        }
        else anim_timer += 1;
        return WB::Frozen;
    }
    else if (state == VS::CapturedLimbsDetached) {
        override_weapon_pos.y = vd.fang_dead_y;
        if (indigo->state != IS::Capturing) {
            set_state(VS::Limbless);
            return Walker_business();
        }
        return WB::Frozen;
    }
    else if (state == VS::Limbless) {
        if (floor) {
            if (anim_timer >= vd.limbless_sequence) {
                set_state(VS::Inch);
                return Walker_business();
            }
            else anim_timer += 1;
        }
        return WB::Occupied;
    }
    else if (state == VS::Inch) {
        if (pos.x >= override_weapon_pos.x - 24) {
            set_state(VS::Snakify);
            return Walker_business();
        }
        else return WB::Free;
    }
    else if (state == VS::Snakify) {
        expect(anim_phase < 8);
        if (anim_timer >= vd.snakify_sequence[anim_phase]) {
            if (anim_phase == 7) {
                set_state(VS::Snake);
            }
            else {
                if (anim_phase == 3) {
                    if (vd.revive_sound) vd.revive_sound->play();
                }
                anim_phase += 1;
                anim_timer = 0;
            }
            return Walker_business();
        }
        else {
            anim_timer += 1;
            body_tint = vd.transform_magic_color;
            switch (anim_phase) {
                case 0: body_tint.a = 0x60; break;
                case 1: body_tint.a = 0xa0; break;
                case 2: case 3: case 4: break;
                case 5: case 6: case 7:
                    body_tint = 0; body_layers = 1; break;
                default: never();
            }
            weapon_tint = body_tint;
            return WB::Frozen;
        }
    }
    else if (state == VS::Snake) {
        if (anim_timer == 0) {
            tongue_timer = 0;
            anim_timer += 1;
        }
        animate_tongue();
        return WB::Free;
    }
    else if (state == VS::SnakeAttack) {
        if (indigo && indigo->state == WS::Dead &&
            distance(pos.x, indigo->pos.x) < 20
        ) {
            set_state(VS::SnakeEat);
            auto& poses = static_cast<VerdantPoses&>(*vd.poses);
            left = !indigo->left;
            pos = indigo->pos - left_flip(poses.eat[0].body->head);
            return Walker_business();
        }
        if (anim_phase == 0 && anim_timer == 0) tongue_timer = 0;
        else if (anim_phase >= 2) tongue_timer = 0;
        else animate_tongue();
        expect(anim_phase < 6);
        if (anim_timer >= vd.snake_attack_sequence[anim_phase]) {
            if (anim_phase == 0) return WB::HoldAttack;
            if (anim_phase == 5) {
                set_state(VS::Snake);
            }
            else {
                anim_timer = 0;
                anim_phase += 1;
                if (anim_phase == 2) {
                    vel = left_flip(vd.snake_attack_vel);
                    if (data->attack_sound) data->attack_sound->play();
                }
            }
            return Walker_business();
        }
        else {
            anim_timer += 1;
            switch (anim_phase) {
                case 0: case 1: return WB::Occupied;
                case 2: return WB::DoAttack;
                case 3: case 4: return WB::Occupied;
                case 5: return WB::Interruptible;
                default: never();
            }
        }
    }
    else if (state == VS::SnakeBite) {
        expect(indigo);
        expect(indigo->anim_phase < 9);
        if (indigo->anim_phase == 8) {
            pos = indigo->pos + vd.bite_indigo_offsets[indigo->anim_phase];
            vel = vd.bite_release_vel;
            set_state(VS::Snake);
            return Walker_business();
        }
        else {
            pos = indigo->pos + vd.bite_indigo_offsets[indigo->anim_phase];
        }
        return WB::Frozen;
    }
    else if (state == VS::SnakeCaptured) {
        body_layers = 0b1001;
        return WB::Frozen;
    }
    else if (state == VS::SnakeEat) {
        if (anim_timer == 0 && !(anim_phase % 2)) {
            if (vd.snake_eat_sound) vd.snake_eat_sound->play();
        }
        if (anim_timer >= vd.eat_sequence[anim_phase]) {
            if (anim_phase == 33) {
                indigo->verdant = null;
                indigo->anim_phase = 1;
                indigo = null;
                set_state(VS::Desnakify);
            }
            else {
                anim_phase += 1;
                anim_timer = 0;
            }
            return Walker_business();
        }
        else anim_timer += 1;
        expect(indigo);
        indigo->set_state(IS::Eaten);
        auto& poses = static_cast<VerdantPoses&>(*vd.poses);
        indigo->pos = pos + left_flip(poses.eat[anim_phase].body->head);
        return WB::Frozen;
    }
    else if (state == VS::Desnakify) {
        if (anim_phase == DP::WalkAway) {
            return WB::Free;
        }
        if (anim_timer == 0) {
            if (anim_phase == DP::FullGlow0) {
                body_tint = vd.transform_magic_color;
                body_tint.a = 0x60;
                body_layers = 1;
            }
            else if (anim_phase == DP::FullGlow1) {
                body_tint = vd.transform_magic_color;
                body_tint.a = 0xa0;
            }
            else if (anim_phase == DP::FullGlow2) {
                body_tint = vd.transform_magic_color;
            }
            else if (anim_phase == DP::FullScreenGlow2) {
                if (vd.revive_sound) vd.revive_sound->play();
            }
            else if (anim_phase == DP::FangRevived0) {
                body_tint = {};
                override_weapon_pos = pos + left_flip(vd.desnakify_fang_pos[0]);
                override_weapon_scale = {-1, 1};
            }
            else if (anim_phase == DP::FangGlow0) {
                weapon_tint = vd.transform_magic_color;
                weapon_tint.a = 0x60;
                override_weapon_pos = pos + left_flip(vd.desnakify_fang_pos[1]);
                for (int i = 0; i < 4; i++) {
                    limb_initial_pos[i] = limb_pos[i];
                }
            }
            else if (anim_phase == DP::FangGlow1) {
                weapon_tint = vd.transform_magic_color;
                weapon_tint.a = 0xa0;
                limb_tint = weapon_tint;
                for (int i = 0; i < 4; i++) {
                    limb_pos[i] = pos + left_flip(vd.desnakify_fang_pos[1]) +
                        vd.desnakify_limb_offsets[i];
                }
            }
            else if (anim_phase == DP::FangGlow2) {
                weapon_tint = vd.transform_magic_color;
                limb_tint = weapon_tint;
            }
            else if (anim_phase == DP::FangScreenGlow0) {
                if (vd.revive_sound) vd.revive_sound->play();
            }
            else if (anim_phase == DP::BackToHuman0) {
                weapon_tint = {};
                limb_tint = {};
                for (usize i = 0; i < 4; i++) {
                    limb_pos[i] = GNAN;
                }
                if (desnakify_leaving_door) {
                     // Skip opening animation and sound
                    desnakify_leaving_door->state = DoorState::Open;
                    desnakify_leaving_door->pos =
                        desnakify_leaving_door->open_pos;
                }
            }
        }
        if (anim_timer >= vd.desnakify_sequence[anim_phase]) {
            anim_phase += 1;
            anim_timer = 0;
            return Walker_business();
        }
        else {
            anim_timer += 1;
            if (anim_phase >= DP::FangFloats0 && anim_phase <= DP::FangFloats2) {
                float sum = vd.desnakify_sequence[DP::FangFloats0] +
                            vd.desnakify_sequence[DP::FangFloats1] +
                            vd.desnakify_sequence[DP::FangFloats2];
                float acc = anim_timer;
                if (anim_phase > DP::FangFloats0) {
                    acc += vd.desnakify_sequence[DP::FangFloats0];
                }
                if (anim_phase > DP::FangFloats1) {
                    acc += vd.desnakify_sequence[DP::FangFloats1];
                }
                override_weapon_pos = pos + lerp(
                    left_flip(vd.desnakify_fang_pos[0]),
                    left_flip(vd.desnakify_fang_pos[1]),
                    ease_in_out_sin(acc / sum)
                );
            }
            else if (anim_phase >= DP::FangGlow0 && anim_phase <= DP::FangGlow2) {
                float sum = vd.desnakify_sequence[DP::FangGlow0] +
                            vd.desnakify_sequence[DP::FangGlow1] +
                            vd.desnakify_sequence[DP::FangGlow2];
                float acc = anim_timer;
                if (anim_phase > DP::FangGlow0) {
                    acc += vd.desnakify_sequence[DP::FangGlow0];
                }
                if (anim_phase > DP::FangGlow1) {
                    acc += vd.desnakify_sequence[DP::FangGlow1];
                }
                for (int i = 0; i < 4; i++) {
                    limb_pos[i] = lerp(
                        limb_initial_pos[i],
                        pos + left_flip(vd.desnakify_fang_pos[1]) +
                            vd.desnakify_limb_offsets[i],
                        ease_in_out_sin(acc / sum)
                    );
                }
            }
            else if (anim_phase >= DP::TakeFang0 && anim_phase <= DP::TakeFang1) {
                auto& poses = static_cast<VerdantPoses&>(*vd.poses);
                float sum = vd.desnakify_sequence[DP::TakeFang0] +
                            vd.desnakify_sequence[DP::TakeFang1];
                float acc = anim_timer;
                if (anim_phase > DP::TakeFang0) {
                    acc += vd.desnakify_sequence[DP::TakeFang0];
                }
                override_weapon_pos = pos + left_flip(lerp(
                    vd.desnakify_fang_pos[1],
                    poses.desnakify[DP::TakeFang1].body->weapon,
                    ease_in_out_sin(acc / sum)
                ));
            }
            else if (anim_phase >= DP::HoldFang) {
                override_weapon_pos = GNAN;
                override_weapon_scale = GNAN;
            }
            return WB::Frozen;
        }
    }
    return Walker::Walker_business();
}

void Verdant::Walker_move (const Controls& controls) {
    auto& vd = static_cast<VerdantData&>(*data);
    if (state == VS::Inch) {
        expect(business == WB::Free);
        if (controls[Control::Right]) {
            left = false;
            if (anim_phase == 0) {
                anim_phase = 1;
                anim_timer = 1;
                pos.x += 1;
            }
            else if (anim_timer >= vd.inch_sequence[anim_phase-1]) {
                pos.x += 1;
                anim_phase = anim_phase == 2 ? 1 : 2;
                anim_timer = 0;
            }
            else anim_timer += 1;
        }
        else {
            if (anim_phase == 1) pos.x -= 1;
            anim_phase = 0;
            anim_timer = 0;
        }
        return;
    }
    else if (state == VS::Snake || state == VS::SnakeAttack) {
        bool decelerate = false;
        restart_move:
        if (business == WB::Frozen) vel = {0, 0};
        else if (business == WB::HoldAttack) {
            if (!controls[Control::Attack]) {
                anim_phase = 1;
                anim_timer = 0;
                business = Walker_business();
                goto restart_move;
            }
            decelerate = true;
        }
        else if (business == WB::DoAttack || business == WB::Occupied) {
            decelerate = true;
        }
        else if (business == WB::Interruptible || business == WB::Free) {
            if (controls[Control::Left] && !controls[Control::Right]) {
                if (business == WB::Interruptible) set_state(VS::Snake);
                tongue_timer = 0;
                left = true;
                if (vel.x > -vd.snake_max) {
                    vel.x -= vd.snake_acc;
                    if (vel.x < -vd.snake_max) vel.x = -vd.snake_max;
                }
            }
            else if (controls[Control::Right]) {
                if (business == WB::Interruptible) set_state(VS::Snake);
                tongue_timer = 0;
                left = false;
                if (vel.x < vd.snake_max) {
                    vel.x += vd.snake_acc;
                    if (vel.x > vd.snake_max) vel.x = vd.snake_max;
                }
            }
            else decelerate = true;
            if (controls[Control::Jump]) {
                if (floor && controls[Control::Jump] < vd.hold_buffer) {
                    if (controls[Control::Down]) {
                         // Drop through semisolid
                        if (floor->types & Types::Semisolid) {
                            pos.y -= min(data->jump_crouch_lift, 4);
                            floor = null;
                            crouch = false;
                            no_crouch_timer = 30;
                        }
                    }
                    else {
                        vel.y += vd.snake_jump_vel;
                        floor = null;
                    }
                    if (business == WB::Interruptible) set_state(VS::Snake);
                }
                drop_timer = 0;
            }
            else if (drop_timer < data->drop_duration) {
                drop_timer += 1;
            }
            if (controls[Control::Attack] &&
                controls[Control::Attack] <= data->hold_buffer
            ) {
                attack_done = false;
                set_state(VS::SnakeAttack);
            }
        }
        if (decelerate) {
            if (floor) {
                if (vel.x > 0) {
                    vel.x -= vd.snake_dec;
                    if (vel.x < 0) vel.x = 0;
                }
                else {
                    vel.x += vd.snake_dec;
                    if (vel.x > 0) vel.x = 0;
                }
                walk_start_x = pos.x;
            }
        }
        vel.y -= gravity();
        pos += vel;
        return;
    }
    else if (state == VS::SnakeAttack) {
        if (business == WB::HoldAttack) {
            if (floor) {
                if (vel.x > 0) {
                    vel.x -= vd.snake_dec;
                    if (vel.x < 0) vel.x = 0;
                }
                else {
                    vel.x += vd.snake_dec;
                    if (vel.x > 0) vel.x = 0;
                }
                walk_start_x = pos.x;
            }
        }
        vel.y -= gravity();
        pos += vel;
        return;
    }
    return Walker::Walker_move(controls);
}

void Verdant::Walker_set_hitboxes () {
    auto& vd = static_cast<VerdantData&>(*data);
    if (state == VS::Snake || state == VS::SnakeAttack) {
        clear_hitboxes();
        body_hb.box = left_flip(vd.snake_box);
        add_hitbox(body_hb);
        if (business != WB::Frozen) {
            damage_hb.box = left_flip(vd.snake_box);
            add_hitbox(damage_hb);
        }
        if (business == WB::DoAttack && !attack_done) {
            weapon_hb.box = left_flip(vd.snake_attack_box);
            add_hitbox(weapon_hb);
        }
        return;
    }
    Walker::Walker_set_hitboxes();
}

void Verdant::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (o.types & Types::Door) {
        if (state == WS::Dead) return;
        auto& door = static_cast<Door&>(o);
        if (door.state != DoorState::Closed || door.pos == door.closed_pos) {
            goto not_handled;
        }
        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
         // Allow a bit of grace.  TODO: Make horizontal situation better.
        if (height(overlap) < 12) return;
        else if (width(overlap) <= 3) {
            goto not_handled;
        }
        else if (overlap.t == here.t) {
            if (floor) {
                set_state(WS::Dead);
                anim_phase = 3;
                damage_forward = true;
                pos.y += data->dead_floor_lift;
                vel.x = 0;
                door.crush = true;
            }
            pos.y -= height(overlap);
            if (vel.y > 0) vel.y = 0;
        }
        else goto not_handled;
    }
    not_handled: return Walker::Resident_on_collide(hb, o, o_hb);
}

void Verdant::Walker_on_hit (
    const Hitbox& hb, Walker& victim, const Hitbox& o_hb
) {
    if (state == VS::Snake || state == VS::SnakeAttack) {
        auto& vd = static_cast<VerdantData&>(*data);
        if (vd.snake_bite_sound) vd.snake_bite_sound->play();
        victim.poison_level += 4;
        victim.poison_timer = 360;
        if (victim.types & Types::Indigo) {
            indigo = &static_cast<Indigo&>(victim);
            expect(indigo->state == IS::Bed);
            indigo->set_state(IS::Bit);
            set_state(VS::SnakeBite);
        }
        else {
            victim.set_state(WS::Stun);
            victim.stun_duration = 30;
        }
        return;
    }
     // Find place to stab
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    auto& victim_body = *victim.data->poses->damage.body;
    usize min_dist = GINF;
    usize decal_i = -1;
    float weapon_y = pos.y + weapon_offset.y;
    for (usize i = 0; i < max_decals; i++) {
        float decal_y = victim.pos.y + victim_body.decals[i].y;
        float dist = distance(decal_y, weapon_y);
        if (dist < min_dist) {
            min_dist = dist;
            decal_i = i;
        }
    }
     // Calculate stab depth
    float weapon_tip = pos.x + left_flip(
        weapon_offset.x + data->poses->hit[0].weapon->hitbox.r
    );
    Vec decal_pos = victim.pos + victim.left_flip(victim_body.decals[decal_i]);
    float stab_depth = left_flip(weapon_tip - decal_pos.x);
     // Stab
    victim.decal_type = DecalType::Stab;
    victim.decal_index = decal_i;
    if (victim.data->flavor == WF::Lemon) pending_weapon_layers |= 0x8;
    else if (stab_depth > 12) pending_weapon_layers |= 0x4;
    else pending_weapon_layers |= 0x2;
     // Move victim vertically
    float height_diff = decal_pos.y - weapon_y;
    victim.pos.y -= height_diff;
     // Move victor horizontally
    if (stab_depth < -2) {
        pos.x += left_flip(2 - stab_depth);
    }
    else if (stab_depth > 16) {
        pos.x -= left_flip(stab_depth - 16);
    }
     // Supercall
    Walker::Walker_on_hit(hb, victim, o_hb);
}

Pose Verdant::Walker_pose () {
    auto& vd = static_cast<VerdantData&>(*data);
    auto poses = static_cast<VerdantPoses&>(*vd.poses);
    if (state == VS::PreTransform) {
        return poses.walk_hold[walk_frame()];
    }
    else if (state == VS::Transform) {
        Pose r;
        expect(anim_phase < 14);
        r = poses.transform[anim_phase];
         // Wave hair magically
        if (anim_phase >= 3 && anim_phase <= 10) {
            r.head = poses.walk[(transform_timer / 8) % 6].head;
        }
        if (anim_phase == 9) weapon_layers = 0x5;
        else if (anim_phase == 10) weapon_layers = 0x1;
        return r;
    }
    else if (state == WS::Stun) {
        if (poison_level == 2 && paralyze_symbol_timer) {
            return floor ? poses.downf : poses.damagefallf;
        }
    }
    else if (state == WS::Dead) {
        if (damage_forward) {
            Pose r;
             // Mirror the WS::Dead case in Walker::Walker_pos
            switch (anim_phase) {
                case 0: case 1: case 2: r = poses.damagef; break;
                case 3: r = poses.damagefallf; break;
                default: r = poses.deadf[anim_phase - 4]; break;
            }
            return r;
        }
    }
    else if (state == VS::FangHelp) {
        return damage_forward ? poses.deadf[6] : poses.dead[6];
    }
    else if (state == VS::Captured) {
        if (indigo->anim_phase == CP::MoveTarget) {
            return poses.captured_damage;
        }
        else return poses.captured;
    }
    else if (state == VS::CapturedWeaponTaken) {
        expect(anim_phase < 3);
        return poses.weapon_taken[anim_phase];
    }
    else if (state == VS::CapturedWeaponBroken) {
        expect(anim_phase < 9);
        return poses.weapon_broken[anim_phase];
    }
    else if (state == VS::CapturedLimbsDetached) {
        if (indigo->anim_phase < CP::TakeLimbs) {
            return poses.limbs_detached;
        }
        else return poses.limbs_taken;
    }
    else if (state == VS::Limbless) {
        if (floor) return poses.limbless;
        else return poses.limbless_fall;
    }
    else if (state == VS::Inch) {
        expect(anim_phase < 3);
        if (anim_phase == 0) {
            return poses.inch[pos.x == walk_start_x ? 0 : 2];
        }
        else return poses.inch[anim_phase];
    }
    else if (state == VS::Snakify) {
        expect(anim_phase < 8);
        if (anim_phase < 6) {
            return poses.inch[2];
        }
        else return poses.snake_stand;
    }
    else if (state == VS::Snake) {
        if (floor) {
            if (!defined(walk_start_x)) walk_start_x = pos.x;
            float dist = distance(walk_start_x, pos.x);
            if (dist >= 1) {
                uint32 i = geo::floor(dist / vd.snake_walk_cycle_dist) % 4;
                return poses.snake_walk[i];
            }
            return poses.snake_stand;
        }
        else return poses.snake_walk[0];
    }
    else if (state == VS::SnakeAttack) {
        expect(anim_phase < 6);
        return poses.snake_attack[anim_phase];
    }
    else if (state == VS::SnakeBite) {
        expect(indigo && indigo->anim_phase < 9);
        return poses.snake_bite[indigo->anim_phase];
    }
    else if (state == VS::SnakeCaptured) {
        return poses.snake_captured;
    }
    else if (state == VS::SnakeEat) {
        expect(anim_phase < 34);
        return poses.eat[anim_phase];
    }
    else if (state == VS::Desnakify) {
        expect(anim_phase < DP::N_Phases);
        if (anim_phase == DP::WalkAway) {
            return poses.walk_hold[walk_frame()];
        }
        return poses.desnakify[anim_phase];
    }
    return Walker::Walker_pose();
}

void Verdant::Walker_draw_weapon (const Pose& pose) {
    auto& vd = static_cast<VerdantData&>(*data);
    auto& poses = static_cast<VerdantPoses&>(*vd.poses);
    Vec scale = {left ? -1 : 1, 1};
     // Draw limbs if they have a position
    for (usize i = 0; i < 4; i++) {
        if (defined(limb_pos[i])) {
            draw_layers(
                *poses.captured_limbs[i], limb_layers,
                limb_pos[i],
                pose.z + poses.captured_limbs[i]->z_offset, {1, 1},
                limb_tint
            );
        }
    }
    if (state == VS::Transform) {
        Vec weapon_offset;
        glow::RGBA8 tint = weapon_tint;
        uint8 weapon_layers = 1;
        if (anim_phase == TP::Lifting || anim_phase == TP::Waiting) {
            float end = vd.transform_sequence[TP::Lifting] +
                        vd.transform_sequence[TP::Waiting];
             // anim_timer ranges from 1 to n here
            float t = anim_timer - 1;
            if (anim_phase == TP::Waiting) {
                t += vd.transform_sequence[TP::Lifting];
            }
            weapon_offset = lerp(
                Vec(poses.transform[TP::Lifting].body->weapon),
                Vec(poses.transform[TP::Waiting].body->weapon),
                ease_in_out_sin(t / end)
            );
        }
        else if (anim_phase == TP::Settling2 || anim_phase == TP::Receiving) {
            float end = vd.transform_sequence[TP::Settling2] +
                        vd.transform_sequence[TP::Receiving];
            float t = anim_timer - 1;
            if (anim_phase == TP::Receiving) {
                t += vd.transform_sequence[TP::Settling2];
            }
            weapon_offset = lerp(
                 // These are backwards on purpose
                Vec(poses.transform[TP::Receiving].body->weapon),
                Vec(poses.transform[TP::Settling2].body->weapon),
                ease_in_out_sin(1 - (t / end))
            );
        }
        else {
            weapon_offset = pose.body->weapon;
        }
        if (anim_phase == TP::Holding ||
            anim_phase == TP::Waiting ||
            anim_phase == TP::Naked
        ) {
             // Stick tongue out
            if (anim_timer >= 10 && anim_timer < 22) {
                if (anim_timer >= 14 && anim_timer < 18) {
                    weapon_layers = 0x7;
                }
                else weapon_layers = 0x3;
            }
        }
        else if (anim_phase == TP::Assembling2) {
             // Glow
            tint = vd.transform_magic_color;
        }
        draw_layers(
            *pose.weapon, weapon_layers,
            pos + weapon_offset * scale,
            pose.z + Z::WeaponOffset,
            scale, tint
        );
        return;
    }
    else if (state == VS::FangHelp) {
        Vec initial_pos = left_flip(pose.body->weapon);
        Vec final_pos = visual_center() + Vec(0, 14);
        uint32 current_time = anim_timer;
        uint32 total_time = 0;
        for (usize i = 0; i < 5; i++) {
            if (anim_phase > i) {
                current_time += vd.fang_help_sequence[i];
            }
            total_time += vd.fang_help_sequence[i];
        }
        float t = ease_out_sin(float(current_time) / total_time);
        draw_layers(
            *poses.fang_help[anim_phase],
            weapon_layers,
            pos + lerp(initial_pos, final_pos, t),
            pose.z + Z::WeaponOffset, scale,
            anim_phase == 1 ? vd.transform_magic_color : weapon_tint
        );
        return;
    }
    else if (state == VS::Snakify) {
        if (anim_phase >= 3) {
            glow::RGBA8 screen = vd.transform_magic_color;
            if (anim_phase == 3 || anim_phase == 7) {
                screen.a *= float(0x60) / 0xff;
            }
            else if (anim_phase == 4 || anim_phase == 6) {
                screen.a *= float(0xa0) / 0xff;
            }
            draw_rectangle(
                Rect(Vec(0, 0), camera_size),
                screen, Z::ScreenEffect
            );
        }
    }
    else if (state == VS::Desnakify) {
        uint8 a;
        switch (anim_phase) {
            case DP::FullScreenGlow0: case DP::FangRevived1:
            case DP::FangScreenGlow0: case DP::BackToHuman1:
                a = 0x60; break;
            case DP::FullScreenGlow1: case DP::FangRevived0:
            case DP::FangScreenGlow1: case DP::BackToHuman0:
                a = 0xa0; break;
            case DP::FullScreenGlow2: case DP::FangScreenGlow2:
                a = 0xff; break;
            default: a = 0; break;
        }
        if (a) {
            auto screen = vd.transform_magic_color;
            screen.a = a;
            draw_rectangle(
                Rect(Vec(0, 0), camera_size),
                screen, Z::ScreenEffect
            );
        }
    }
    Walker::Walker_draw_weapon(pose);
}

void Verdant::Resident_on_exit () {
    decal_type = DecalType::None;
    decal_index = -1;
    poison_level = 0;
    stun_duration = 0;
    paralyze_symbol_timer = 0;
    weapon_layers = 0x1;
    pending_weapon_layers = 0x1;
    Walker::Resident_on_exit();
}

Controls VerdantMind::Mind_think (Resident& r) {
    if (r.types & Types::Verdant) {
        auto& v = static_cast<Verdant&>(r);
        if (v.state == VS::PreTransform) {
            auto& vd = static_cast<VerdantData&>(*v.data);
            auto dir = v.pos.x < vd.transform_pos.x
                ? Control::Right : Control::Left;
            Controls r = {};
            r[dir] = 1;
            return r;
        }
        else if (v.state == VS::Desnakify && v.anim_phase == DP::WalkAway) {
            Controls r = {};
            r[Control::Right] = 1;
            return r;
        }
    }
    return next->Mind_think(r);
}

Verdant* find_verdant () {
    if (auto game = current_game)
    if (auto room = game->state().current_room)
    if (auto v = room->find_with_types(Types::Verdant)) {
        return static_cast<Verdant*>(v);
    }
    return null;
}

void restart_if_dead_ () {
    if (!current_game || current_game->menus) return;
    if (auto v = find_verdant()) {
        if (v->state == WS::Dead || v->state == VS::FangHelp) {
            v->go_to_limbo();
            v->revive_phase = 1;
            v->revive_timer = 0;
        }
    }
}
control::Command restart_if_dead (restart_if_dead_, "restart_if_dead", "Restart from checkpoint if player is dead");

void force_restart_ () {
    if (!current_game || current_game->menus) return;
    if (auto v = find_verdant()) {
        v->go_to_limbo();
        v->revive_phase = 1;
        v->revive_timer = 0;
    }
}
control::Command force_restart (force_restart_, "force_restart", "Restart from checkpoint even if player is alive.");

void set_body_layers_ (uint8 layers) {
    if (auto v = find_verdant()) {
        v->body_layers = layers;
    }
}
control::Command set_body_layers (set_body_layers_, "set_body_layers");

void do_transform_sequence_ () {
    if (auto v = find_verdant()) {
        v->set_state(VS::Transform);
    }
}
control::Command do_transform_sequence (do_transform_sequence_, "do_transform_sequence");

void become_snake_ () {
    if (auto v = find_verdant()) {
        v->set_state(VS::Snake);
    }
}
control::Command become_snake (become_snake_, "become_snake");

void unbecome_snake_ () {
    if (auto v = find_verdant()) {
        v->set_state(VS::Desnakify);
        for (usize i = 0 ; i < 4; i++) {
            if (!defined(v->limb_pos[i])) {
                v->limb_pos[i] = Vec(256, 32);
            }
        }
    }
}
control::Command unbecome_snake (unbecome_snake_, "unbecome_snake");

void debug_info_ () {
    if (!current_game) return;
    auto& state = current_game->state();
    if (!state.current_room) return;
    for (auto r : state.current_room->residents) {
        ayu::dump(r, r->pos);
    }
}
control::Command debug_info (debug_info_, "debug_info");

} using namespace vf;

AYU_DESCRIBE(vf::LimbFrame,
    elems(
        elem(&LimbFrame::target),
        elem(&LimbFrame::offset),
        elem(&LimbFrame::attached),
        elem(&LimbFrame::detached),
        elem(&LimbFrame::z_offset)
    ),
    attrs(
        attr("vf::Frame", base<Frame>(), include)
    ),
    init([](LimbFrame& v){ v.init(); })
)

AYU_DESCRIBE(vf::VerdantPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("walk_hold", &VerdantPoses::walk_hold),
        attr("transform", &VerdantPoses::transform),
        attr("damagef", &VerdantPoses::damagef),
        attr("damagefallf", &VerdantPoses::damagefallf),
        attr("downf", &VerdantPoses::downf),
        attr("deadf", &VerdantPoses::deadf),
        attr("fang_help", &VerdantPoses::fang_help),
        attr("captured", &VerdantPoses::captured),
        attr("captured_damage", &VerdantPoses::captured_damage),
        attr("weapon_taken", &VerdantPoses::weapon_taken),
        attr("weapon_broken", &VerdantPoses::weapon_broken),
        attr("limbs_detached", &VerdantPoses::limbs_detached),
        attr("limbs_taken", &VerdantPoses::limbs_taken),
        attr("limbless_fall", &VerdantPoses::limbless_fall),
        attr("limbless", &VerdantPoses::limbless),
        attr("captured_limbs", &VerdantPoses::captured_limbs),
        attr("inch", &VerdantPoses::inch),
        attr("snake_stand", &VerdantPoses::snake_stand),
        attr("snake_walk", &VerdantPoses::snake_walk),
        attr("snake_attack", &VerdantPoses::snake_attack),
        attr("snake_bite", &VerdantPoses::snake_bite),
        attr("snake_captured", &VerdantPoses::snake_captured),
        attr("eat", &VerdantPoses::eat),
        attr("desnakify", &VerdantPoses::desnakify)
    )
)

AYU_DESCRIBE(vf::CutsceneSound,
    elems(
        elem(&CutsceneSound::phase),
        elem(&CutsceneSound::timer),
        elem(&CutsceneSound::sound)
    )
)

AYU_DESCRIBE(vf::VerdantData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("center", &VerdantData::center),
        attr("dead_center", &VerdantData::dead_center),
        attr("dead_center_forward", &VerdantData::dead_center_forward),
        attr("transform_sequence", &VerdantData::transform_sequence),
        attr("transform_pos", &VerdantData::transform_pos),
        attr("transform_magic_color", &VerdantData::transform_magic_color),
        attr("transform_sounds", &VerdantData::transform_sounds),
        attr("revive_sequence", &VerdantData::revive_sequence),
        attr("revive_tint", &VerdantData::revive_tint),
        attr("fang_help_sequence", &VerdantData::fang_help_sequence),
        attr("weapon_taken_sequence", &VerdantData::weapon_taken_sequence),
        attr("weapon_broken_sequence", &VerdantData::weapon_broken_sequence),
        attr("fang_gravity", &VerdantData::fang_gravity),
        attr("fang_dead_y", &VerdantData::fang_dead_y),
        attr("limbless_sequence", &VerdantData::limbless_sequence),
        attr("inch_sequence", &VerdantData::inch_sequence),
        attr("snakify_sequence", &VerdantData::snakify_sequence),
        attr("snake_box", &VerdantData::snake_box),
        attr("snake_attack_box", &VerdantData::snake_attack_box),
        attr("snake_acc", &VerdantData::snake_acc),
        attr("snake_max", &VerdantData::snake_max),
        attr("snake_dec", &VerdantData::snake_dec),
        attr("snake_jump_vel", &VerdantData::snake_jump_vel),
        attr("snake_walk_cycle_dist", &VerdantData::snake_walk_cycle_dist),
        attr("snake_attack_sequence", &VerdantData::snake_attack_sequence),
        attr("snake_attack_vel", &VerdantData::snake_attack_vel),
        attr("snake_tongue_cycle", &VerdantData::snake_tongue_cycle),
        attr("bite_indigo_offsets", &VerdantData::bite_indigo_offsets),
        attr("bite_release_vel", &VerdantData::bite_release_vel),
        attr("eat_sequence", &VerdantData::eat_sequence),
        attr("desnakify_sequence", &VerdantData::desnakify_sequence),
        attr("desnakify_fang_pos", &VerdantData::desnakify_fang_pos),
        attr("desnakify_limb_offsets", &VerdantData::desnakify_limb_offsets),
        attr("music_after_transform", &VerdantData::music_after_transform, optional),
        attr("unstab_sound", &VerdantData::unstab_sound, optional),
        attr("revive_sound", &VerdantData::revive_sound, optional),
        attr("spear_break_sound", &VerdantData::spear_break_sound, optional),
        attr("snake_death_sound", &VerdantData::snake_death_sound, optional),
        attr("limb_detach_sound", &VerdantData::limb_detach_sound, optional),
        attr("snake_bite_sound", &VerdantData::snake_bite_sound, optional),
        attr("snake_eat_sound", &VerdantData::snake_eat_sound, optional)
    )
)

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("damage_forward", &Verdant::damage_forward, optional),
        attr("transform_timer", &Verdant::transform_timer, optional),
        attr("limbo", &Verdant::limbo, optional),
        attr("revive_phase", &Verdant::revive_phase, optional),
        attr("revive_timer", &Verdant::revive_timer, optional),
        attr("limb_layers", &Verdant::limb_layers, optional),
        attr("limb_pos", &Verdant::limb_pos, optional),
        attr("limb_tint", &Verdant::limb_tint, optional),
        attr("indigo", &Verdant::indigo, optional),
        attr("fang_vel_y", &Verdant::fang_vel_y, optional),
        attr("tongue_timer", &Verdant::tongue_timer, optional),
        attr("desnakify_leaving_door", &Verdant::desnakify_leaving_door, optional)
    )
)

AYU_DESCRIBE(vf::VerdantMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("next", &VerdantMind::next)
    )
)
