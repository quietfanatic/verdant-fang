#include "bug.h"
#include <random>
#include "../../dirt/ayu/reflection/describe.h"
#include "../game/game.h"
#include "../game/state.h"
#include "quadratics.h"

namespace vf {

Bug::Bug () {
    types |= Types::Bug;
}

void Bug::init () {
    if (!defined(home_pos)) home_pos = pos;
    if (!home_left) home_left = left;
}

void Bug::Resident_before_step () {
    wings_timer += 1;
    if (wings_timer >= 6) wings_timer = 0;
    Walker::Resident_before_step();
}

void Bug::Walker_on_hit (const Hitbox& hb, Walker& victim, const Hitbox& o_hb) {
     // We haven't implemented backstabbing so turn victim around.
    victim.left = !left;
     // Find place to stab
    Vec weapon_offset = data->poses->hit[0].body->weapon;
    auto& victim_body = *victim.data->poses->damage.body;
    usize min_dist = GINF;
    usize decal_i = -1;
    float weapon_y = pos.y + weapon_offset.y;
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
    if (victim.data->flavor == WF::Lemon) weapon_layers |= 0x8;
    else if (stab_depth > 12) weapon_layers |= 0x4;
    else weapon_layers |= 0x2;
     // Move victim vertically
    float height_diff = decal_pos.y - weapon_y;
    victim.pos.y -= height_diff;
     // Move victor horizontally
    if (stab_depth < -2) {
        pos.x += left_flip(2 - stab_depth);
    }
    else if (stab_depth > 16) {
        pos.x -= left_flip(stab_depth - 16);
    }
     // Supercall
    Walker::Walker_on_hit(hb, victim, o_hb);
}

Pose Bug::Walker_pose () {
    auto& poses = *data->poses;
    Pose r = Walker::Walker_pose();
     // Using the head segment for the wings
    if (r.head == poses.fly[0].head || r.head == poses.fly[1].head) {
        r.head = wings_timer < 3 ? poses.fly[0].head : poses.fly[1].head;
    }
    return r;
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
    auto& self = static_cast<Bug&>(s);
    if (!target || target->state == WS::Dead) return r;


    auto set_roam_timer = [&]{
        auto& rng = current_game->state().rng;
        self.roam_pos.x = std::uniform_real_distribution<float>(
            roam_territory.l, roam_territory.r
        )(rng);
         // Bias toward top of the room
        float h = std::uniform_real_distribution<float>(0, 1)(rng);
        self.roam_pos.y = roam_territory.b
                        + height(roam_territory) * (1 - length2(1 - h));

        self.roam_timer = std::uniform_int_distribution(
            roam_interval.l, roam_interval.r
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
        if (self.alert_timer == 0) {
             // Alert other bugs
            for (auto& r : self.room->residents) {
                if (!(r.types & Types::Bug)) continue;
                auto& fren = static_cast<Bug&>(r);
                if (fren.state == WS::Dead) continue;
                if (fren.alert_phase == 0) fren.alert_phase = 1;
            }
            set_roam_timer();
            self.alert_timer += 1;
        }
        if (self.state == WS::Attack && self.anim_phase < 3) {
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
        else if (length2(rel) < length2(personal_space)) {
            r[self.left ? Control::Right : Control::Left] = 1;
        }
        else {
            if (self.roam_timer == 0) {
                set_roam_timer();
            }
            else self.roam_timer -= 1;
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
        attr("roam_timer", &Bug::roam_timer, optional)
    ),
    init<&Bug::init>()
)

AYU_DESCRIBE(vf::BugMind,
    attrs(
        attr("vf::Mind", base<Mind>(), include),
        attr("target", &BugMind::target),
        attr("sight_range", &BugMind::sight_range),
        attr("attack_area", &BugMind::attack_area),
        attr("alert_sequence", &BugMind::alert_sequence, optional),
        attr("roam_territory", &BugMind::roam_territory),
        attr("roam_interval", &BugMind::roam_interval),
        attr("roam_tolerance", &BugMind::roam_tolerance, optional),
        attr("personal_space", &BugMind::personal_space)
    )
)
