#include "verdant.h"

#include "../../dirt/control/command.h"
#include "../game/camera.h"
#include "../game/game.h"
#include "../game/state.h"
#include "door.h"
#include "math.h"

namespace vf {

namespace VS {
    constexpr WalkerState PreTransform = WS::Custom + 0;
    constexpr WalkerState Transform = WS::Custom + 1;
    constexpr WalkerState FangHelp = WS::Custom + 2;
    constexpr WalkerState Captured = WS::Custom + 3;
     // Update world.ayu if this changes
    static_assert(PreTransform == 6);
};

struct LimbFrame : Frame {
     // Relative to body pos
    Vec attached;
    Vec detached;
    float z_offset;
};

struct VerdantPoses : WalkerPoses {
    Pose walk_hold [6];
    Pose transform [14];
    Pose damagef;
    Pose damagefallf;
    Pose downf;
    Pose deadf [7];
    Frame* fang_help [6];
    Pose captured [19];
    LimbFrame* captured_limbs [4];
    Pose inch [3];
};

struct TransformSound {
    Sound* sound;
    uint8 phase;
    uint8 timer;
};

struct VerdantData : WalkerData {
    Vec center;
    Vec dead_center;
    Vec dead_center_forward;
    uint8 transform_sequence [14];
    Vec transform_pos;
    glow::RGBA8 transform_magic_color;
    TransformSound transform_sounds [3];
     // Phases are:
     // 0 = spear form
     // 1 = spear form glow
     // 2 = snake form long
     // 3 = snake form uncurling
     // 4 = snake form normal (still rising)
     // 5 = snake_form_normal (timer stops)
    uint8 fang_help_sequence [5];
    uint8 revive_sequence [5];
    glow::RGBA8 revive_tint [6];
     // 0 = moving
     // 1 = spear taken
     // 2 = spear rotates 1
     // 3 = spear rotates 2
     // 4 = spear broken
     // 5 = spear broken (glow)
     // 6 = snake broken
     // 7 = snake falls
     // 8 9 10 11 = snake on floor
     // 12 = right arm taken
     // 13 = left leg taken
     // 14 = left arm taken
     // 15 = right leg taken
     // 16 = limbs leaving
     // 17 = falling (no index in this array
     // 18 = landed (no index in this array)
    uint8 captured_sequence [17];
    uint8 captured_limb_phases [4];
    Vec captured_pos;
    Vec captured_fang_pos_high;
    Vec captured_fang_pos_low;
    Sound* unhit_sound;
};

 // Overrides movement during pre-transformation cutscene
struct VerdantMind : Mind {
    Mind* next;
    Controls Mind_think (Resident&) override;
};

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
            .enter_at = 0
        };
        set_transition_center(focus);
    }
}

WalkerBusiness Verdant::Walker_business () {
    auto& vd = static_cast<VerdantData&>(*data);
     // Treat revive animation separately from other stuff
    if (revive_phase) {
        auto& gs = current_game->state();
        if (!gs.transition) {
            expect(revive_phase < 7);
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
    if (state != WS::Stun && state != WS::Dead && state != VS::FangHelp) {
        damage_forward = false;
    }
    switch (state) {
        case VS::PreTransform: {
            expect(anim_phase == 0);
            if (pos.x >= vd.transform_pos.x) {
                set_state(VS::Transform);
                transform_timer = 0;
                return Walker_business();
            }
            else return WB::Free;
        }
        case VS::Transform: {
            transform_timer += 1;
            expect(anim_phase < 14);
            for (auto& ts : vd.transform_sounds) {
                if (anim_phase == ts.phase && anim_timer == ts.timer) {
                    ts.sound->play();
                }
            }
            if (anim_timer >= vd.transform_sequence[anim_phase]) {
                if (anim_phase == 13) {
                    transform_timer = 0;
                    set_state(WS::Neutral);
                    current_game->state().save_checkpoint(pos + visual_center());
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
        case WS::Hit: {
            if (anim_phase == 1 && anim_timer == data->hit_sequence[1]) {
                static_cast<VerdantData*>(data)->unhit_sound->play();
            }
            break;
        }
        case WS::Stun: {
            if (poison_level == 2 && anim_timer == 0 && paralyze_symbol_timer && floor) {
                damage_forward = true;
                if (floor) pos.y += data->dead_floor_lift;
            }
            break;
        }
        case WS::Dead: {
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
            break;
        }
        case VS::FangHelp: {
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
        case VS::Captured: {
            left = false;
            expect(anim_phase < 19);
            if (anim_phase == 17) {
                if (floor) anim_phase = 18;
                return WB::Occupied;
            }
            else if (anim_phase == 18) {
                return WB::Occupied;
            }
            else if (anim_timer >= vd.captured_sequence[anim_phase]) {
                anim_phase += 1;
                anim_timer = 0;
                return Walker_business();
            }
            else {
                if (anim_phase == 0 && anim_timer == 0) {
                    captured_start_pos = pos;
                }
                anim_timer += 1;
                 // Move limbs
                auto& poses = static_cast<VerdantPoses&>(*vd.poses);
                for (uint i = 0; i < 4; i++) {
                    if (anim_phase < vd.captured_limb_phases[i]) {
                        limb_pos[i] = pos + poses.captured_limbs[i]->attached;
                    }
                    else if (anim_phase == vd.captured_limb_phases[i]) {
                         // Note that anim_timer goes from 1..n here instead of
                         // 0..n-1, so the phase will end with limb_pos[i] ==
                         // poses.captured_limbs[i].detached.
                        float t = float(anim_timer) /
                            vd.captured_sequence[anim_phase];
                        limb_pos[i] = pos + lerp(
                            poses.captured_limbs[i]->attached,
                            poses.captured_limbs[i]->detached,
                            ease_out_sin(t)
                        );
                    }
                     // else leave it where it is.  Indigo will take it away.
                }
                return WB::Frozen;
            }
        }
        default: break;
    }
    return Walker::Walker_business();
}

void Verdant::Walker_move (const Controls& controls) {
    auto& vd = static_cast<VerdantData&>(*data);
    if (state == VS::Captured) {
        if (anim_phase == 0) {
            float t = float(anim_timer) / vd.captured_sequence[0];
            pos = lerp(captured_start_pos, vd.captured_pos, ease_in_out_sin(t));
        }
        else if (anim_phase == 1) {
            pos = vd.captured_pos;
        }
    }
    return Walker::Walker_move(controls);
}

void Verdant::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (o.types & Types::Door) {
        if (state == WS::Dead) return;
        auto& door = static_cast<Door&>(o);
        if (door.open || door.pos == door.closed_pos) goto not_handled;
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
    auto poses = static_cast<VerdantPoses&>(*data->poses);
    switch (state) {
        case VS::PreTransform: {
            return poses.walk_hold[walk_frame()];
        }
        case VS::Transform: {
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
        case WS::Stun: {
            if (poison_level == 2 && paralyze_symbol_timer) {
                return floor ? poses.downf : poses.damagefallf;
            }
            else break;
        }
        case WS::Dead: {
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
            else break;
        }
        case VS::FangHelp: {
            return damage_forward ? poses.deadf[6] : poses.dead[6];
        }
        case VS::Captured: {
            expect(anim_phase < 19);
            return poses.captured[anim_phase];
        }
        default: break;
    }
    return Walker::Walker_pose();
}

void Verdant::Walker_draw_weapon (const Pose& pose) {
    auto& vd = static_cast<VerdantData&>(*data);
    auto& poses = static_cast<VerdantPoses&>(*vd.poses);
    Vec scale = {left ? -1 : 1, 1};
    if (state == VS::Transform) {
        Vec weapon_offset;
        glow::RGBA8 tint = weapon_tint;
        uint8 weapon_layers = 1;
        if (anim_phase == 1 || anim_phase == 2) {
            float end = vd.transform_sequence[1] + vd.transform_sequence[2];
             // anim_timer ranges from 1 to n here
            float t = anim_timer - 1;
            if (anim_phase == 2) t += vd.transform_sequence[1];
            weapon_offset = lerp(
                Vec(poses.transform[1].body->weapon),
                Vec(poses.transform[2].body->weapon),
                ease_in_out_sin(t / end)
            );
        }
        else if (anim_phase == 12 || anim_phase == 13) {
            float end = vd.transform_sequence[12] + vd.transform_sequence[13];
            float t = anim_timer - 1;
            if (anim_phase == 13) t += vd.transform_sequence[12];
            weapon_offset = lerp(
                Vec(poses.transform[13].body->weapon),
                Vec(poses.transform[12].body->weapon),
                ease_in_out_sin(1 - (t / end))
            );
        }
        else {
            weapon_offset = pose.body->weapon;
        }
        if (anim_phase == 0 || anim_phase == 2 || anim_phase == 6) {
             // Stick tongue out
            if (anim_timer >= 10 && anim_timer < 22) {
                if (anim_timer >= 14 && anim_timer < 18) {
                    weapon_layers = 0x7;
                }
                else weapon_layers = 0x3;
            }
        }
        else if (anim_phase == 9) {
             // Glow
            tint = vd.transform_magic_color;
        }
        draw_layers(
            *pose.weapon, weapon_layers,
            pos + weapon_offset * scale,
            pose.z + Z::WeaponOffset,
            scale, tint
        );
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
    }
    else if (state == VS::Captured) {
        if (anim_phase >= 2) scale.x = -scale.x;
        if (anim_phase == 0) {
            Walker::Walker_draw_weapon(pose);
        }
        else if (anim_phase == 1 || anim_phase == 2 || anim_phase == 3) {
            uint32 t = anim_timer;
            uint32 total = 0;
            for (usize i = 1; i <= 3; i++) {
                if (anim_phase > i) t += vd.captured_sequence[i];
                total += vd.captured_sequence[i];
            }
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b1001,
                lerp(
                    pos + poses.captured[1].body->weapon,
                    vd.captured_fang_pos_high,
                    ease_out_sin(float(t) / total)
                ),
                pose.z + Z::WeaponOffset, scale,
                weapon_tint
            );
        }
        else if (anim_phase == 4) {
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b1011,
                vd.captured_fang_pos_high,
                pose.z + Z::WeaponOffset, scale,
                weapon_tint
            );
        }
        else if (anim_phase == 5) {
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b11,
                vd.captured_fang_pos_high,
                pose.z + Z::WeaponOffset, scale,
                vd.transform_magic_color
            );
        }
        else if (anim_phase == 6) {
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b11,
                vd.captured_fang_pos_high,
                pose.z + Z::WeaponOffset, scale,
                weapon_tint
            );
        }
        else if (anim_phase == 7) {
             // I thought we would need a bunch of fancy math to reverse
             // calculate the gravity needed to make Fang fall the given
             // distance in the given time, but we can just think of it as an
             // animation (which it is) and ease_in_quadratic.
            float t = float(anim_timer) / vd.captured_sequence[anim_phase];
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b11,
                lerp(
                    vd.captured_fang_pos_high,
                    vd.captured_fang_pos_low,
                    ease_in_quadratic(t)
                ),
                pose.z + Z::WeaponOffset, scale,
                weapon_tint
            );
        }
        else {
            draw_layers(
                *poses.captured[anim_phase].weapon, 0b11,
                vd.captured_fang_pos_low,
                pose.z + Z::WeaponOffset, scale,
                weapon_tint
            );
        }
         // Draw limbs
        for (usize i = 0; i < 4; i++) {
            draw_layers(
                *poses.captured_limbs[i], body_layers | 0b1000,
                limb_pos[i],
                pose.z + poses.captured_limbs[i]->z_offset, {1, 1},
                body_tint
            );
        }
    }
    else Walker::Walker_draw_weapon(pose);
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

void do_captured_sequence_ () {
    if (auto v = find_verdant()) {
        v->set_state(VS::Captured);
    }
}
control::Command do_captured_sequence (do_captured_sequence_, "do_captured_sequence");

void print_pos_ () {
    if (auto v = find_verdant()) {
        ayu::dump(v->pos);
    }
}
control::Command print_pos (print_pos_, "print_pos");

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("damage_forward", &Verdant::damage_forward, optional),
        attr("transform_timer", &Verdant::transform_timer, optional),
        attr("limbo", &Verdant::limbo, optional),
        attr("revive_phase", &Verdant::revive_phase, optional),
        attr("revive_timer", &Verdant::revive_timer, optional),
        attr("captured_start_pos", &Verdant::captured_start_pos, optional),
        attr("limb_pos", &Verdant::limb_pos, optional)
    )
)

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
        attr("captured_limbs", &VerdantPoses::captured_limbs),
        attr("inch", &VerdantPoses::inch)
    )
)

AYU_DESCRIBE(vf::TransformSound,
    elems(
        elem(&TransformSound::sound),
        elem(&TransformSound::phase),
        elem(&TransformSound::timer)
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
        attr("captured_sequence", &VerdantData::captured_sequence),
        attr("captured_pos", &VerdantData::captured_pos),
        attr("captured_limb_phases", &VerdantData::captured_limb_phases),
        attr("captured_fang_pos_high", &VerdantData::captured_fang_pos_high),
        attr("captured_fang_pos_low", &VerdantData::captured_fang_pos_low),
        attr("unhit_sound", &VerdantData::unhit_sound)
    )
)

AYU_DESCRIBE(vf::VerdantMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("next", &VerdantMind::next)
    )
)
