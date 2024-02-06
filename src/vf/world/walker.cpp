#include "walker.h"

#include "../game/settings.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../game/state.h"
#include "block.h"

namespace vf {

Walker::Walker () {
    types |= Types::Walker;
    hbs[0].layers_1 = Layers::Walker_Block | Layers::Walker_Walker;
    hbs[0].layers_2 = Layers::Weapon_Walker | Layers::Walker_Walker;
    hbs[1].layers_1 = Layers::Weapon_Block | Layers::Weapon_Walker;
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

     // Read controls.
    Controls controls;
    if (mind) controls = mind->Mind_think(*this);

     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    business = B::Free;
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
                case 0: business = B::Occupied; break;
                case 1: business = B::Interruptible; break;
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
                    business = B::Occupied; break;
                }
                case 1: {
                    if (anim_timer == 1) do_attack = true;
                    business = B::Occupied; break;
                }
                case 2: business = B::Occupied; break;
                case 3: business = B::Interruptible; break;
                case 4: set_state(WS::Neutral); break;
            }
            break;
        }
        case WS::Hit: {
            expect(anim_phase == 0);
            anim_timer += 1;
            if (anim_timer > phys.hit_sequence) {
                if (data->sfx.unhit) data->sfx.unhit->play();
                state = WS::Attack;
                anim_phase = 2;
                anim_timer = 0;
            }
            business = B::Frozen;
            break;
        }
        case WS::Dead: {
            expect(anim_phase < 4);
            switch (anim_phase) {
                case 0: {
                    anim_timer += 1;
                    if (anim_timer > phys.dead_sequence[anim_phase]) {
                        anim_phase += 1;
                        anim_timer = 0;
                        if (floor) pos.y += phys.dead_floor_lift;
                        business = B::Occupied; break;
                    }
                    business = B::Frozen; break;
                }
                case 1: {
                    if (floor) {
                        anim_phase = 2;
                        anim_timer = 0;
                    }
                    business = B::Occupied; break;
                }
                case 2: {
                    anim_timer += 1;
                    if (anim_timer > phys.dead_sequence[anim_phase]) {
                        anim_phase += 1;
                        anim_timer = 0;
                    }
                    business = B::Occupied; break;
                }
                case 3: business = B::Occupied; break;
            }
            break;
        }
        default: never();
    }

     // Choose some physics parameters
    float acc = floor ? phys.ground_acc : phys.air_acc;
    float max = floor ? phys.ground_max : phys.air_max;
    float dec = floor
        ? business == B::Occupied || state == WS::Crouch
            ? phys.coast_dec : phys.ground_dec
        : phys.air_dec;

     // Try to move
    bool decelerate = false;
    switch (business) {
    case B::Frozen: break;
    case B::Occupied: decelerate = true; break;
    case B::Interruptible:
    case B::Free: {
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
            if (business == B::Interruptible) {
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
            if (business == B::Interruptible) {
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
                if (business == B::Interruptible) set_state(WS::Neutral);
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
        break;
    }
    }

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

    if (business == B::Frozen) {
        vel = {0, 0};
    }
    else {
         // Fall
        if (vel.y > phys.fall_start_vel || !defined(fall_start_y)) {
            fall_start_y = pos.y;
        }
        vel.y -= state == WS::Dead || drop_timer == 0 ? phys.gravity_jump
               : drop_timer <= phys.drop_duration ? phys.gravity_drop
               : phys.gravity_fall;

         // Apply velocity and see if we need to play footstep sound
        auto walk_frame_before = walk_frame();
        pos += vel;
        if (floor && state == WS::Neutral) {
            auto walk_frame_after = walk_frame();
            if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
                auto i = std::uniform_int_distribution(0, 4)(current_state->rng);
                expect(i >= 0 && i <= 4);
                data->sfx.step[i]->play();
            }
        }
    }

     // Set up hitboxes
    hbs[0].box = phys.body_box;
    if (left) hbs[0].fliph();
    if (do_attack) {
        hbs[1].box = phys.weapon_box + data->body.attack[1].weapon;
        if (left) hbs[1].fliph();
        hitboxes = Slice<Hitbox>(&hbs[0], 2);
    }
    else hitboxes = Slice<Hitbox>(&hbs[0], 1);

     // Prepare for collision detection
    new_floor = null;
}

void Walker::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Block) {
        auto& block = static_cast<Block&>(o);

        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
        if (height(overlap) <= width(overlap)) {
            if (overlap.b == here.b) {
                pos.y += height(overlap);
                if (vel.y < 0) vel.y = 0;
                 // Land on block
                if (!floor && !new_floor) {
                     // You can cheat a little and cancel the landing animation
                     // with the end of the attack animation.
                    if (state == WS::Neutral) set_state(WS::Land);
                    if (state != WS::Dead) {
                        data->sfx.land->play();
                    }
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
    else if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Walker) {
        auto& other = static_cast<Walker&>(o);
        if (state != WS::Dead && other.state != WS::Dead) {
            float diff = pos.x < o.pos.x ? 1 : -1;
            pos.x -= diff;
            o.pos.x += diff;
        }
    }
    else if (&hb == &hbs[1] && o_hb.layers_2 & Layers::Weapon_Block) {
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
    else if (&hb == &hbs[1] && o_hb.layers_2 & Layers::Weapon_Walker) {
        auto& victim = static_cast<Walker&>(o);
        if (victim.state != WS::Hit && victim.state != WS::Dead) {
            set_state(WS::Hit);
            victim.set_state(WS::Dead);
            data->sfx.hit_soft->play();
        }
    }
}

void Walker::Resident_after_step () {
    if (business != B::Frozen) {
        floor = new_floor;
        new_floor = null;
    }
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
        case WS::Hit: {
            pose = poses.attack[1];
            if (!floor) {
                pose.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case WS::Dead: {
            switch (anim_phase) {
                case 0: pose = poses.damage; break;
                case 1: pose = poses.dead[0]; break;
                case 2: case 3: pose = poses.dead[1]; break;
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
        attr("attack", &BodyFrames::attack),
        attr("damage", &BodyFrames::damage),
        attr("dead", &BodyFrames::dead)
    )
)

AYU_DESCRIBE(vf::HeadFrames,
    attrs(
        attr("neutral", &HeadFrames::neutral),
        attr("wave", &HeadFrames::wave),
        attr("fall", &HeadFrames::fall),
        attr("back", &HeadFrames::back),
        attr("down", &HeadFrames::down),
        attr("damage", &HeadFrames::damage),
        attr("dead", &HeadFrames::dead)
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
        attr("body_box", &WalkerPhys::body_box),
        attr("weapon_box", &WalkerPhys::weapon_box),
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
        attr("land_sequence", &WalkerPhys::land_sequence),
        attr("attack_sequence", &WalkerPhys::attack_sequence),
        attr("hit_sequence", &WalkerPhys::hit_sequence),
        attr("dead_sequence", &WalkerPhys::dead_sequence),
        attr("jump_crouch_lift", &WalkerPhys::jump_crouch_lift),
        attr("dead_floor_lift", &WalkerPhys::dead_floor_lift),
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
        attr("hit_soft", &WalkerSfx::hit_soft),
        attr("unhit", &WalkerSfx::unhit, optional)
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
        value("land", WS::Land),
        value("hit", WS::Hit),
        value("dead", WS::Dead)
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
