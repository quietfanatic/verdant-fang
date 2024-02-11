#include "verdant.h"
#include "door.h"

namespace vf {

struct VerdantPoses : WalkerPoses {
    Pose deadf [3];
};

struct VerdantData : WalkerData {
    Sound* unhit_sound;
};

Verdant::Verdant () {
    types |= Types::Verdant;
    hbs[0].layers_2 |= Layers::LoadingZone_Verdant;
}

WalkerBusiness Verdant::Walker_business () {
    if (state == WS::Hit && anim_phase == 1 &&
        anim_timer == data->hit_sequence[1]
    ) {
        static_cast<VerdantData*>(data)->unhit_sound->play();
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
            set_state(WS::Dead);
            anim_phase = 3;
            damage_forward = true;
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
    if (stab_depth > 12) weapon_state = 2;
    else if (weapon_state < 1) weapon_state = 1;
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
    Pose r;
    if (state == WS::Dead && damage_forward) {
        auto poses = static_cast<VerdantPoses&>(*data->poses);
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
    else return Walker::Walker_pose();
}

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)

AYU_DESCRIBE(vf::VerdantPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("deadf", &VerdantPoses::deadf)
    )
)

AYU_DESCRIBE(vf::VerdantData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("unhit_sound", &VerdantData::unhit_sound)
    )
)
