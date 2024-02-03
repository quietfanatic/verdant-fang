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

struct PlayerFrames {
    Frame stand;
    Frame walk [6];
    Frame attack [2];
};

static glow::Texture* player_tex = null;
static PlayerFrames* player_frames = null;
static Sound* land_snd = null;

Player::Player () {
    bounds = {-8, 0, 8, 32};
    layers_1 = Layers::Player_Block;
    if (!player_tex) {
        ayu::global(&player_tex);
        player_tex = assets()["player_tex"][1];
    }
    if (!player_frames) {
        ayu::global(&player_frames);
        player_frames = assets()["player_frames"][1];
    }
    if (!land_snd) {
        ayu::global(&land_snd);
        land_snd = assets()["land_snd"][1];
    }
}

static constexpr float ground_acc = 0.4;
static constexpr float ground_max = 2.5;
static constexpr float ground_dec = 0.4;
static constexpr float air_acc = 0.2;
static constexpr float air_max = 2;
static constexpr float air_dec = 0;
static constexpr float jump_vel = 3.5;
static constexpr float gravity_jump = 0.1;
static constexpr float gravity_fall = 0.2;
static constexpr uint8 attack_sequence [] = {4, 6, 4};

void Player::Resident_before_step () {
    bool floaty = false;
    bool can_move = true;
     // Advance animations
    anim_timer += 1;
    if (state == PS::Attack) {
        expect(anim_phase < 3);
        if (anim_timer > attack_sequence[anim_phase]) {
            anim_timer = 0;
            anim_phase += 1;
        }
        if (anim_phase == 3) {
            anim_phase = 0;
            state = PS::Neutral;
        }
        else {
            can_move = false;
            floaty = true;
        }
    }
     // Now decide what to do and do it
    Controls controls = current_game->settings().get_controls();

    float acc = floor ? ground_acc : air_acc;
    float max = floor ? ground_max : air_max;
    float dec = floor ? ground_dec : air_dec;

    if (can_move && controls[Control::Left] && !controls[Control::Right]) {
        left = true;
        if (vel.x > -max) {
            vel.x -= acc;
            if (vel.x < -max) vel.x = -max;
        }
        if (vel.x >= 0) walk_start_x = pos.x;
    }
    else if (can_move && controls[Control::Right]) {
        left = false;
        if (vel.x < max) {
            vel.x += acc;
            if (vel.x > max) vel.x = max;
        }
        if (vel.x <= 0) walk_start_x = pos.x;
    }
    else {
        if (vel.x > 0) {
            vel.x -= dec;
            if (vel.x < 0) vel.x = 0;
        }
        else {
            vel.x += dec;
            if (vel.x > 0) vel.x = 0;
        }
        walk_start_x = pos.x;
    }
    if (can_move && controls[Control::Jump]) {
        if (floor) {
            vel.y += jump_vel;
            floor = null;
            state = PS::Neutral;
            anim_timer = 0;
        }
        floaty = true;
    }
    if (can_move && controls[Control::Attack]) {
        state = PS::Attack;
        anim_phase = 0;
        anim_timer = 0;
        floaty = true;
    }

    vel.y -= floaty ? gravity_jump : gravity_fall;

    pos += vel;
    new_floor = null;
}

void Player::Resident_collide (Resident& other) {
    expect(other.layers_2 & Layers::Player_Block);
    auto& block = static_cast<Block&>(other);

    Rect here = bounds + pos;
    Rect there = block.bounds + block.pos;
    Rect overlap = here & there;
    expect(proper(overlap));
    if (height(overlap) <= width(overlap)) {
        vel.y = 0;
        if (overlap.b == here.b) {
            pos.y += height(overlap);
            if (!floor && !new_floor) {
                land_snd->play();
            }
            new_floor = &block;
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
    if (new_floor && !floor) walk_start_x = pos.x;
    floor = new_floor;
    new_floor = null;
}

void Player::Resident_draw () {
    Frame* frame;
    switch (state) {
        case PS::Neutral: {
            if (floor) {
                float dist = distance(walk_start_x, pos.x);
                if (dist >= 1) {
                    frame = &player_frames->walk[geo::floor(dist / 16) % 6];
                }
                else frame = &player_frames->stand;
            }
            else frame = &player_frames->walk[0];
            break;
        }
        case PS::Attack: {
            if (anim_phase == 1) {
                frame = &player_frames->attack[1];
            }
            else frame = &player_frames->attack[0];
            break;
        }
        default: never();
    }
    draw_frame(pos, *frame, *player_tex, BVec{left, false});
}

} using namespace vf;

AYU_DESCRIBE(vf::PlayerState,
    values(
        value("neutral", PS::Neutral),
        value("attack", PS::Attack)
    )
)

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vel", &Player::vel, optional),
        attr("left", &Player::left, optional),
        attr("state", &Player::state, optional),
        attr("anim_phase", &Player::anim_phase, optional),
        attr("anim_timer", &Player::anim_timer, optional),
        attr("walk_start_x", &Player::walk_start_x, optional)
    )
)

AYU_DESCRIBE(vf::PlayerFrames,
    attrs(
        attr("stand", &PlayerFrames::stand),
        attr("walk", &PlayerFrames::walk),
        attr("attack", &PlayerFrames::attack)
    )
)
