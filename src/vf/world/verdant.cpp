#include "verdant.h"

#include "../../dirt/control/command.h"
#include "../game/game.h"
#include "../game/state.h"
#include "door.h"

namespace vf {

namespace VS {
    constexpr WalkerState Transform = WS::Custom + 0;
     // Update world.ayu if this changes
    static_assert(Transform == 5);
};

struct VerdantPoses : WalkerPoses {
    Pose deadf [3];
    Pose transform [10];
};

struct VerdantData : WalkerData {
    uint8 transform_sequence [10];
    Sound* unhit_sound;
};

Verdant::Verdant () {
    types |= Types::Verdant;
    hbs[0].layers_2 |= Layers::LoadingZone_Verdant;
    body_layers = 0x7;
}

WalkerBusiness Verdant::Walker_business () {
    switch (state) {
        case VS::Transform: {
            expect(anim_phase < 10);
            auto vd = static_cast<VerdantData*>(data);
            if (anim_timer >= vd->transform_sequence[anim_phase]) {
                if (anim_phase == 9) {
                    set_state(WS::Neutral);
                }
                else {
                    anim_phase += 1;
                    anim_timer = 0;
                }
                return Walker_business();
            }
            else {
                anim_timer += 1;
                return WB::Occupied;
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
        case VS::Transform: {
            Pose r;
            expect(anim_phase < 10);
            r = poses.transform[anim_phase];
            if (anim_phase < 4) r.body_layers = 0x5;
            else if (anim_phase == 4) r.body_layers = 0x1;
            return r;
        }
        case WS::Dead: {
            if (damage_forward) {
                Pose r;
                 // Mirror the WS::Dead case in Walker::Walker_pos
                switch (anim_phase) {
                    case 0: case 1: case 2: {
                        r = poses.deadf[0];
                        r.damage_overlap = true;
                        break;
                    }
                    case 3: r = poses.deadf[1]; break;
                    default: r = poses.deadf[2]; break;
                }
                r.z = Z::Dead;
                return r;
            }
            else break;
        }
        default: break;
    }
    return Walker::Walker_pose();
}

void set_body_layers_ (uint8 layers) {
    if (auto game = current_game)
    if (auto room = game->state().current_room)
    if (auto v = room->find_with_types(Types::Verdant)) {
        static_cast<Verdant*>(v)->body_layers = layers;
    }
}
control::Command set_body_layers (set_body_layers_, "set_body_layers");

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)

AYU_DESCRIBE(vf::VerdantPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("deadf", &VerdantPoses::deadf),
        attr("transform", &VerdantPoses::transform)
    )
)

AYU_DESCRIBE(vf::VerdantData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("transform_sequence", &VerdantData::transform_sequence),
        attr("unhit_sound", &VerdantData::unhit_sound)
    )
)
