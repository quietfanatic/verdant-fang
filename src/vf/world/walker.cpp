#include "walker.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/game.h"
#include "../game/state.h"

namespace vf {

Walker::Walker () {
    types |= Types::Walker;
    body_hb.layers_1 = Layers::Walker_Solid | Layers::Walker_Walker
                    | Layers::Walker_Semisolid;
    body_hb.layers_2 = Layers::Walker_Walker;
    damage_hb.layers_2 = Layers::Weapon_Walker | Layers::Projectile_Walker;
    weapon_hb.layers_1 = Layers::Weapon_Solid | Layers::Weapon_Walker;
    weapon_hb.layers_2 = Layers::Switch_Weapon;
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

float Walker::gravity () {
    return state == WS::Stun ? data->gravity_fall
         : state == WS::Dead ? data->gravity_dead
         : fly ? data->gravity_fly
         : drop_timer == 0 ? data->gravity_jump
         : drop_timer <= data->drop_duration ? data->gravity_drop
         : data->gravity_fall;
}

void Walker::Resident_set_pos (Vec p) {
    Resident::Resident_set_pos(p);
    walk_start_x = p.x;
    vel = {0, 0};
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
                    weapon_layers = pending_weapon_layers;
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
        case WS::Stun: {
            fly = false;
            if (anim_timer >= stun_duration) {
                set_state(WS::Neutral);
                return Walker_business();
            }
            else {
                anim_timer += 1;
                return WB::Occupied;
            }
        }
        case WS::Dead: {
            fly = false;
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

void Walker::Walker_move (const Controls& controls) {
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
        if (fly) {
             // You are unable to change direction while flying, so you'll need
             // to change directions in the mind.
            if (controls[Control::Up] && !controls[Control::Down]) {
                if (business == WB::Interruptible) set_state(WS::Neutral);
                if (vel.y < max) {
                    vel.y += acc;
                    if (vel.y > max) vel.y = max;
                }
            }
            else if (controls[Control::Down]) {
                if (business == WB::Interruptible) set_state(WS::Neutral);
                if (vel.y > -max) {
                    vel.y -= acc;
                    if (vel.y < -max) vel.y = -max;
                }
            }
            if (controls[Control::Left] && !controls[Control::Right]) {
                if (business == WB::Interruptible) set_state(WS::Neutral);
                if (vel.x > -max) {
                    vel.x -= acc;
                    if (vel.x < -max) vel.x = -max;
                }
            }
            else if (controls[Control::Right]) {
                if (business == WB::Interruptible) set_state(WS::Neutral);
                if (vel.x < max) {
                    vel.x += acc;
                    if (vel.x > max) vel.x = max;
                }
            }
            else decelerate = true;
        }
        else {
             // Try flying
            if (data->can_fly && controls[Control::Up]) {
                fly = true;
                if (floor) pos.y += data->fly_floor_lift;
                goto restart_move;
            }
             // Crouch or don't
            if (no_crouch_timer) {
                crouch = false;
                no_crouch_timer -= 1;
            }
            else if (controls[Control::Down]) {
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
                if (floor && controls[Control::Jump] <= data->hold_buffer) {
                    if (controls[Control::Down]) {
                         // Drop through semisolid
                        if (floor->types & Types::Semisolid) {
                            pos.y -= min(data->jump_crouch_lift, 4);
                            floor = null;
                            crouch = false;
                            no_crouch_timer = 30;
                        }
                    }
                    else {
                        vel.y += data->jump_vel;
                        floor = null;
                    }
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
        }
         // Attack or don't
        if (controls[Control::Attack] &&
            controls[Control::Attack] <= data->hold_buffer
        ) {
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
        vel.y -= gravity();

         // Apply velocity and see if we need to play footstep sound
        auto walk_frame_before = walk_frame();
        pos += vel;
        if (walking) {
            auto walk_frame_after = walk_frame();
            if (walk_frame_before % 3 == 1 && walk_frame_after % 3 == 2) {
                auto i = std::uniform_int_distribution(0, 4)(current_game->state().rng);
                expect(i >= 0 && i <= 4);
                if (data->step_sound[i]) data->step_sound[i]->play();
            }
        }
    }
}

void Walker::Walker_set_hitboxes () {
    clear_hitboxes();
    body_hb.box = state == WS::Dead ? data->dead_body_box
                : crouch ? data->crouch_body_box
                : data->body_box;
    if (left) body_hb.fliph();
    add_hitbox(body_hb);
    if (state != WS::Dead && business != WB::Frozen) {
        damage_hb.box = crouch ? data->crouch_damage_box : data->damage_box;
        if (left) damage_hb.fliph();
        add_hitbox(damage_hb);
    }
    if (business == WB::DoAttack) {
        if (crouch) {
            weapon_hb.box = data->poses->crouch_attack[anim_phase].body->weapon
                          + data->poses->crouch_attack[anim_phase].weapon->hitbox;
            if (data->crouch_attack_sound) data->crouch_attack_sound->play();
            else if (data->attack_sound) data->attack_sound->play();
        }
        else {
            weapon_hb.box = data->poses->attack[anim_phase].body->weapon
                          + data->poses->attack[anim_phase].weapon->hitbox;
            if (data->attack_sound) data->attack_sound->play();
        }
        if (left) weapon_hb.fliph();
        add_hitbox(weapon_hb);
    }
}

void Walker::Resident_before_step () {
     // Do poison
    if (poison_level && state != WS::Dead) {
        if (poison_timer >= 180) {
            if (poison_level >= 3) {
                set_state(WS::Dead);
                paralyze_symbol_timer = 90;
            }
            else {
                set_state(WS::Stun);
                stun_duration = poison_level * 30;
                paralyze_symbol_timer = stun_duration;
                poison_timer = 0;
            }
            if (data->paralyzed_sound) data->paralyzed_sound->play();
        }
        else poison_timer += 1;
    }
     // Advance animations and do state-dependent things
    expect(anim_timer < 255);
    business = Walker_business();

     // Read controls.
    Controls controls {};
    if (mind) controls = mind->Mind_think(*this);

     // Move
    Walker_move(controls);

     // Set up hitboxes
    Walker_set_hitboxes();

     // Prepare for collision detection
    new_floor = null;
    hit_sound = null;
}

 // This read y velocity so you need to call it before setting vel.y
void Walker::set_floor (Resident& o) {
    if (!floor && !new_floor) {
        if (vel.y < -1) {
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
                if (data->land_sound) data->land_sound->play();
            }
        }
        no_crouch_timer = 0;
        walk_start_x = pos.x;
    }
    new_floor = &o;
}

void Walker::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (&hb == &body_hb && o_hb.layers_2 & Layers::Walker_Solid) {
        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
         // Bias toward contacting from above.
        if (height(overlap) - 2 <= width(overlap) && overlap.b == here.b) {
            pos.y += height(overlap);
             // Note, if solid has velocity, this will not take it into account
            if (vel.y < 0) {
                set_floor(o);
                vel.y = 0;
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
    else if (&hb == &body_hb && o_hb.layers_2 & Layers::Walker_Semisolid) {
        if (fly) return;
        Rect here = hb.box + pos;
        Rect there = o_hb.box + o.pos;
        Rect overlap = here & there;
        expect(proper(overlap));
        if (vel.y < 0 && overlap.b == here.b && height(overlap) <= 2 - vel.y) {
            // Hack to make dead bodies not hang over the edge as much
            if (state == WS::Dead && !contains(lr(there), pos.x)) return;
            pos.y += height(overlap);
            set_floor(o);
            vel.y = 0;
        }
    }
    else if (&hb == &body_hb && o_hb.layers_2 & Layers::Walker_Walker) {
        auto& other = static_cast<Walker&>(o);
        if (state == WS::Dead || other.state == WS::Dead) return;
        if (!fly && !other.fly && !!floor != !!other.floor) return;
        if (invincible || other.invincible) return;
        float diff = pos.x < o.pos.x ? 1 : -1;
        pos.x -= diff;
        o.pos.x += diff;
    }
    else if (&hb == &weapon_hb && o_hb.layers_2 & Layers::Weapon_Solid) {
        if (!hit_sound) hit_sound = data->hit_solid_sound;
        do_recoil = true;
    }
    else if (&hb == &weapon_hb && o_hb.layers_2 & Layers::Weapon_Walker) {
        auto& other = static_cast<Walker&>(o);
        if (!other.invincible) {
            Walker_on_hit(hb, other, o_hb);
        }
    }
}

void Walker::Walker_on_hit (const Hitbox&, Walker& victim, const Hitbox&) {
    if (state == WS::Dead) {
        mutual_kill = victim.mutual_kill = true;
    }
    else set_state(WS::Hit);
    victim.set_state(WS::Dead);
    if (victim.data->attack_sound) victim.data->attack_sound->stop();
    if (victim.data->damage_sound) victim.data->damage_sound->play();
}

void Walker::Resident_after_step () {
    if (business != WB::Frozen) {
        floor = new_floor;
        new_floor = null;
        if (floor) fly = false;
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
            if (fly) {
                if (left_flip(vel.x) > 0.5) {
                    r = poses.fly[1];
                }
                else if (left_flip(vel.x) < -0.5) {
                    r = poses.fly[2];
                }
                else r = poses.fly[0];
            }
            else if (crouch) {
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
        case WS::Stun: {
            r = poses.damage;
            break;
        }
        case WS::Dead: {
            switch (anim_phase) {
                case 0: case 1: case 2: {
                    r = mutual_kill ? poses.hit[0] : r = poses.damage;
                    r.damage_overlap = true;
                    break;
                }
                case 3: r = poses.damagefall; break;
                default: r = poses.dead[anim_phase - 4]; break;
            }
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
            draw_layers(
                *pose.head, head_layers,
                pos + head_offset, pose.z, scale, body_tint
            );
        }
        draw_layers(
            *pose.body, body_layers & pose.body_layers,
            pos, pose.z, scale, body_tint
        );
        if (pose.damage_overlap && decal_index < max_decals) {
            float cutoff = pose.body->decals[decal_index].x;
            Frame overlap = *pose.body;
            if (overlap.bounds.r > cutoff) overlap.bounds.r = cutoff;
            draw_layers(
                overlap, body_layers & pose.body_layers,
                pos, Z::Overlap, scale, body_tint
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
                    pos + head_offset, Z::Overlap, scale, body_tint
                );
            }
        }
        Walker_draw_weapon(pose);
    }
    draw_decal(*this, pose);
}

void Walker::Walker_draw_weapon (const Pose& pose) {
    if (pose.weapon && pose.weapon->target) {
        Vec w_scale = defined(override_weapon_scale)
            ? override_weapon_scale : Vec(left ? -1 : 1, 1);
        Vec w_pos = defined(override_weapon_pos)
            ? override_weapon_pos : pos + pose.body->weapon * w_scale;
        float z = pose.z;
        if (pose.damage_overlap && !mutual_kill) z = Z::Overlap;
        draw_layers(
            *pose.weapon, weapon_layers,
            w_pos, z, w_scale, weapon_tint
        );
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::BodyFrame,
    elems(
        elem(&BodyFrame::target),
        elem(&BodyFrame::offset),
        elem(&BodyFrame::head, optional),
        elem(&BodyFrame::weapon, optional),
        elem(&BodyFrame::decals, optional),
        elem(&BodyFrame::decal_dirs, optional)
    ),
     // Just for castability.  We need a better solution.
    attrs(
        attr("vf::Frame", base<Frame>(), include),
        attr("head", &BodyFrame::head, optional),
        attr("weapon", &BodyFrame::weapon, optional),
        attr("decals", &BodyFrame::decals, optional),
        attr("decal_dirs", &BodyFrame::decal_dirs, optional)
    ),
    init([](BodyFrame& v){ v.init(); })
)

AYU_DESCRIBE(vf::WeaponFrame,
    elems(
        elem(&WeaponFrame::target),
        elem(&WeaponFrame::offset),
        elem(&WeaponFrame::hitbox, optional)
    ),
    attrs(
        attr("vf::Frame", base<Frame>(), include),
        attr("hitbox", &WeaponFrame::hitbox, optional)
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
        attr("fly", &WalkerPoses::fly, optional),
        attr("land", &WalkerPoses::land),
        attr("attack", &WalkerPoses::attack),
        attr("crouch_attack", &WalkerPoses::crouch_attack),
        attr("hit", &WalkerPoses::hit),
        attr("damage", &WalkerPoses::damage),
        attr("damagefall", &WalkerPoses::damagefall),
        attr("dead", &WalkerPoses::dead)
    )
)

AYU_DESCRIBE(vf::WalkerFlavor,
    values(
        value("strawberry", WF::Strawberry),
        value("lemon", WF::Lemon)
    )
)

AYU_DESCRIBE(vf::WalkerData,
    attrs(
        attr("body_box", &WalkerData::body_box),
        attr("crouch_body_box", &WalkerData::crouch_body_box),
        attr("dead_body_box", &WalkerData::dead_body_box),
        attr("damage_box", &WalkerData::damage_box),
        attr("crouch_damage_box", &WalkerData::crouch_damage_box),
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
        attr("gravity_dead", &WalkerData::gravity_dead),
        attr("gravity_fly", &WalkerData::gravity_fly),
        attr("can_fly", &WalkerData::can_fly),
        attr("drop_duration", &WalkerData::drop_duration),
        attr("land_sequence", &WalkerData::land_sequence),
        attr("attack_sequence", &WalkerData::attack_sequence),
        attr("hit_sequence", &WalkerData::hit_sequence),
        attr("dead_sequence", &WalkerData::dead_sequence),
        attr("jump_crouch_lift", &WalkerData::jump_crouch_lift, optional),
        attr("dead_floor_lift", &WalkerData::dead_floor_lift),
        attr("fly_floor_lift", &WalkerData::fly_floor_lift, optional),
        attr("hold_buffer", &WalkerData::hold_buffer),
        attr("walk_cycle_dist", &WalkerData::walk_cycle_dist),
        attr("fall_cycle_dist", &WalkerData::fall_cycle_dist),
        attr("jump_end_vel", &WalkerData::jump_end_vel),
        attr("fall_start_vel", &WalkerData::fall_start_vel),
        attr("step_sound", &WalkerData::step_sound, optional),
        attr("land_sound", &WalkerData::land_sound, optional),
        attr("attack_sound", &WalkerData::attack_sound, optional),
        attr("crouch_attack_sound", &WalkerData::crouch_attack_sound, optional),
        attr("hit_solid_sound", &WalkerData::hit_solid_sound, optional),
        attr("damage_sound", &WalkerData::damage_sound, optional),
        attr("paralyzed_sound", &WalkerData::paralyzed_sound, optional),
        attr("poses", &WalkerData::poses),
        attr("decals", &WalkerData::decals),
        attr("flavor", &WalkerData::flavor)
    )
)

AYU_DESCRIBE(vf::Walker,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Walker::data),
        attr("mind", &Walker::mind),
        attr("vel", &Walker::vel, optional),
        attr("left", &Walker::left, optional),
        attr("crouch", &Walker::crouch, optional),
        attr("fly", &Walker::fly, optional),
        attr("state", &Walker::state, optional),
        attr("anim_phase", &Walker::anim_phase, optional),
        attr("anim_timer", &Walker::anim_timer, optional),
        attr("drop_timer", &Walker::drop_timer, optional),
        attr("floor", &Walker::floor, optional),
        attr("override_weapon_pos", &Walker::override_weapon_pos, optional),
        attr("walk_start_x", &Walker::walk_start_x, optional),
        attr("fall_start_x", &Walker::walk_start_x, optional),
        attr("body_tint", &Walker::body_tint, optional),
        attr("weapon_tint", &Walker::weapon_tint, optional),
        attr("decal_type", &Walker::decal_type, optional),
        attr("decal_index", &Walker::decal_index, optional),
        attr("poison_level", &Walker::poison_level, optional),
        attr("poison_timer", &Walker::poison_timer, optional),
        attr("paralyze_symbol_timer", &Walker::paralyze_symbol_timer, optional),
        attr("body_layers", &Walker::body_layers, optional),
        attr("head_layers", &Walker::head_layers, optional),
        attr("weapon_layers", &Walker::weapon_layers, optional),
        attr("pending_weapon_layers", &Walker::pending_weapon_layers, optional),
        attr("mutual_kill", &Walker::mutual_kill, optional),
        attr("stun_duration", &Walker::stun_duration, optional),
        attr("no_crouch_timer", &Walker::no_crouch_timer, optional)
    )
)
