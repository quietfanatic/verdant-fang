#include "verdant.h"

#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/state.h"
#include "door.h"

namespace vf {

namespace VS {
    constexpr WalkerState PreTransform = WS::Custom + 0;
    constexpr WalkerState Transform = WS::Custom + 1;
     // Update world.ayu if this changes
    static_assert(PreTransform == 5);
};

struct VerdantPoses : WalkerPoses {
    Pose deadf [3];
    Pose walk_hold [6];
    Pose transform [14];
};

struct TransformSound {
    Sound* sound;
    uint8 phase;
    uint8 timer;
};

struct VerdantData : WalkerData {
    uint8 transform_sequence [14];
    Vec transform_pos;
    TransformSound transform_sounds [3];
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

WalkerBusiness Verdant::Walker_business () {
    auto& vd = static_cast<VerdantData&>(*data);
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
                    current_game->state().save_checkpoint();
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
        default: break;
    }
    return Walker::Walker_business();
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
        else if (width(overlap) - 1 < height(overlap) - 12) {
            goto not_handled;
        }
        else if (overlap.t == here.t) {
            if (floor) {
                set_state(WS::Dead);
                anim_phase = 3;
                damage_forward = true;
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
     // We haven't implemented backstabbing so turn victim around.
    victim.left = !left;
     // Find place to stab
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    auto& victim_body = *victim.data->poses->dead[0].body;
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
    if (stab_depth > 12) weapon_layers = 0x5;
    else if (weapon_layers == 0x1) weapon_layers = 0x3;
     // Move victim vertically
    float height_diff = decal_pos.y - weapon_y;
    victim.pos.y -= height_diff;
     // Move victor horizontally
    if (stab_depth < -2) {
        pos.x += left_flip(2 - stab_depth);
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
        case WS::Dead: {
            if (damage_forward) {
                Pose r;
                 // Mirror the WS::Dead case in Walker::Walker_pos
                switch (anim_phase) {
                    case 0: case 1: case 2: r = poses.deadf[0]; break;
                    case 3: r = poses.deadf[1]; break;
                    default: r = poses.deadf[2]; break;
                }
                return r;
            }
            else break;
        }
        default: break;
    }
    return Walker::Walker_pose();
}

void Verdant::Walker_draw_weapon (const Pose& pose) {
    if (state == VS::Transform) {
        auto& vd = static_cast<VerdantData&>(*data);
        auto& poses = static_cast<VerdantPoses&>(*vd.poses);
        Vec weapon_offset;
        uint8 weapon_layers = 1;
        Vec scale = {left ? -1 : 1, 1};
        if (anim_phase == 1 || anim_phase == 2) {
            float end = vd.transform_sequence[1] + vd.transform_sequence[2];
             // anim_timer ranges from 1 to n here
            float t = anim_timer - 1;
            if (anim_phase == 2) t += vd.transform_sequence[1];
            weapon_offset = lerp(
                Vec(poses.transform[1].body->weapon),
                Vec(poses.transform[2].body->weapon),
                std::sin(t / end * float(4 / M_PI))
            );
        }
        else if (anim_phase == 12 || anim_phase == 13) {
            float end = vd.transform_sequence[12] + vd.transform_sequence[13];
            float t = anim_timer - 1;
            if (anim_phase == 13) t += vd.transform_sequence[12];
            weapon_offset = lerp(
                Vec(poses.transform[13].body->weapon),
                Vec(poses.transform[12].body->weapon),
                1 - std::sin(t / end * float(4 / M_PI))
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
            weapon_layers = 5;
        }
        draw_layers(
            *pose.weapon, weapon_layers,
            pos + weapon_offset * scale,
            pose.z + Z::WeaponOffset,
            scale
        );
    }
    else Walker::Walker_draw_weapon(pose);
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

static Verdant* find_verdant () {
    if (auto game = current_game)
    if (auto room = game->state().current_room)
    if (auto v = room->find_with_types(Types::Verdant)) {
        return static_cast<Verdant*>(v);
    }
    return null;
}

void restart_if_dead_ () {
    if (auto v = find_verdant()) {
        if (v->state == WS::Dead) {
            current_game->state().load_checkpoint = true;
        }
    }
}
control::Command restart_if_dead (restart_if_dead_, "restart_if_dead", "Restart from checkpoint if player is dead");

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
        attr("transform_timer", &Verdant::transform_timer, optional)
    )
)

AYU_DESCRIBE(vf::VerdantPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("deadf", &VerdantPoses::deadf),
        attr("walk_hold", &VerdantPoses::walk_hold),
        attr("transform", &VerdantPoses::transform)
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
        attr("transform_sequence", &VerdantData::transform_sequence),
        attr("transform_pos", &VerdantData::transform_pos),
        attr("transform_sounds", &VerdantData::transform_sounds),
        attr("unhit_sound", &VerdantData::unhit_sound)
    )
)

AYU_DESCRIBE(vf::VerdantMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("next", &VerdantMind::next)
    )
)
