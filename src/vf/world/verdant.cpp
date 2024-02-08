#include "verdant.h"

namespace vf {

Verdant::Verdant () {
    types |= Types::Verdant;
    hbs[0].layers_2 |= Layers::LoadingZone_Verdant;
}

void Verdant::Walker_on_hit (
    const Hitbox& hb, Walker& victim, const Hitbox& o_hb
) {
     // We haven't implemented backstabbing so turn victim around.
    victim.left = !left;
     // Find place to stab
    Vec weapon_offset = data->poses->attack[1].body->weapon;
    auto& victim_body = *victim.data->poses->damage[0].body;
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
        weapon_offset.x + data->poses->attack[1].weapon->hitbox.r
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
        pos.x += left_flip(-stab_depth);
    }
     // Supercall
    Walker::Walker_on_hit(hb, victim, o_hb);
}

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
