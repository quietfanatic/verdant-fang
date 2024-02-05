#include "walker.h"

#include "../game/settings.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../game/state.h"
#include "block.h"

namespace vf {

Walker::Walker () {
    layers_1 |= Layers::Walker_Block;
}

void Walker::set_state (WalkerState st) {
    state = st;
    anim_phase = 0;
    anim_timer = 0;
}

 // Physics
static constexpr float ground_acc = 0.3;
static constexpr float ground_max = 2.5;
static constexpr float ground_dec = 0.3;
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
static constexpr uint8 land_sequence [2] = {4, 4};

 // Animation
uint8 Walker::walk_frame () {
    if (!defined(walk_start_x)) {
         // Was the player's initial state moving?
        walk_start_x = pos.x;
    }
    float dist = distance(walk_start_x, pos.x);
    return geo::floor(dist / 16) % 6;
}
static constexpr float jump_end_vel = 0.4;
static constexpr float fall_start_vel = -0.8;
uint8 Walker::jump_frame () {
    if (!defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
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

void Walker::Resident_before_step () {
     // Read controls.
    Controls controls;
    if (mind) controls = mind->Mind_think(*this);
     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    bool occupied = false;
    bool interruptable = false;
    switch (state) {
        case WS::Neutral: break;
        case WS::Crouch: break;
        case WS::Land: {
            expect(anim_phase < 2);
            anim_timer += 1;
            if (anim_timer > land_sequence[anim_phase]) {
                anim_timer = 0;
                anim_phase += 1;
            }
            switch (anim_phase) {
                case 0: occupied = true; break;
                case 1: interruptable = true; break;
                case 2: set_state(WS::Neutral); break;
            }
            break;
        }
        case WS::Attack: {
            expect(anim_phase < 4);
            anim_timer += 1;
            if (anim_timer > attack_sequence[anim_phase]) {
                anim_timer = 0;
                anim_phase += 1;
            }
            switch (anim_phase) {
                case 0: {
                    if (anim_timer == attack_sequence[0]) {
                         // Delay attack if the button is being held.
                         // We're not really supposed to read the controls yet
                         // but we need to do it before playing the sound
                         // effect.
                        if (controls[Control::Attack]) {
                            anim_timer -= 1;
                        }
                        else {
                            data->attack_sfx.play();
                        }
                    }
                    occupied = true; break;
                }
                case 1: case 2: occupied = true; break;
                case 3: interruptable = true; break;
                case 4: set_state(WS::Neutral); break;
            }
            break;
        }
        default: never();
    }

     // Choose some physics parameters
    float acc = floor ? ground_acc : air_acc;
    float max = floor ? ground_max : air_max;
    float dec = floor
        ? occupied || state == WS::Crouch ? coast_dec : ground_dec
        : air_dec;

     // Try to move
    bool decelerate = false;
    if (!occupied) {
         // Crouch or don't
        if (controls[Control::Down]) {
            if (state != WS::Crouch) {
                set_state(WS::Crouch);
                 // Lift legs up
                if (!floor) pos.y += 4;
            }
        }
        else if (state == WS::Crouch) {
            set_state(WS::Neutral);
            walk_start_x = pos.x;
             // Put legs down
            if (!floor) pos.y -= 4;
        }

         // Walk or don't
        if (state == WS::Crouch && floor) decelerate = true;
        else if (controls[Control::Left] && !controls[Control::Right]) {
            if (interruptable) {
                set_state(WS::Neutral);
                walk_start_x = pos.x;
            }
            left = true;
            if (floor && vel.x == 0) {
                 // Start moving at least one pixel
                pos.x -= 1;
            }
            if (vel.x > -max) {
                vel.x -= acc;
                if (vel.x < -max) vel.x = -max;
            }
            if (vel.x >= 0) walk_start_x = pos.x;
        }
        else if (controls[Control::Right]) {
            if (interruptable) {
                set_state(WS::Neutral);
                walk_start_x = pos.x;
            }
            left = false;
            if (floor && vel.x == 0) {
                pos.x += 1;
            }
            if (vel.x < max) {
                vel.x += acc;
                if (vel.x > max) vel.x = max;
            }
            if (vel.x <= 0) walk_start_x = pos.x;
        }
        else decelerate = true;

         // Jump or don't
        if (controls[Control::Jump]) {
            if (floor && !controls[Control::Down]) {
                vel.y += jump_vel;
                floor = null;
                if (interruptable) set_state(WS::Neutral);
            }
            drop_timer = 0;
        }
        else {
             // Allow drop_timer to count even when on floor
            drop_timer += 1;
            if (drop_timer > drop_duration+1) drop_timer = drop_duration+1;
        }
         // Attack or don't
        if (controls[Control::Attack]) {
            set_state(WS::Attack);
        }
    }
    else decelerate = true;

    if (decelerate) {
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

     // Fall
    if (vel.y > fall_start_vel || !defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    vel.y -= drop_timer == 0 ? gravity_jump
           : drop_timer <= drop_duration ? gravity_drop
           : gravity_fall;

    auto walk_frame_before = walk_frame();
     // Apply velocity
    pos += vel;

     // See if we need to play footstep sound
    if (floor && state == WS::Neutral) {
        auto walk_frame_after = walk_frame();
        if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
            auto i = std::uniform_int_distribution(0, 4)(current_state->rng);
            expect(i >= 0 && i <= 4);
            data->step_sfx[i].play();
        }
    }
    new_floor = null;
}

void Walker::Resident_collide (Resident& other) {
    expect(other.layers_2 & Layers::Walker_Block);
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
                if (state == WS::Neutral) set_state(WS::Land);
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

void Walker::Resident_after_step () {
    floor = new_floor;
    new_floor = null;
}

void Walker::Resident_draw () {
    auto& poses = data->poses;
    Pose pose;
    switch (state) {
        case WS::Neutral: {
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
        case WS::Crouch: {
            pose = poses.crouch;
            if (!floor) {
                pose.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case WS::Land: {
            pose = poses.land[anim_phase];
            break;
        }
        case WS::Attack: {
            expect(anim_phase < 4);
            pose = poses.attack[anim_phase];
             // If falling, use head from non-attacking poses
            if (!floor) {
                pose.head = poses.jump[jump_frame()].head;
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

AYU_DESCRIBE(vf::WalkerState,
    values(
        value("neutral", WS::Neutral),
        value("crouch", WS::Crouch),
        value("attack", WS::Attack),
        value("land", WS::Land)
    )
)

AYU_DESCRIBE(vf::Walker,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Walker::data),
        attr("mind", &Walker::mind),
        attr("vel", &Walker::vel, optional),
        attr("left", &Walker::left, optional),
        attr("state", &Walker::state, optional),
        attr("anim_phase", &Walker::anim_phase, optional),
        attr("anim_timer", &Walker::anim_timer, optional),
        attr("drop_timer", &Walker::drop_timer, optional),
        attr("walk_start_x", &Walker::walk_start_x, optional),
        attr("fall_start_x", &Walker::walk_start_x, optional),
        attr("floor", &Walker::floor, optional)
    )
)

AYU_DESCRIBE(vf::BodyFrame,
    elems(
        elem(&BodyFrame::offset),
        elem(&BodyFrame::bounds),
        elem(&BodyFrame::head),
        elem(&BodyFrame::weapon, optional)
    )
)

AYU_DESCRIBE(vf::BodyFrames,
    attrs(
        attr("stand", &BodyFrames::stand),
        attr("crouch", &BodyFrames::crouch),
        attr("walk", &BodyFrames::walk),
        attr("fall1", &BodyFrames::fall1),
        attr("land", &BodyFrames::land),
        attr("attack", &BodyFrames::attack)
    )
)

AYU_DESCRIBE(vf::HeadFrames,
    attrs(
        attr("neutral", &HeadFrames::neutral),
        attr("wave", &HeadFrames::wave),
        attr("fall", &HeadFrames::fall),
        attr("back", &HeadFrames::back)
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
        attr("crouch", &Poses::crouch),
        attr("walk", &Poses::walk),
        attr("jump", &Poses::jump),
        attr("land", &Poses::land),
        attr("attack", &Poses::attack)
    )
)

AYU_DESCRIBE(vf::WalkerData,
    attrs(
        attr("img", &WalkerData::img),
        attr("body_tex", &WalkerData::body_tex),
        attr("head_tex", &WalkerData::head_tex),
        attr("bodies", &WalkerData::bodies),
        attr("heads", &WalkerData::heads),
        attr("poses", &WalkerData::poses),
        attr("step_sfx", &WalkerData::step_sfx),
        attr("land_sfx", &WalkerData::land_sfx),
        attr("attack_sfx", &WalkerData::attack_sfx),
        attr("hit_solid_sfx", &WalkerData::hit_solid_sfx)
    )
)