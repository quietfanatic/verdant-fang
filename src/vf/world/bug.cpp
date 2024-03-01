#include "bug.h"
#include <random>
#include "../../dirt/ayu/reflection/describe.h"
#include "../game/game.h"
#include "../game/options.h"
#include "../game/state.h"
#include "math.h"

namespace vf {

struct BugPoses : WalkerPoses {
    Pose spit [3];
    Frame* projectile [2];
    Frame* projectile_hit [2];
};

struct BugData : WalkerData {
    Rect projectile_box;
    uint8 spit_sequence [3];
    Vec spit_mouth_offset;
     // up, diagonal up, neutral, diagonal down, down
    Vec projectile_vel [5];
    float projectile_gravity;
    uint32 projectile_duration;
    uint8 projectile_anim_cycle;
    uint8 projectile_hit_sequence [2];
    uint8 projectile_stun;
    Sound* unstab_sound = null;
    Sound* spit_sound = null;
    Sound* projectile_hit_sound = null;
};

Bug::Bug () {
    types |= Types::Bug;
    projectile_hb.layers_1 = Layers::Projectile_Walker
                           | Layers::Projectile_Solid;
}

void Bug::init () {
    if (!defined(home_pos)) home_pos = pos;
    if (!home_left) home_left = left;
}

WalkerBusiness Bug::Walker_business () {
    auto& bd = static_cast<BugData&>(*data);
    if (projectile_state == 1) {
        projectile_vel.y -= bd.projectile_gravity;
        projectile_pos += projectile_vel;
        projectile_timer += 1;
    }
    else if (projectile_state == 2) {
        uint32 dur = bd.projectile_hit_sequence[0]
                   + bd.projectile_hit_sequence[1];
        if (projectile_timer >= dur) {
            projectile_state = 0;
            projectile_timer = 0;
            projectile_pos = GNAN;
            projectile_vel = GNAN;
        }
        else projectile_timer += 1;
    }
    if (state == WS::Hit) {
        if (anim_phase == 1 && anim_timer == data->hit_sequence[1]) {
            if (bd.unstab_sound) bd.unstab_sound->play();
        }
        return Walker::Walker_business();
    }
    else if (state == BS::Spit) {
        expect(anim_phase < 3);
        uint32 limit = bd.spit_sequence[anim_phase];
        if (data->can_be_easy &&
            current_game->options().enemy_difficulty <= 1
        ) limit *= 2;
        if (anim_timer >= limit) {
            if (anim_phase == 2) {
                set_state(WS::Neutral);
            }
            else {
                anim_phase += 1;
                anim_timer = 0;
            }
            return Walker_business();
        }
        else {
            anim_timer += 1;
            return WB::Occupied;
        }
    }
    else return Walker::Walker_business();
}

void Bug::Walker_move (const Controls& controls) {
    auto& bd = static_cast<BugData&>(*data);
    if (state == BS::Spit && anim_phase == 2 && anim_timer == 1) {
        projectile_state = 1;
        projectile_pos = pos + left_flip(bd.spit_mouth_offset);
        if (controls[Control::Up] && !controls[Control::Down]) {
            if (controls[Control::Left] || controls[Control::Right]) {
                projectile_vel = left_flip(bd.projectile_vel[1]);
            }
            else projectile_vel = left_flip(bd.projectile_vel[0]);
        }
        else if (controls[Control::Down]) {
            if (controls[Control::Left] || controls[Control::Right]) {
                projectile_vel = left_flip(bd.projectile_vel[3]);
            }
            else projectile_vel = left_flip(bd.projectile_vel[4]);
        }
        else projectile_vel = left_flip(bd.projectile_vel[2]);
        if (bd.spit_sound) bd.spit_sound->play();
    }
    if (business == WB::Free && controls[Control::Special] &&
        controls[Control::Special] <= data->hold_buffer &&
        projectile_state == 0
    ) {
        set_state(BS::Spit);
    }
    Walker::Walker_move(controls);
}

void Bug::Resident_before_step () {
    wings_timer += 1;
    if (wings_timer >= 6) wings_timer = 0;
    Walker::Resident_before_step();
    if (projectile_state == 1) {
        auto& bd = static_cast<BugData&>(*data);
         // projectile_pos is absolute, but hitbox is relative to self pos
        projectile_hb.box = projectile_pos - pos + bd.projectile_box;
        add_hitbox(projectile_hb);
    }
}

void Bug::Resident_on_collide (
    const Hitbox& hb, Resident& o, const Hitbox& o_hb
) {
    if (&hb == &projectile_hb && o_hb.layers_2 & Layers::Projectile_Solid) {
         // Go through ceiling
        if (projectile_pos.y < 120) {
            projectile_state = 2;
            projectile_timer = 0;
        }
    }
    else if (&hb == &projectile_hb && o_hb.layers_2 & Layers::Projectile_Walker) {
        auto& victim = static_cast<Walker&>(o);
        if (victim.state != WS::Dead && !victim.invincible &&
            !(victim.types & Types::Bug)
        ) {
            auto& bd = static_cast<BugData&>(*data);
            victim.set_state(WS::Stun);
            victim.stun_duration = bd.projectile_stun;
            victim.poison_level += 1;
            victim.body_tint = 0xeedd2280;
             // Don't play sound twice on double collision (probably doesn't
             // matter if it has an assigned channel)
            if (projectile_state == 1) {
                if (bd.projectile_hit_sound) bd.projectile_hit_sound->play();
            }
            projectile_state = 2;
            projectile_timer = 0;
        }
    }
    else Walker::Resident_on_collide(hb, o, o_hb);
}

void Bug::Walker_on_hit (const Hitbox& hb, Walker& victim, const Hitbox& o_hb) {
     // Find place to stab
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    auto& weapon_frame = *data->poses->hit[0].weapon;
    auto& victim_body = *victim.data->poses->damage.body;
    usize min_dist = GINF;
    usize decal_i = -1;
    float weapon_y = pos.y + weapon_offset.y + center(weapon_frame.hitbox).y;
    for (usize i = 0; i < max_decals; i++) {
        float decal_y = victim.pos.y + victim_body.decals[i].y;
        float dist = distance(decal_y, weapon_y);
        if (dist < min_dist) {
            min_dist = dist;
            decal_i = i;
        }
    }
     // Calculate stab depth
    float weapon_tip = pos.x + left_flip(
        weapon_offset.x + data->poses->hit[0].weapon->hitbox.r
    );
    Vec decal_pos = victim.pos + victim.left_flip(victim_body.decals[decal_i]);
    float stab_depth = left_flip(weapon_tip - decal_pos.x);
     // Stab
    victim.decal_type = DecalType::Stab;
    victim.decal_index = decal_i;
    if (victim.data->flavor == WF::Lemon) pending_weapon_layers |= 0x8;
    else if (stab_depth > 12) pending_weapon_layers |= 0x4;
    else pending_weapon_layers |= 0x2;
     // Move victim vertically
    float height_diff = decal_pos.y - weapon_y;
    victim.pos.y -= height_diff;
     // Move victor horizontally
    if (stab_depth < 2) {
        pos.x += left_flip(2 - stab_depth);
    }
    else if (stab_depth > 16) {
        pos.x -= left_flip(stab_depth - 16);
    }
     // Supercall
    Walker::Walker_on_hit(hb, victim, o_hb);
}

Pose Bug::Walker_pose () {
    auto& bd = static_cast<BugData&>(*data);
    auto& poses = static_cast<BugPoses&>(*bd.poses);
    Pose r;
    if (state == BS::Spit) {
        r = poses.spit[anim_phase];
    }
    else r = Walker::Walker_pose();
     // Using the head segment for the wings
    if (r.head == poses.fly[0].head || r.head == poses.fly[1].head) {
        r.head = wings_timer < 3 ? poses.fly[0].head : poses.fly[1].head;
    }
    return r;
}

void Bug::Resident_draw () {
    auto& bd = static_cast<BugData&>(*data);
    auto& poses = static_cast<BugPoses&>(*bd.poses);
    Frame* projectile_frame = null;
    if (projectile_state == 1) {
        projectile_frame = poses.projectile[
            (projectile_timer * 2 / bd.projectile_anim_cycle) % 2
        ];
    }
    else if (projectile_state == 2) {
        if (projectile_timer < bd.projectile_hit_sequence[0]) {
            projectile_frame = poses.projectile_hit[0];
        }
        else projectile_frame = poses.projectile_hit[1];
    }
    if (projectile_frame) {
        draw_frame(*projectile_frame, 0, projectile_pos, Z::Projectile);
    }
    Walker::Resident_draw();
}

void Bug::Resident_on_exit () {
    if (state == WS::Dead) {
         // Finish dying offscreen
        anim_phase = 10;
        anim_timer = 0;
    }
    else {
         // Otherwise return to start
        set_state(WS::Neutral);
        pos = home_pos;
        vel = {0, 0};
        left = *home_left;
        weapon_layers = 1;
        alert_phase = 0;
        alert_timer = 0;
    }
    Walker::Resident_on_exit();
}

Controls BugMind::Mind_think (Resident& s) {
    Controls r {};
    if (!(s.types & Types::Bug)) return r;
    if (!target) return r;
    auto& self = static_cast<Bug&>(s);
    auto& bd = static_cast<BugData&>(*self.data);

    auto set_roam_timer = [&]{
        auto& rng = current_game->state().rng;
         // Limit number of roam_pos choices
        for (usize i = 0; i < 8; i++) {
            self.roam_pos.x = std::uniform_real_distribution<float>(
                self.roam_territory.l, self.roam_territory.r
            )(rng);
             // Bias toward top of the room
            float h = std::uniform_real_distribution<float>(0, 1)(rng);
            self.roam_pos.y = self.roam_territory.b +
                height(self.roam_territory) * (1 - length2(1 - h));
             // Rechoose roam position if it overlaps with another bug's roam
             // position.
            for (auto r : self.room->residents) {
                if (!(r->types & Types::Bug)) continue;
                auto& fren = static_cast<Bug&>(*r);
                if (fren.state == WS::Dead) continue; // :(
                if (distance2(self.roam_pos, fren.roam_pos) < length2(32)) {
                    goto rechoose;
                }
            }
            break;
            rechoose:;
        }

        self.roam_timer = std::uniform_int_distribution(
            roam_interval.l, roam_interval.r
        )(rng);
    };
    auto set_spit_timer = [&]{
        auto& rng = current_game->state().rng;
        self.spit_timer = std::uniform_int_distribution(
            spit_interval.l, spit_interval.r
        )(rng);
    };

    Vec rel = target->pos - self.pos;
     // Predict where target will be when attack hitbox comes out.
    float attack_delay = self.data->attack_sequence[0]
                       + self.data->attack_sequence[1];
    Vec pred_attack_pos = Vec(
        quadratic_until_stop(
            self.pos.x, self.vel.x,
            self.left_flip(-self.data->air_dec),
            attack_delay
        ),
        quadratic(self.pos.y, self.vel.y, -self.data->gravity_fly, attack_delay)
    );
    Vec pred_target_pos = Vec(
        quadratic_until_stop(
            target->pos.x, target->vel.x,
            target->left_flip(
                target->floor ? -target->data->ground_dec : -target->data->air_dec
            ),
            attack_delay
        ),
        quadratic(
            target->pos.y, target->vel.y,
             // Don't know what target's current gravity is because we can't
             // read their inputs and we have no memory, so guess something.
            target->floor ? 0
            : target->vel.y > 0 ? -target->data->gravity_jump
            : -target->data->gravity_fall,
            attack_delay
        )
    );
    Vec attack_rel = pred_target_pos - pred_attack_pos;
    Rect actual_attack_area = attack_area;
     // Take target's hitbox into account when deciding to attack.  To increase
     // hitbox size, decrease l and b and increase r and t.  Target hitbox's l
     // and b are likely zero or negative.  Assume target is facing left.
    actual_attack_area.l += target->data->damage_box.l;
    actual_attack_area.b -= target->data->damage_box.t;
    actual_attack_area.r += target->data->damage_box.r;
    actual_attack_area.t -= target->data->damage_box.b;
    actual_attack_area = self.left_flip(actual_attack_area);

     // Note that unlike Monster's AI, our coordinate system is not flipped if
     // self is facing left.

    next_alert_phase:
    if (self.alert_phase == 0) {
        if (length2(rel) < length2(sight_range)) {
            self.alert_phase = 1;
            goto next_alert_phase;
        }
    }
    else if (self.alert_phase == 1) {
        if (self.alert_timer >= alert_sequence) {
             // Alert other bugs
            for (auto r : self.room->residents) {
                if (!(r->types & Types::Bug)) continue;
                auto& fren = static_cast<Bug&>(*r);
                if (fren.state == WS::Dead) continue;
                if (fren.alert_phase == 0) fren.alert_phase = 1;
            }
            set_roam_timer();
            set_spit_timer();
            self.alert_phase = 2;
            self.alert_timer = 0;
            goto next_alert_phase;
        }
        else {
            self.alert_timer += 1;
             // Do nothing, haven't reacted yet
        }
    }
    else if (self.alert_phase == 2) {
        if (target->state == WS::Dead) {
            self.fly = false;
        }
        else if (self.state == BS::Spit) {
             // Aim projectile.  Don't predict positions for this one.  The most
             // dangerous scenario for us is if the target is headed straight
             // for us, which shouldn't change our aim much.
            float dist = self.left_flip(rel.x);
            float aimable_xs [5];
            for (int i = 0; i < 5; i++) {
                 // This returns a time, so if have to multiply it by a speed to
                 // get a distance.
                aimable_xs[i] = positive_quadratic_root(
                     // Subtract an amount for the target's height, and also an
                     // amount that I'm not really sure why
                    -rel.y - 24,
                    bd.projectile_vel[i].y,
                    -bd.projectile_gravity
                ) * bd.projectile_vel[i].x;
                 // I don't know what I'm doing wrong for these to be off by a
                 // factor of two, but they are, and I don't have time to
                 // investigate.
                aimable_xs[i] *= 2;
            }
            uint8 closest_i = 0;
            float closest_x = distance(aimable_xs[0], dist);
            for (int i = 1; i < 5; i++) {
                float x = distance(aimable_xs[i], dist);
                if (x < closest_x) {
                    closest_i = i;
                    closest_x = x;
                }
            }
            switch (closest_i) {
                case 0: r[Control::Up] = 1; break;
                case 1: r[Control::Up] = 1; r[Control::Left] = 1; break;
                case 2: break;
                case 3: r[Control::Down] = 1; r[Control::Left] = 1; break;
                case 4: r[Control::Down] = 1; break;
                default: never();
            }
            if (debug) {
                draw_rectangle(
                    self.pos + rel + Vec(0, 24) + Rect(-4, -4, 4, 4),
                    0xff000080, Z::Debug
                );
                for (auto& x : aimable_xs) {
                    if (defined(x)) {
                        draw_rectangle(
                            Vec(self.pos.x + self.left_flip(x), self.pos.y + (rel.y + 24))
                                + Rect(-4, -4, 4, 4),
                            0x00ff0080, Z::Debug
                        );
                    }
                }
            }
        }
        else if (self.state == WS::Attack && self.anim_phase < 3) {
             // Occupied with attacking, don't do anything else
        }
        else if (self.floor) {
             // Start flying
            r[Control::Up] = 1;
        }
        else if (self.left_flip(rel.x) < 0) {
             // Turn around here because flying physics don't allow turning
             // around
            self.left = !self.left;
        }
        else if (contains(actual_attack_area, attack_rel)) {
            r[Control::Attack] = 1;
        }
        else {
            if (self.roam_timer == 0 ||
                distance(self.roam_pos.x, target->pos.x) < personal_space
            ) {
                set_roam_timer();
            }
            else self.roam_timer -= 1;
            if (self.spit_timer == 0 && self.projectile_state == 0) {
                if (self.vel.y >= 0) {
                    r[Control::Special] = 1;
                    set_spit_timer();
                }
                else {
                    r[Control::Up] = 1;
                }
            }
            else {
                if (self.spit_timer) self.spit_timer -= 1;
                if (self.pos.x < self.roam_pos.x - roam_tolerance) {
                    r[Control::Right] = 1;
                }
                else if (self.pos.x > self.roam_pos.x + roam_tolerance) {
                    r[Control::Left] = 1;
                }
                if (self.pos.y < self.roam_pos.y - roam_tolerance) {
                    r[Control::Up] = 1;
                }
                else if (self.pos.y > self.roam_pos.y + roam_tolerance) {
                    r[Control::Down] = 1;
                }
            }
        }
    }
    return r;
}

} using namespace vf;

AYU_DESCRIBE(vf::Bug,
    attrs(
        attr("vf::Walker", base<Walker>(), include),
        attr("home_pos", &Bug::home_pos, optional),
        attr("home_left", &Bug::home_left, optional),
        attr("wings_timer", &Bug::wings_timer, optional),
        attr("alert_phase", &Bug::alert_phase, optional),
        attr("alert_timer", &Bug::alert_timer, optional),
        attr("roam_pos", &Bug::roam_pos, optional),
        attr("roam_territory", &Bug::roam_territory),
        attr("roam_timer", &Bug::roam_timer, optional),
        attr("spit_timer", &Bug::spit_timer, optional)
    ),
    init<&Bug::init>()
)

AYU_DESCRIBE(vf::BugMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &BugMind::target),
        attr("sight_range", &BugMind::sight_range),
        attr("alert_sequence", &BugMind::alert_sequence, optional),
        attr("personal_space", &BugMind::personal_space, optional),
        attr("attack_area", &BugMind::attack_area),
        attr("roam_interval", &BugMind::roam_interval),
        attr("roam_tolerance", &BugMind::roam_tolerance, optional),
        attr("spit_interval", &BugMind::spit_interval),
        attr("spit_range_cutoffs", &BugMind::spit_range_cutoffs, optional),
        attr("debug", &BugMind::debug, optional)
    )
)

AYU_DESCRIBE(vf::BugPoses,
    attrs(
        attr("vf::WalkerPoses", base<WalkerPoses>(), include),
        attr("spit", &BugPoses::spit),
        attr("projectile", &BugPoses::projectile),
        attr("projectile_hit", &BugPoses::projectile_hit)
    )
)

AYU_DESCRIBE(vf::BugData,
    attrs(
        attr("vf::WalkerData", base<WalkerData>(), include),
        attr("projectile_box", &BugData::projectile_box),
        attr("spit_sequence", &BugData::spit_sequence),
        attr("spit_mouth_offset", &BugData::spit_mouth_offset),
        attr("projectile_vel", &BugData::projectile_vel),
        attr("projectile_gravity", &BugData::projectile_gravity),
        attr("projectile_duration", &BugData::projectile_duration),
        attr("projectile_anim_cycle", &BugData::projectile_anim_cycle),
        attr("projectile_hit_sequence", &BugData::projectile_hit_sequence),
        attr("projectile_stun", &BugData::projectile_stun),
        attr("unstab_sound", &BugData::unstab_sound, optional),
        attr("spit_sound", &BugData::spit_sound, optional),
        attr("projectile_hit_sound", &BugData::projectile_hit_sound, optional)
    )
)
