#include "player.h"

#include <SDL2/SDL_mixer.h>
#include "../dirt/iri/iri.h"
#include "../dirt/ayu/resources/global.h"
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/resource-texture.h"
#include "../dirt/glow/texture-program.h"
#include "assets.h"
#include "block.h"
#include "camera.h"
#include "game.h"
#include "settings.h"
#include "sound.h"

namespace vf {

static glow::Texture* player_tex = null;
static Sound* a_sound = null;

Player::Player () {
    box = {-0.5, 0, 0.5, 2};
    layers_1 = Layers::Player_Block;
    if (!player_tex) {
        ayu::global(&player_tex);
        player_tex = assets()["block"][1];
    }
    if (!a_sound) {
        ayu::global(&a_sound);
        a_sound = assets()["sound"][1];
    }
}

void Player::Resident_before_step () {
    Actions actions = current_game->settings().get_actions();
    switch (actions[Action::Right] - actions[Action::Left]) {
        case -1: {
            vel.x -= 0.02;
            if (vel.x < -0.2) vel.x = -0.2;
            break;
        }
        case 1: {
            vel.x += 0.02;
            if (vel.x > 0.2) vel.x = 0.2;
            break;
        }
        case 0: {
            if (vel.x > 0) {
                vel.x -= 0.02;
                if (vel.x < 0) vel.x = 0;
            }
            else if (vel.x < 0) {
                vel.x += 0.02;
                if (vel.x > 0) vel.x = 0;
            }
        }
    }
    if (standing) {
        if (actions[Action::Jump]) {
            vel.y = 0.2;
            standing = false;
        }
    }
    vel.y -= 0.01;
    pos += vel;
    floor = null;
}

void Player::Resident_collide (Resident& other) {
    expect(other.layers_2 & Layers::Player_Block);
    auto& block = static_cast<Block&>(other);

    Rect here = box + pos;
    Rect there = block.box + block.pos;
    Rect overlap = here & there;
    expect(proper(overlap));
    if (height(overlap) <= width(overlap)) {
        vel.y = 0;
        if (overlap.b == here.b) {
            pos.y += height(overlap);
            if (!standing && !floor) {
                a_sound->play();
            }
            floor = &block;
        }
        else if (overlap.t == here.t) {
            pos.y -= height(overlap);
        }
        else never();
    }
    else {
        if (overlap.l == here.l) {
            pos.x += width(overlap);
        }
        else if (overlap.r == here.r) {
            pos.x -= width(overlap);
        }
        else never();
    }
}

void Player::Resident_after_step () {
    standing = !!floor;
}

void Player::Resident_draw () {
    draw_texture(*player_tex, world_to_screen(box + pos));
}

} using namespace vf;

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vel", &Player::vel, optional)
    )
)
