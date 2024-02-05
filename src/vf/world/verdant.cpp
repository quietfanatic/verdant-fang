#include "verdant.h"

namespace vf {

Verdant::Verdant () { }

Verdant::Fang::Fang () {
    bounds = {0, -1, 5, 1};
    layers_1 |= Layers::Fang_Block;
}

void Verdant::Resident_before_step () {
    Walker::Resident_before_step();

     // Set up attack hitbox.  anim_timer was already incremented so it will
     // start at 1.
    if (state == WS::Attack && anim_phase == 1 && anim_timer == 1) {
        fang.set_room(room);
        Vec offset = data->bodies.attack[1].weapon;
        if (left) offset.x = -offset.x;
        fang.pos = pos + offset;
    }
}

void Verdant::Fang::Resident_collide (Resident& other) {
    usize offset = (char*)&((Verdant*)this)->fang - (char*)this;
    auto wielder = (Verdant*)((char*)this - offset);
    wielder->fang_collide(other);
}

void Verdant::fang_collide (Resident& other) {
    expect(other.layers_2 & Layers::Fang_Block);
    data->hit_solid_sfx.play();
    if (left) {
        vel.x += 1;
        if (vel.x < 0.5) vel.x = 0.5;
    }
    else {
        vel.x -= 1;
        if (vel.x > -0.5) vel.x = -0.5;
    }
}

void Verdant::Resident_after_step () {
    Walker::Resident_after_step();
    fang.set_room(null);
}

} using namespace vf;

AYU_DESCRIBE(vf::Verdant,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
