#include "walker.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/game.h"
#include "../game/state.h"

namespace vf {

Walker::Walker () {
    types |= Types::Walker;
    hbs[0].layers_1 = Layers::Walker_Solid | Layers::Walker_Walker;
    hbs[0].layers_2 = Layers::Walker_Walker;
    hbs[1].layers_2 = Layers::Weapon_Walker;
    hbs[2].layers_1 = Layers::Weapon_Solid | Layers::Weapon_Walker;
    hbs[2].layers_2 = Layers::Switch_Weapon;
    hitboxes = Slice<Hitbox>(&hbs[0], 3);
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

void Walker::recoil () {
    if (left) {
        vel.x += 1;
        if (vel.x < 0.5) vel.x = 0.5;
    }
    else {
        vel.x -= 1;
        if (vel.x > -0.5) vel.x = -0.5;
    }
}

void Walker::Resident_set_pos (Vec p) {
    Resident::Resident_set_pos(p);
    walk_start_x = p.x;
}

WalkerBusiness Walker::Walker_business () {
    auto& phys = data->phys;
    switch (state) {
        case WS::Neutral: return WB::Free;
        case WS::Land: {
            expect(anim_phase < 2);
            if (anim_timer >= phys.land_sequence[anim_phase]) {
                if (anim_phase == 1) {
                    set_state(WS::Neutral);
                }
                else {
                    anim_timer = 0;
                    anim_phase += 1;
                }
                return Walker_business();
            }
            else {
                anim_timer += 1;
                switch (anim_phase) {
                    case 0: return WB::Occupied;
                    case 1: return WB::Interruptible;
                    default: never();
                }
            }
        }
        case WS::Attack: {
            expect(anim_phase < 6);
            if (anim_timer >= phys.attack_sequence[anim_phase]) {
                if (anim_phase == 0) return WB::HoldAttack;
                if (anim_phase == 6) {
                    set_state(WS::Neutral);
                }
                else {
                    anim_timer = 0;
                    anim_phase += 1;
                }
                return Walker_business();
            }
            else {
                anim_timer += 1;
                switch (anim_phase) {
                    case 0: case 1: return WB::Occupied;
                    case 2: return WB::DoAttack;
                    case 3: case 4: return WB::Occupied;
                    case 5: return WB::Interruptible;
                    default: never();
                }
            }
        }
        case WS::Hit: {
            expect(anim_phase == 0);
            if (anim_timer >= phys.hit_sequence) {
                 // TODO: move this to Verdant
                if (data->sfx.unhit) data->sfx.unhit->play();
                state = WS::Attack;
                anim_phase = 4;
                anim_timer = 0;
                return Walker_business();
            }
            else {
                anim_timer += 1;
                return WB::Frozen;
            }
        }
        case WS::Dead: {
            expect(anim_phase < 11);
            if (anim_phase == 10) {
                 // End of animation (double meaning)
                return WB::Occupied;
            }
            else if (anim_timer >= phys.dead_sequence[anim_phase]) {
                if (anim_phase == 3 && !floor) {
                     // Hold phase 3 until we land
                    return WB::Occupied;
                }
                else {
                    anim_phase += 1;
                    anim_timer = 0;
                    if (anim_phase == 3) pos.y += phys.dead_floor_lift;
                    return Walker_business();
                }
            }
            else {
                anim_timer += 1;
                switch (anim_phase) {
                    case 0: case 1: case 2: return WB::Frozen;
                    default: return WB::Occupied;
                }
            }
        }
        default: never();
    }
}

void Walker::Resident_before_step () {
    auto& phys = data->phys;

     // Read controls.
    Controls controls;
    if (mind) controls = mind->Mind_think(*this);

     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    expect(state != WS::Hit || anim_phase == 0);
    business = Walker_business();
    expect(state != WS::Hit || anim_phase == 0);

     // Choose some physics parameters
    float acc = floor ? phys.ground_acc : phys.air_acc;
    float max = floor ? phys.ground_max : phys.air_max;
    float dec = floor
        ? business == WB::Occupied || crouch
            ? phys.coast_dec : phys.ground_dec
        : phys.air_dec;

     // Try to move
    bool decelerate = false;
    redo_move:;
    switch (business) {
    case WB::Frozen: vel = {0, 0}; break;
    case WB::HoldAttack: {
        expect(state == WS::Attack);
        if (!controls[Control::Attack]) {
            anim_phase = 1;
            anim_timer = 0;
            business = Walker_business();
            goto redo_move;
        }
        decelerate = true; break;
    }
    case WB::DoAttack:
    case WB::Occupied: decelerate = true; break;
    case WB::Interruptible:
    case WB::Free: {
         // Crouch or don't
        if (controls[Control::Down]) {
            if (!crouch) {
                crouch = true;
                if (business == WB::Interruptible) {
                    set_state(WS::Neutral);
                }
                if (!floor) pos.y += phys.jump_crouch_lift;
            }
        }
        else if (crouch) {
            crouch = false;
            if (!floor) pos.y -= phys.jump_crouch_lift;
            walk_start_x = pos.x;
        }

         // Walk or don't
        if (crouch && floor) decelerate = true;
        else if (controls[Control::Left] && !controls[Control::Right]) {
            if (business == WB::Interruptible) {
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
            if (business == WB::Interruptible) {
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
                if (business == WB::Interruptible) set_state(WS::Neutral);
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

    if (business != WB::Frozen) {
         // Fall
        if (vel.y > phys.fall_start_vel || !defined(fall_start_y)) {
            fall_start_y = pos.y;
        }
         // TODO: Differentiate between dead anim phases?
        vel.y -= state == WS::Dead ? phys.gravity_damage
               : drop_timer == 0 ? phys.gravity_jump
               : drop_timer <= phys.drop_duration ? phys.gravity_drop
               : phys.gravity_fall;

         // Apply velocity and see if we need to play footstep sound
        auto walk_frame_before = walk_frame();
        pos += vel;
        if (floor && state == WS::Neutral) {
            auto walk_frame_after = walk_frame();
            if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
                auto i = std::uniform_int_distribution(0, 4)(current_game->state().rng);
                expect(i >= 0 && i <= 4);
                data->sfx.step[i]->play();
            }
        }
    }

     // Set up hitboxes
    hbs[0].box = phys.body_box;
    if (left) hbs[0].fliph();
    if (state != WS::Dead && business != WB::Frozen) {
        hbs[1].box = phys.damage_box;
        if (left) hbs[1].fliph();
    }
    else hbs[1].box = GNAN;
    if (business == WB::DoAttack) {
        hbs[2].box = data->poses->attack[anim_phase].body->weapon
                   + data->poses->attack[anim_phase].weapon->hitbox;
        if (left) hbs[2].fliph();
        data->sfx.attack->play();
    }
    else hbs[2].box = GNAN;

     // Prepare for collision detection
    new_floor = null;
    hit_sound = null;
}

void Walker::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Solid) {
        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
         // Bias toward contacting from above.
        if (height(overlap) - 2 <= width(overlap) && overlap.b == here.b) {
            pos.y += height(overlap);
            if (vel.y < 0) {
                vel.y = 0;
                 // Land on block
                if (!floor && !new_floor) {
                     // Cancel attack endlag with landing lag.  I don't like
                     // this, but the alternative is cancelling landing lag with
                     // attack endlag, which sounds nice but monsters end up
                     // doing it all the time.
                    if (state == WS::Neutral ||
                        (state == WS::Attack && anim_phase >= 2)
                    ) set_state(WS::Land);
                    if (state != WS::Dead) {
                        data->sfx.land->play();
                    }
                    walk_start_x = pos.x;
                }
                new_floor = &o;
            }
        }
         // The bias toward contacting the side so we don't hit our head upwards
         // on flat wall blocks.
        else if (width(overlap) - 1 < height(overlap)) {
            if (overlap.l == here.l) {
                pos.x += width(overlap);
                walk_start_x = pos.x;
                if (vel.x < 0) vel.x = 0;
            }
            else if (overlap.r == here.r) {
                pos.x -= width(overlap);
                walk_start_x = pos.x;
                if (vel.x > 0) vel.x = 0;
            }
            else never();
        }
        else if (overlap.t == here.t) {
            pos.y -= height(overlap);
            if (vel.y > 0) vel.y = 0;
        }
        else never();
    }
    else if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Walker) {
        auto& other = static_cast<Walker&>(o);
        if (state != WS::Dead && other.state != WS::Dead &&
            !!floor == !!other.floor
        ) {
            float diff = pos.x < o.pos.x ? 1 : -1;
            pos.x -= diff;
            o.pos.x += diff;
        }
    }
    else if (&hb == &hbs[2] && o_hb.layers_2 & Layers::Weapon_Solid) {
        if (!hit_sound) hit_sound = data->sfx.hit_solid;
        recoil();
    }
    else if (&hb == &hbs[2] && o_hb.layers_2 & Layers::Weapon_Walker) {
        Walker_on_hit(hb, static_cast<Walker&>(o), o_hb);
    }
}

void Walker::Walker_on_hit (const Hitbox&, Walker& victim, const Hitbox&) {
     // Prevent undefined behavior (trying to draw decals when state is WS::Hit)
     // if two walkers hit each other at the same time.
    if (state != WS::Dead) set_state(WS::Hit);
    victim.set_state(WS::Dead);
    victim.data->sfx.attack->stop();
    data->sfx.hit_soft->play();
}

void Walker::Resident_after_step () {
    if (business != WB::Frozen) {
        floor = new_floor;
        new_floor = null;
    }
    if (hit_sound) {
        hit_sound->play();
        hit_sound = null;
    }
}

Pose Walker::Walker_pose () {
    Pose r;
    auto& poses = *data->poses;
    switch (state) {
        case WS::Neutral: {
            if (crouch) {
                r = poses.crouch;
                if (!floor) {
                    r.head = poses.jump[jump_frame()].head;
                }
            }
            else if (floor) {
                if (distance(walk_start_x, pos.x) >= 1) {
                    r = poses.walk[walk_frame()];
                }
                else r = poses.stand;
            }
            else {
                r = poses.jump[jump_frame()];
            }
            break;
        }
        case WS::Land: {
            r = poses.land[anim_phase];
            break;
        }
        case WS::Attack: {
            expect(anim_phase < 6);
            r = poses.attack[anim_phase];
             // If falling, use head from non-attacking poses
            if (!floor) {
                r.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case WS::Hit: {
             // TODO: use dedicated hit pose array
            r = poses.attack[3];
            if (!floor) {
                r.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case WS::Dead: {
            switch (anim_phase) {
                case 0: case 1: case 2: {
                    r = poses.dead[0];
                    r.damage_overlap = true;
                    break;
                }
                case 3: r = poses.dead[1]; break;
                default: r = poses.dead[2]; break;
            }
            r.z = Z::Dead;
            break;
        }
        default: never();
    }
    return r;
}

void Walker::Resident_draw () {
    Pose pose = Walker_pose();
    Vec scale {left ? -1 : 1, 1};
    if (pose.body) {
        Vec head_offset = pose.body->head * scale;
        if (pose.head) {
            draw_frame(
                *pose.head, 0, pos + head_offset, pose.z + Z::HeadOffset, scale
            );
        }
        draw_frame(*pose.body, 0, pos, pose.z, scale);
        if (pose.damage_overlap && decal_index < max_decals) {
            float cutoff = pose.body->decals[decal_index].x;
            Frame overlap = *pose.body;
            if (overlap.bounds.r > cutoff) overlap.bounds.r = cutoff;
            draw_frame(overlap, 0, pos, Z::Overlap, scale);
            if (pose.head) {
                overlap = *pose.head;
                 // Make sure to cancel the addition of head_offset to pos
                float head_cutoff = cutoff - head_offset.x;
                if (overlap.bounds.r > head_cutoff) {
                    overlap.bounds.r = head_cutoff;
                }
                draw_frame(
                    overlap, 0,
                    pos + head_offset, Z::Overlap + Z::HeadOffset, scale
                );
            }
        }
        if (pose.weapon) {
            expect(weapon_state < 3);
            Vec weapon_offset = pose.body->weapon * scale;
            draw_frame(
                *pose.weapon, weapon_state,
                pos + weapon_offset,
                (pose.damage_overlap ? Z::Overlap : pose.z) + Z::WeaponOffset,
                scale
            );
        }
    }
    draw_decal(*this, pose);
}

void Walker::Resident_on_exit () {
    weapon_state = 0;
}

} using namespace vf;

AYU_DESCRIBE(vf::BodyFrame,
    elems(
        elem(&BodyFrame::target),
        elem(&BodyFrame::offset),
        elem(&BodyFrame::head),
        elem(&BodyFrame::weapon, optional),
        elem(&BodyFrame::decals, optional),
        elem(&BodyFrame::decal_dirs, optional)
    ),
    init([](BodyFrame& v){ v.init(); })
)

AYU_DESCRIBE(vf::WeaponFrame,
    elems(
        elem(&WeaponFrame::target),
        elem(&WeaponFrame::offset),
        elem(&WeaponFrame::hitbox, optional)
    ),
    init([](WeaponFrame& v){ v.init(); })
)

AYU_DESCRIBE(vf::Pose,
    elems(
        elem(&Pose::body),
        elem(&Pose::head),
        elem(&Pose::weapon)
    )
)

AYU_DESCRIBE(vf::WalkerPoses,
    attrs(
        attr("stand", &WalkerPoses::stand),
        attr("crouch", &WalkerPoses::crouch),
        attr("walk", &WalkerPoses::walk),
        attr("jump", &WalkerPoses::jump),
        attr("land", &WalkerPoses::land),
        attr("attack", &WalkerPoses::attack),
        attr("dead", &WalkerPoses::dead)
    )
)

AYU_DESCRIBE(vf::WalkerPhys,
    attrs(
        attr("body_box", &WalkerPhys::body_box),
        attr("damage_box", &WalkerPhys::damage_box),
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
        attr("gravity_damage", &WalkerPhys::gravity_damage),
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
        attr("poses", &WalkerData::poses),
        attr("sfx", &WalkerData::sfx),
        attr("decals", &WalkerData::decals)
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
        attr("floor", &Walker::floor, optional),
        attr("walk_start_x", &Walker::walk_start_x, optional),
        attr("fall_start_x", &Walker::walk_start_x, optional),
        attr("decal_type", &Walker::decal_type, optional),
        attr("decal_index", &Walker::decal_index, optional),
        attr("weapon_state", &Walker::weapon_state, optional)
    )
)
