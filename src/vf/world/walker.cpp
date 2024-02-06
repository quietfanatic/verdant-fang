#include "walker.h"

#include "../game/settings.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../game/state.h"
#include "block.h"

namespace vf {

Walker::Walker () {
    layers_1 |= Layers::Walker_Block;
    layers_2 |= Layers::Weapon_Walker;
}

Walker::Weapon::Weapon () {
    layers_1 |= Layers::Weapon_Block;
    layers_1 |= Layers::Weapon_Walker;
}

void Walker::set_state (WalkerState st) {
    state = st;
    anim_phase = 0;
    anim_timer = 0;
}

 // Animation
uint8 Walker::walk_frame () {
    if (!defined(walk_start_x)) {
         // Was the player's initial state moving?
        walk_start_x = pos.x;
    }
    float dist = distance(walk_start_x, pos.x);
    return geo::floor(dist / data->phys.walk_cycle_dist) % 6;
}
uint8 Walker::jump_frame () {
    if (!defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    if (vel.y > data->phys.jump_end_vel) {
        return 0;
    }
    else if (vel.y > data->phys.fall_start_vel) {
        return 1;
    }
    else {
        float dist = distance(fall_start_y, pos.y);
        return 2 + geo::floor(dist / data->phys.fall_cycle_dist) % 2;
    }
}

void Walker::Resident_before_step () {
    auto& phys = data->phys;

     // Do nothing if we're frozen
    if (freeze_frames) {
        freeze_frames -= 1;
         // Kinda dumb way to disable collision but whatever
        bounds = GNAN;
        return;
    }
    else {
        bounds = phys.bounds;
    }

     // Read controls.
    Controls controls;
    if (mind) controls = mind->Mind_think(*this);

     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    bool occupied = false;
    bool interruptable = false;
    bool do_attack = false;
    switch (state) {
        case WS::Neutral: break;
        case WS::Crouch: break;
        case WS::Land: {
            expect(anim_phase < 2);
            anim_timer += 1;
            if (anim_timer > phys.land_sequence[anim_phase]) {
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
            if (anim_timer > phys.attack_sequence[anim_phase]) {
                anim_timer = 0;
                anim_phase += 1;
            }
            switch (anim_phase) {
                case 0: {
                    if (anim_timer == phys.attack_sequence[0]) {
                         // Delay attack if the button is being held.
                         // We're not really supposed to read the controls yet
                         // but we need to do it before playing the sound
                         // effect.
                        if (controls[Control::Attack]) {
                            anim_timer -= 1;
                        }
                        else {
                            data->sfx.attack->play();
                        }
                    }
                    occupied = true; break;
                }
                case 1: {
                    if (anim_timer == 1) do_attack = true;
                    occupied = true; break;
                }
                case 2: occupied = true; break;
                case 3: interruptable = true; break;
                case 4: set_state(WS::Neutral); break;
            }
            break;
        }
        case WS::Dead: {
            expect(anim_phase < 3);
            if (anim_phase < 2) {
                anim_timer += 1;
                if (anim_timer > phys.dead_sequence[anim_phase]) {
                    anim_phase += 1;
                    anim_timer = 0;
                }
            }
            occupied = true; break;
        }
        default: never();
    }

     // Choose some physics parameters
    float acc = floor ? phys.ground_acc : phys.air_acc;
    float max = floor ? phys.ground_max : phys.air_max;
    float dec = floor
        ? occupied || state == WS::Crouch
            ? phys.coast_dec : phys.ground_dec
        : phys.air_dec;

     // Try to move
    bool decelerate = false;
    if (!occupied) {
         // Crouch or don't
        if (controls[Control::Down]) {
            if (state != WS::Crouch) {
                set_state(WS::Crouch);
                if (!floor) pos.y += phys.jump_crouch_lift;
            }
        }
        else if (state == WS::Crouch) {
            set_state(WS::Neutral);
            walk_start_x = pos.x;
            if (!floor) pos.y -= phys.jump_crouch_lift;
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
            if (floor && !controls[Control::Down] &&
                controls[Control::Jump] <= phys.hold_buffer
            ) {
                vel.y += phys.jump_vel;
                floor = null;
                if (interruptable) set_state(WS::Neutral);
            }
            drop_timer = 0;
        }
        else {
             // Allow drop_timer to count even when on floor
            if (drop_timer < phys.drop_duration) {
                drop_timer += 1;
            }
        }
         // Attack or don't
        if (controls[Control::Attack] &&
            controls[Control::Attack] <= phys.hold_buffer) {
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
    if (vel.y > phys.fall_start_vel || !defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    vel.y -= drop_timer == 0 ? phys.gravity_jump
           : drop_timer <= phys.drop_duration ? phys.gravity_drop
           : phys.gravity_fall;

    auto walk_frame_before = walk_frame();

     // Apply velocity
    pos += vel;

     // See if we need to play footstep sound
    if (floor && state == WS::Neutral) {
        auto walk_frame_after = walk_frame();
        if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
            auto i = std::uniform_int_distribution(0, 4)(current_state->rng);
            expect(i >= 0 && i <= 4);
            data->sfx.step[i]->play();
        }
    }

     // Set up attack hitbox.
    if (do_attack) {
        weapon.set_room(room);
        weapon.bounds = phys.weapon_bounds;
        Vec offset = data->body.attack[1].weapon;
        if (left) {
            offset.x = -offset.x;
            weapon.bounds *= Vec(-1, 1);
        }
        weapon.pos = pos + offset;
    }
    new_floor = null;
}

void Walker::Resident_collide (Resident& other) {
    if (other.layers_2 & Layers::Walker_Block) {
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
                    data->sfx.land->play();
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
}

void Walker::Weapon::Resident_collide (Resident& other) {
     // Horrible offset jankery just to avoid storing a pointer
    usize offset = (char*)&((Walker*)this)->weapon - (char*)this;
    auto wielder = (Walker*)((char*)this - offset);
    wielder->Walker_on_weapon_collide(other);
}

void Walker::Walker_on_weapon_collide (Resident& other) {
    if (other.layers_2 & Layers::Weapon_Block) {
        data->sfx.hit_solid->play();
        if (left) {
            vel.x += 1;
            if (vel.x < 0.5) vel.x = 0.5;
        }
        else {
            vel.x -= 1;
            if (vel.x > -0.5) vel.x = -0.5;
        }
    }
    if (other.layers_2 & Layers::Weapon_Walker) {
        auto& victim = static_cast<Walker&>(other);
        if (victim.state != WS::Dead) {
            freeze_frames = 20;
            victim.freeze_frames = 20;
            victim.set_state(WS::Dead);
            data->sfx.hit_soft->play();
        }
    }
}

void Walker::Resident_after_step () {
    if (freeze_frames) return;
    weapon.set_room(null);
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
        case WS::Dead: {
            if (freeze_frames) {
                pose = poses.damage;
            }
            else switch (anim_phase) {
                case 0: pose = poses.dead[0]; break;
                case 1: case 2: pose = poses.dead[1]; break;
                default: never();
            }
            break;
        }
        default: never();
    }
    Vec scale {left ? -1 : 1, 1};
    float z = state == WS::Dead ? 100 : 200;
    if (pose.body) {
        if (pose.head) {
            Vec head_pos = pos + pose.body->head * scale;
            draw_frame(head_pos, *pose.head, data->head_tex, scale, z);
        }
        draw_frame(pos, *pose.body, data->body_tex, scale, z);
    }
}


} using namespace vf;

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
        attr("back", &HeadFrames::back),
        attr("down", &HeadFrames::down)
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
        attr("attack", &Poses::attack),
        attr("damage", &Poses::damage),
        attr("dead", &Poses::dead)
    )
)

AYU_DESCRIBE(vf::WalkerPhys,
    attrs(
        attr("bounds", &WalkerPhys::bounds),
        attr("weapon_bounds", &WalkerPhys::weapon_bounds),
        attr("ground_acc", &WalkerPhys::ground_acc),
        attr("ground_max", &WalkerPhys::ground_max),
        attr("ground_dec", &WalkerPhys::ground_dec),
        attr("coast_dec", &WalkerPhys::coast_dec),
        attr("air_acc", &WalkerPhys::air_acc),
        attr("air_max", &WalkerPhys::air_max),
        attr("air_dec", &WalkerPhys::air_dec),
        attr("jump_vel", &WalkerPhys::jump_vel),
        attr("gravity_jump", &WalkerPhys::gravity_jump),
        attr("gravity_fall", &WalkerPhys::gravity_fall),
        attr("gravity_drop", &WalkerPhys::gravity_drop),
        attr("drop_duration", &WalkerPhys::drop_duration),
        attr("jump_crouch_lift", &WalkerPhys::jump_crouch_lift),
        attr("land_sequence", &WalkerPhys::land_sequence),
        attr("attack_sequence", &WalkerPhys::attack_sequence),
        attr("dead_sequence", &WalkerPhys::dead_sequence),
        attr("hold_buffer", &WalkerPhys::hold_buffer),
        attr("walk_cycle_dist", &WalkerPhys::walk_cycle_dist),
        attr("fall_cycle_dist", &WalkerPhys::fall_cycle_dist),
        attr("jump_end_vel", &WalkerPhys::jump_end_vel),
        attr("fall_start_vel", &WalkerPhys::fall_start_vel)
    )
)

AYU_DESCRIBE(vf::WalkerSfx,
    attrs(
        attr("step", &WalkerSfx::step),
        attr("land", &WalkerSfx::land),
        attr("attack", &WalkerSfx::attack),
        attr("hit_solid", &WalkerSfx::hit_solid),
        attr("hit_soft", &WalkerSfx::hit_soft)
    )
)

AYU_DESCRIBE(vf::WalkerData,
    attrs(
        attr("phys", &WalkerData::phys),
        attr("body_tex", &WalkerData::body_tex),
        attr("head_tex", &WalkerData::head_tex),
        attr("body", &WalkerData::body),
        attr("head", &WalkerData::head),
        attr("poses", &WalkerData::poses),
        attr("sfx", &WalkerData::sfx)
    )
)

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
        attr("freeze_frames", &Walker::freeze_frames, optional),
        attr("walk_start_x", &Walker::walk_start_x, optional),
        attr("fall_start_x", &Walker::walk_start_x, optional),
        attr("floor", &Walker::floor, optional)
    )
)
