#include "player.h"

namespace vf {

Player::Player () {
    bounds = {-8, 0, 8, 32};
}

Player::Fang::Fang () {
    bounds = {0, -1, 5, 1};
    layers_1 |= Layers::Fang_Block;
}

void Player::Resident_before_step () {
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

void Player::Fang::Resident_collide (Resident& other) {
    usize offset = (char*)&((Player*)this)->fang - (char*)this;
    auto wielder = (Player*)((char*)this - offset);
    wielder->fang_collide(other);
}

void Player::fang_collide (Resident& other) {
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

void Player::Resident_after_step () {
    Walker::Resident_after_step();
    fang.set_room(null);
}

} using namespace vf;

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Walker", base<Walker>(), include)
    )
)
