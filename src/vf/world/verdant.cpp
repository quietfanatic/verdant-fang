#include "verdant.h"

namespace vf {

Verdant::Verdant () {
    types |= Types::Verdant;
}

void Verdant::Walker_on_hit (
    const Hitbox& hb, Walker& victim, const Hitbox& o_hb
) {
     // We haven't implemented backstabbing so turn victim around.
    victim.left = !left;
     // Find place to stab
    auto& victim_body = victim.data->body.damage[0];
    usize min_dist = GINF;
    usize decal_i = -1;
    float weapon_y = pos.y + data->body.attack[1].weapon.y;
    for (usize i = 0; i < max_decals; i++) {
        float decal_y = victim.pos.y + victim_body.decals[i].y;
        float dist = distance(decal_y, weapon_y);
        if (dist < min_dist) {
            min_dist = dist;
            decal_i = i;
        }
    }
     // Stab
    victim.decal_type = DecalType::Stab;
    victim.decal_index = decal_i;
     // Snap to stab location.
    float tip_offset = data->body.attack[1].weapon.x + data->phys.weapon_box.r;
    if (left) tip_offset = -tip_offset;
    float weapon_tip = pos.x + tip_offset;
    Vec decal_offset = victim_body.decals[decal_i];
    if (victim.left) decal_offset.x = -decal_offset.x;
    Vec decal_pos = victim.pos + decal_offset;
     // Move victor horizontally
    if (left) {
        if (decal_pos.x - weapon_tip < 2) {
            pos.x -= decal_pos.x - weapon_tip;
        }
    }
    else {
        if (weapon_tip - decal_pos.x < 2) {
            pos.x += weapon_tip - decal_pos.x;
        }
    }
     // Move victim vertically
    float height_diff = decal_pos.y - weapon_y;
    victim.pos.y -= height_diff;
     // Supercall
    Walker::Walker_on_hit(hb, victim, o_hb);
}

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
