#include "player.h"

#include <SDL2/SDL_mixer.h>
#include "../../dirt/iri/iri.h"
#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/file-image.h"
#include "../../dirt/glow/image-texture.h"
#include "../../dirt/glow/texture-program.h"
#include "../game/camera.h"
#include "../game/frame.h"
#include "../game/game.h"
#include "../game/settings.h"
#include "../game/sound.h"
#include "block.h"

namespace vf {

struct BodyFrame : Frame {
    Vec head;
};

struct BodyFrames {
    BodyFrame stand;
    BodyFrame walk [6];
    BodyFrame attack [3];
    BodyFrame fall1;
};

struct HeadFrames {
    Frame neutral;
    Frame wave [3];
    Frame back;
    Frame fall [2];
};

struct Pose {
    BodyFrame* body = null;;
    Frame* head = null;
};

struct Poses {
    Pose stand;
    Pose walk [6];
    Pose attack [3];
    Pose jump [2];
    Pose fall [2];
};

struct PlayerData {
    glow::FileImage img;
    glow::PixelTexture body_tex;
    glow::PixelTexture head_tex;
    BodyFrames bodies;
    HeadFrames heads;
    Poses poses;
    Sound land_sfx;
    Sound stab_sfx;
    Sound step_sfx [5];
};

Player::Player () {
    bounds = {-8, 0, 8, 32};
    layers_1 = Layers::Player_Block;
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
static constexpr float gravity_drop = 0.3;
static constexpr uint8 drop_duration = 6;

static constexpr uint8 attack_sequence [] = {5, 6, 5};

 // For animation
static constexpr float jump_end_vel = 0.4;
static constexpr float fall_start_vel = -0.8;

void Player::Resident_before_step () {
    bool can_move = true;
     // Advance animations
    anim_timer += 1;
    if (state == PS::Attack) {
        expect(anim_phase < 3);
        if (anim_phase == 1 && anim_timer == 3) {
            data->stab_sfx.play();
        }
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
    if (can_move) {
        if (controls[Control::Jump]) {
            if (floor) {
                vel.y += jump_vel;
                floor = null;
                state = PS::Neutral;
                anim_timer = 0;
            }
            drop_timer = 0;
        }
        else if (!floor) drop_timer += 1;
        if (controls[Control::Attack]) {
            state = PS::Attack;
            anim_phase = 0;
            anim_timer = 0;
        }
    }

    if (vel.y > fall_start_vel || !defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    vel.y -= drop_timer == 0 ? gravity_jump
           : drop_timer <= drop_duration ? gravity_drop
           : gravity_fall;

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
        if (overlap.b == here.b) {
            pos.y += height(overlap);
            if (vel.y < 0) vel.y = 0;
             // Land on block
            if (!floor && !new_floor) {
                data->land_sfx.play();
            }
            new_floor = &block;
        }
        else if (overlap.t == here.t) {
            pos.y -= height(overlap);
            if (vel.y > 0) vel.y = 0;
        }
        else never();
    }
    else {
        if (overlap.l == here.l) {
            pos.x += width(overlap);
            if (vel.x < 0) vel.x = 0;
        }
        else if (overlap.r == here.r) {
            pos.x -= width(overlap);
            if (vel.x > 0) vel.x = 0;
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
    auto& poses = data->poses;
    Pose pose;
    switch (state) {
        case PS::Neutral: {
            if (floor) {
                float dist = distance(walk_start_x, pos.x);
                if (dist >= 1) {
                    pose = poses.walk[geo::floor(dist / 16) % 6];
                }
                else pose = poses.stand;
            }
            else {
                if (vel.y > jump_end_vel) {
                    pose = poses.jump[0];
                }
                else if (vel.y > fall_start_vel) {
                    pose = poses.jump[1];
                }
                else {
                    float dist = distance(fall_start_y, pos.y);
                    pose = poses.fall[geo::floor(dist / 16) % 2];
                }
            }
            break;
        }
        case PS::Attack: {
            expect(anim_phase < 3);
            pose = poses.attack[anim_phase];
             // If falling, use head from non-attacking poses
            if (!floor) {
                if (vel.y > jump_end_vel) {
                }
                else if (vel.y > fall_start_vel) {
                    pose.head = poses.jump[1].head;
                }
                else {
                    float dist = distance(fall_start_y, pos.y);
                    pose.head = poses.fall[geo::floor(dist / 16) % 2].head;
                }
            }
            break;
        }
        default: never();
    }
    Vec scale {left ? -1 : 1, 1};
    if (pose.body) {
        if (pose.head) {
            Vec head_pos = pos + pose.body->head * scale;
            draw_frame(head_pos, *pose.head, data->head_tex, scale);
        }
        draw_frame(pos, *pose.body, data->body_tex, scale);
    }
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
        attr("data", &Player::data),
        attr("vel", &Player::vel, optional),
        attr("left", &Player::left, optional),
        attr("state", &Player::state, optional),
        attr("anim_phase", &Player::anim_phase, optional),
        attr("anim_timer", &Player::anim_timer, optional),
        attr("drop_timer", &Player::drop_timer, optional),
        attr("walk_start_x", &Player::walk_start_x, optional),
        attr("fall_start_x", &Player::walk_start_x, optional),
        attr("floor", &Player::floor, optional)
    )
)

AYU_DESCRIBE(vf::BodyFrame,
    elems(
        elem(&BodyFrame::offset),
        elem(&BodyFrame::bounds),
        elem(&BodyFrame::head)
    )
)

AYU_DESCRIBE(vf::BodyFrames,
    attrs(
        attr("stand", &BodyFrames::stand),
        attr("walk", &BodyFrames::walk),
        attr("attack", &BodyFrames::attack),
        attr("fall1", &BodyFrames::fall1)
    )
)

AYU_DESCRIBE(vf::HeadFrames,
    attrs(
        attr("neutral", &HeadFrames::neutral),
        attr("wave", &HeadFrames::wave),
        attr("back", &HeadFrames::back),
        attr("fall", &HeadFrames::fall)
    )
)

AYU_DESCRIBE(vf::Pose,
    elems(
        elem(&Pose::body),
        elem(&Pose::head)
    )
)

AYU_DESCRIBE(vf::Poses,
    attrs(
        attr("stand", &Poses::stand),
        attr("walk", &Poses::walk),
        attr("attack", &Poses::attack),
        attr("jump", &Poses::jump),
        attr("fall", &Poses::fall)
    )
)

AYU_DESCRIBE(vf::PlayerData,
    attrs(
        attr("img", &PlayerData::img),
        attr("body_tex", &PlayerData::body_tex),
        attr("head_tex", &PlayerData::head_tex),
        attr("bodies", &PlayerData::bodies),
        attr("heads", &PlayerData::heads),
        attr("poses", &PlayerData::poses),
        attr("land_sfx", &PlayerData::land_sfx),
        attr("stab_sfx", &PlayerData::stab_sfx),
        attr("step_sfx", &PlayerData::step_sfx)
    )
)
