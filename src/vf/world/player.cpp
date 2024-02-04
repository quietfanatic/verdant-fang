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
#include "../game/state.h"
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
    BodyFrame land;
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
    Pose attack [4];
    Pose jump [4];
    Pose land;
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

 // Physics
static constexpr float ground_acc = 0.4;
static constexpr float ground_max = 2.5;
static constexpr float ground_dec = 0.4;
static constexpr float coast_dec = 0.1;
static constexpr float air_acc = 0.2;
static constexpr float air_max = 2;
static constexpr float air_dec = 0;

static constexpr float jump_vel = 3.4;
static constexpr float gravity_jump = 0.11;
static constexpr float gravity_fall = 0.2;
static constexpr float gravity_drop = 0.3;
static constexpr uint8 drop_duration = 6;

static constexpr uint8 attack_sequence [4] = {5, 6, 5, 200};
static constexpr uint8 land_sequence [2] = {3, 3};

void Player::set_state (PlayerState st) {
    state = st;
    anim_phase = 0;
    anim_timer = 0;
}

 // Animation
uint8 Player::walk_frame () {
    float dist = distance(walk_start_x, pos.x);
    return geo::floor(dist / 16) % 6;
}
static constexpr float jump_end_vel = 0.4;
static constexpr float fall_start_vel = -0.8;
uint8 Player::jump_frame () {
    if (vel.y > jump_end_vel) {
        return 0;
    }
    else if (vel.y > fall_start_vel) {
        return 1;
    }
    else {
        float dist = distance(fall_start_y, pos.y);
        return 2 + geo::floor(dist / 16) % 2;
    }
}

void Player::Resident_before_step () {
    expect(anim_timer < 255);
    bool can_move = true;
     // Advance animations
    switch (state) {
        case PS::Attack: {
            expect(anim_phase < 4);
            anim_timer += 1;
            if (anim_timer > attack_sequence[anim_phase]) {
                anim_timer = 0;
                anim_phase += 1;
            }
            if (anim_phase == 0 && anim_timer == 3) {
                data->stab_sfx.play();
            }
            if (anim_phase <= 2) {
                can_move = false;
            }
            else if (anim_phase == 4) {
                set_state(PS::Neutral);
            }
            break;
        }
        case PS::Land: {
            expect(anim_phase < 2);
            anim_timer += 1;
            if (anim_timer > land_sequence[anim_phase]) {
                anim_timer = 0;
                anim_phase += 1;
            }
            if (anim_phase == 0) {
                can_move = false;
            }
            else if (anim_phase == 2) {
                set_state(PS::Neutral);
            }
            break;
        }
        default: break;
    }
     // Now decide what to do and do it
    Controls controls = current_game->settings().get_controls();

    float acc = floor ? ground_acc : air_acc;
    float max = floor ? ground_max : air_max;
    float dec = floor
        ? can_move ? ground_dec : coast_dec
        : air_dec;

    if (can_move && controls[Control::Left] && !controls[Control::Right]) {
        if (state != PS::Neutral) {
            set_state(PS::Neutral);
            walk_start_x = pos.x;
        }
        left = true;
        if (vel.x > -max) {
            vel.x -= acc;
            if (vel.x < -max) vel.x = -max;
        }
        if (vel.x >= 0) walk_start_x = pos.x;
    }
    else if (can_move && controls[Control::Right]) {
        if (state != PS::Neutral) {
            set_state(PS::Neutral);
            walk_start_x = pos.x;
        }
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
                set_state(PS::Neutral);
            }
            drop_timer = 0;
        }
        else {
             // Allow drop_timer to count even when on floor
            drop_timer += 1;
            if (drop_timer > drop_duration+1) drop_timer = drop_duration+1;
        }
        if (controls[Control::Attack]) {
            set_state(PS::Attack);
        }
    }

    if (vel.y > fall_start_vel || !defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    vel.y -= drop_timer == 0 ? gravity_jump
           : drop_timer <= drop_duration ? gravity_drop
           : gravity_fall;

    auto walk_frame_before = walk_frame();
    pos += vel;
    if (floor && state == PS::Neutral) {
        auto walk_frame_after = walk_frame();
        if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
            auto i = std::uniform_int_distribution(0, 4)(current_state->rng);
            expect(i >= 0 && i <= 4);
            data->step_sfx[i].play();
        }
    }
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
                 // You can cheat a little and cancel the landing animation with
                 // the end of the attack animation.
                if (state == PS::Neutral) set_state(PS::Land);
                data->land_sfx.play();
                walk_start_x = pos.x;
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
    floor = new_floor;
    new_floor = null;
}

void Player::Resident_draw () {
    auto& poses = data->poses;
    Pose pose;
    switch (state) {
        case PS::Neutral: {
            if (floor) {
                if (distance(walk_start_x, pos.x) >= 1) {
                    pose = poses.walk[walk_frame()];
                }
                else pose = poses.stand;
            }
            else {
                pose = poses.jump[jump_frame()];
            }
            break;
        }
        case PS::Attack: {
            expect(anim_phase < 4);
            pose = poses.attack[anim_phase];
             // If falling, use head from non-attacking poses
            if (!floor) {
                pose.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case PS::Land: {
            pose = poses.land;
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
        attr("fall1", &BodyFrames::fall1),
        attr("land", &BodyFrames::land)
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
        attr("land", &Poses::land)
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
