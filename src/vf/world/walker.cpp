#include "walker.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/game.h"
#include "../game/state.h"

namespace vf {

Walker::Walker () {
    types |= Types::Walker;
    hbs[0].layers_1 = Layers::Walker_Solid | Layers::Walker_Walker
                    | Layers::Walker_Semisolid;
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
    return geo::floor(dist / data->walk_cycle_dist) % 6;
}
uint8 Walker::jump_frame () {
    if (!defined(fall_start_y)) {
        fall_start_y = pos.y;
    }
    if (vel.y > data->jump_end_vel) {
        return 0;
    }
    else if (vel.y > data->fall_start_vel) {
        return 1;
    }
    else {
        float dist = distance(fall_start_y, pos.y);
        return 2 + geo::floor(dist / data->fall_cycle_dist) % 2;
    }
}

void Walker::Resident_set_pos (Vec p) {
    Resident::Resident_set_pos(p);
    walk_start_x = p.x;
}

WalkerBusiness Walker::Walker_business () {
    switch (state) {
        case WS::Neutral: return WB::Free;
        case WS::Land: {
            expect(anim_phase < 2);
            if (anim_timer >= data->land_sequence[anim_phase]) {
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
            if (anim_timer >= data->attack_sequence[anim_phase]) {
                if (anim_phase == 0) return WB::HoldAttack;
                if (anim_phase == 5) {
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
            expect(anim_phase < 2);
            if (anim_timer >= data->hit_sequence[anim_phase]) {
                if (anim_phase == 1) {
                    state = WS::Attack;
                    anim_phase = 4;
                    anim_timer = 0;
                }
                else {
                    anim_phase += 1;
                    anim_timer = 0;
                }
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
            else if (anim_timer >= data->dead_sequence[anim_phase]) {
                if (anim_phase == 3 && !floor) {
                     // Hold phase 3 until we land
                    return WB::Occupied;
                }
                else {
                    anim_phase += 1;
                    anim_timer = 0;
                    if (anim_phase == 3) pos.y += data->dead_floor_lift;
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
    z_override = GNAN;
     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    business = Walker_business();

     // Read controls.
    Controls controls;
    if (mind) controls = mind->Mind_think(*this);

     // Choose some physics parameters
    float acc = floor ? data->ground_acc : data->air_acc;
    float max = floor ? data->ground_max : data->air_max;
    float dec = floor
        ? business == WB::Occupied || crouch
            ? data->coast_dec : data->ground_dec
        : data->air_dec;

     // Try to move
    bool walking = false;
    bool decelerate = false;
    restart_move:
    switch (business) {
    case WB::Frozen: vel = {0, 0}; break;
    case WB::HoldAttack: {
        expect(state == WS::Attack);
        if (!controls[Control::Attack]) {
            anim_phase = 1;
            anim_timer = 0;
            business = Walker_business();
            goto restart_move;
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
                if (!floor) pos.y += data->jump_crouch_lift;
            }
        }
        else if (crouch) {
            crouch = false;
            if (!floor) pos.y -= data->jump_crouch_lift;
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
            walking = !!floor;
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
            walking = !!floor;
        }
        else decelerate = true;

         // Jump or don't
        if (controls[Control::Jump]) {
            if (floor && !controls[Control::Down] &&
                controls[Control::Jump] <= data->hold_buffer
            ) {
                vel.y += data->jump_vel;
                floor = null;
                if (business == WB::Interruptible) set_state(WS::Neutral);
            }
            drop_timer = 0;
        }
        else {
             // Allow drop_timer to count even when on floor
            if (drop_timer < data->drop_duration) {
                drop_timer += 1;
            }
        }
         // Attack or don't
        if (controls[Control::Attack] &&
            controls[Control::Attack] <= data->hold_buffer) {
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
        if (vel.y > data->fall_start_vel || !defined(fall_start_y)) {
            fall_start_y = pos.y;
        }
         // TODO: Differentiate between dead anim phases?
        vel.y -= state == WS::Dead ? data->gravity_damage
               : drop_timer == 0 ? data->gravity_jump
               : drop_timer <= data->drop_duration ? data->gravity_drop
               : data->gravity_fall;

         // Apply velocity and see if we need to play footstep sound
        auto walk_frame_before = walk_frame();
        pos += vel;
        if (walking) {
            auto walk_frame_after = walk_frame();
            if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
                auto i = std::uniform_int_distribution(0, 4)(current_game->state().rng);
                expect(i >= 0 && i <= 4);
                data->step_sound[i]->play();
            }
        }
    }

     // Set up hitboxes
    hbs[0].box = data->body_box;
    if (left) hbs[0].fliph();
    if (state != WS::Dead && business != WB::Frozen) {
        hbs[1].box = data->damage_box;
        if (left) hbs[1].fliph();
    }
    else hbs[1].box = GNAN;
    if (business == WB::DoAttack) {
        if (crouch) {
            hbs[2].box = data->poses->crouch_attack[anim_phase].body->weapon
                       + data->poses->crouch_attack[anim_phase].weapon->hitbox;
            if (left) hbs[2].fliph();
            data->crouch_attack_sound->play();
        }
        else {
            hbs[2].box = data->poses->attack[anim_phase].body->weapon
                       + data->poses->attack[anim_phase].weapon->hitbox;
            if (left) hbs[2].fliph();
            data->attack_sound->play();
        }
    }
    else hbs[2].box = GNAN;

     // Prepare for collision detection
    new_floor = null;
    hit_sound = null;
}

void Walker::set_floor (Resident& o) {
    if (!floor && !new_floor) {
         // Cancel attack endlag with landing lag...but only if we
         // wouldn't get any extra control frames.  TODO: clean this
         // up.
        if (state == WS::Neutral ||
            (state == WS::Attack && (
                (anim_phase == 4 &&
                    anim_timer >= uint8(data->attack_sequence[anim_phase]
                                      - data->land_sequence[0])
                ) || anim_phase == 5
            ))
        ) set_state(WS::Land);
        if (state != WS::Dead) {
            data->land_sound->play();
        }
        walk_start_x = pos.x;
    }
    new_floor = &o;
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
             // Note, if solid has velocity, this will not take it into account
            if (vel.y < 0) {
                vel.y = 0;
                set_floor(o);
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
    else if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Semisolid) {
        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
        if (vel.y < 0 && overlap.b == here.b && height(overlap) <= 2 - vel.y) {
            pos.y += height(overlap);
            vel.y = 0;
            set_floor(o);
        }
    }
    else if (&hb == &hbs[0] && o_hb.layers_2 & Layers::Walker_Walker) {
        auto& other = static_cast<Walker&>(o);
        if (state != WS::Dead && other.state != WS::Dead &&
            !!floor == !!other.floor &&
            !defined(z_override) && !defined(other.z_override)
        ) {
            float diff = pos.x < o.pos.x ? 1 : -1;
            pos.x -= diff;
            o.pos.x += diff;
        }
    }
    else if (&hb == &hbs[2] && o_hb.layers_2 & Layers::Weapon_Solid) {
        if (!hit_sound) hit_sound = data->hit_solid_sound;
        do_recoil = true;
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
    victim.data->attack_sound->stop();
    data->hit_soft_sound->play();
}

void Walker::Resident_after_step () {
    if (business != WB::Frozen) {
        floor = new_floor;
        new_floor = null;
    }
    if (do_recoil) {
        if (left) {
            vel.x += 1;
            if (vel.x < 0.5) vel.x = 0.5;
        }
        else {
            vel.x -= 1;
            if (vel.x > -0.5) vel.x = -0.5;
        }
        do_recoil = false;
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
            if (crouch) {
                r = poses.crouch_attack[anim_phase];
                if (!floor) {
                    r.head = poses.jump[jump_frame()].head;
                }
            }
            else {
                r = poses.attack[anim_phase];
                 // If falling, use head from non-attacking poses
                if (!floor) {
                    r.head = poses.jump[jump_frame()].head;
                }
            }

            break;
        }
        case WS::Hit: {
            r = poses.hit[anim_phase];
            if (!floor) {
                r.head = poses.jump[jump_frame()].head;
            }
            break;
        }
        case WS::Dead: {
            switch (anim_phase) {
                case 0: case 1: case 2: r = poses.dead[0]; break;
                case 3: r = poses.dead[1]; break;
                default: r = poses.dead[2]; break;
            }
            break;
        }
        default: never();
    }
    return r;
}

void Walker::Resident_draw () {
    Pose pose = Walker_pose();
    if (defined(z_override)) pose.z = z_override;
    Vec scale {left ? -1 : 1, 1};
    if (pose.body) {
        Vec head_offset = pose.body->head * scale;
        if (pose.head) {
            draw_layers(
                *pose.head, head_layers,
                pos + head_offset, pose.z + Z::HeadOffset, scale
            );
        }
        else ayu::dump(0);
        draw_layers(
            *pose.body, body_layers & pose.body_layers,
            pos, pose.z, scale
        );
        if (!defined(z_override) && pose.damage_overlap && decal_index < max_decals) {
            float cutoff = pose.body->decals[decal_index].x;
            Frame overlap = *pose.body;
            if (overlap.bounds.r > cutoff) overlap.bounds.r = cutoff;
            draw_layers(
                overlap, body_layers & pose.body_layers,
                pos, Z::Overlap, scale
            );
            if (pose.head) {
                overlap = *pose.head;
                 // Make sure to cancel the addition of head_offset to pos
                float head_cutoff = cutoff - head_offset.x;
                if (overlap.bounds.r > head_cutoff) {
                    overlap.bounds.r = head_cutoff;
                }
                draw_layers(
                    overlap, head_layers,
                    pos + head_offset, Z::Overlap + Z::HeadOffset, scale
                );
            }
        }
        Walker_draw_weapon(pose);
    }
    draw_decal(*this, pose);
}

void Walker::Walker_draw_weapon (const Pose& pose) {
    Vec scale = {left ? -1 : 1, 1};
    if (pose.weapon) {
        draw_layers(
            *pose.weapon, weapon_layers,
            pos + pose.body->weapon * scale,
            (pose.damage_overlap ? Z::Overlap : pose.z) + Z::WeaponOffset,
            scale
        );
    }
}

void Walker::Resident_on_exit () {
    weapon_layers = 0x1;
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
        elem(&Pose::head, optional),
        elem(&Pose::weapon, optional),
        elem(&Pose::z, optional),
        elem(&Pose::damage_overlap, optional),
        elem(member(&Pose::body_layers, prefer_hex), optional)
    ),
    attrs(
        attr("body", &Pose::body, optional),
        attr("head", &Pose::head, optional),
        attr("weapon", &Pose::weapon, optional),
        attr("z", &Pose::z, optional),
        attr("damage_overlap", &Pose::damage_overlap, optional),
        attr("body_layers", member(&Pose::body_layers, prefer_hex), optional)
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
        attr("crouch_attack", &WalkerPoses::crouch_attack),
        attr("hit", &WalkerPoses::hit),
        attr("dead", &WalkerPoses::dead)
    )
)

AYU_DESCRIBE(vf::WalkerData,
    attrs(
        attr("body_box", &WalkerData::body_box),
        attr("damage_box", &WalkerData::damage_box),
        attr("ground_acc", &WalkerData::ground_acc),
        attr("ground_max", &WalkerData::ground_max),
        attr("ground_dec", &WalkerData::ground_dec),
        attr("coast_dec", &WalkerData::coast_dec),
        attr("air_acc", &WalkerData::air_acc),
        attr("air_max", &WalkerData::air_max),
        attr("air_dec", &WalkerData::air_dec),
        attr("jump_vel", &WalkerData::jump_vel),
        attr("gravity_jump", &WalkerData::gravity_jump),
        attr("gravity_fall", &WalkerData::gravity_fall),
        attr("gravity_drop", &WalkerData::gravity_drop),
        attr("gravity_damage", &WalkerData::gravity_damage),
        attr("drop_duration", &WalkerData::drop_duration),
        attr("land_sequence", &WalkerData::land_sequence),
        attr("attack_sequence", &WalkerData::attack_sequence),
        attr("hit_sequence", &WalkerData::hit_sequence),
        attr("dead_sequence", &WalkerData::dead_sequence),
        attr("jump_crouch_lift", &WalkerData::jump_crouch_lift),
        attr("dead_floor_lift", &WalkerData::dead_floor_lift),
        attr("hold_buffer", &WalkerData::hold_buffer),
        attr("walk_cycle_dist", &WalkerData::walk_cycle_dist),
        attr("fall_cycle_dist", &WalkerData::fall_cycle_dist),
        attr("jump_end_vel", &WalkerData::jump_end_vel),
        attr("fall_start_vel", &WalkerData::fall_start_vel),
        attr("step_sound", &WalkerData::step_sound),
        attr("land_sound", &WalkerData::land_sound),
        attr("attack_sound", &WalkerData::attack_sound),
        attr("crouch_attack_sound", &WalkerData::crouch_attack_sound),
        attr("hit_solid_sound", &WalkerData::hit_solid_sound),
        attr("hit_soft_sound", &WalkerData::hit_soft_sound),
        attr("poses", &WalkerData::poses),
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
        attr("body_layers", &Walker::body_layers, optional),
        attr("head_layers", &Walker::head_layers, optional),
        attr("weapon_layers", &Walker::weapon_layers, optional)
    )
)
