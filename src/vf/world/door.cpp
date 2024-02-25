#include "door.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/image-texture.h"
#include "../game/render.h"
#include "../game/sound.h"
#include "verdant.h"

namespace vf {

struct DoorData {
    Rect hitbox;
    Frame frame;
    Sound* open_sound;
    Sound* close_sound;
    Sound* slam_sound;
    Sound* crush_sound;
};

void Door::init () {
    types |= Types::Door;
    hb.layers_2 = Layers::Walker_Solid | Layers::Weapon_Solid;
    hb.box = data->hitbox;
    set_hitbox(hb);
    Vec default_offset = Vec(0, height(data->hitbox));
    if (state == DoorState::Closed) {
        if (!defined(closed_pos)) closed_pos = pos;
        if (!defined(stuck_pos)) stuck_pos = closed_pos + default_offset * (1/4.f);
        if (!defined(open_pos)) open_pos = closed_pos + default_offset;
    }
    else if (state == DoorState::Stuck) {
        if (!defined(closed_pos)) closed_pos = stuck_pos - default_offset * (1/4.f);
        if (!defined(stuck_pos)) stuck_pos = pos;
        if (!defined(open_pos)) open_pos = stuck_pos + default_offset * (3/4.f);
    }
    else if (state == DoorState::Open) {
        if (!defined(open_pos)) open_pos = pos;
        if (!defined(stuck_pos)) stuck_pos = open_pos - default_offset * (3/4.f);
        if (!defined(closed_pos)) closed_pos = open_pos - default_offset;
    }
    else never();
    if (open_activate == DoorState::Stuck) {
        raise(e_General, "vf::Door with open_activate = stuck is NYI.");
    }
}

void Door::set_state (DoorState new_state) {
    if (new_state == state) return;
    if (new_state == DoorState::Closed) {
        if (data->close_sound) data->close_sound->play();
    }
    else {
        if (data->open_sound) data->open_sound->play();
    }
    state = new_state;
}

void Door::Resident_before_step () {
    if (state == DoorState::Open && pos == open_pos && troll &&
        (!no_troll_unless_open || no_troll_unless_open->state == DoorState::Open)
    ) {
        if (auto v = find_verdant()) {
            if (v->pos.y < pos.y &&
                distance(v->pos.x, pos.x) <= detection_dist
            ) {
                set_state(DoorState::Closed);
                troll = false;
                if (closed_activate == DoorState::Stuck) {
                    closed_activate = DoorState::Open;
                }
            }
        }
    }
    if (state == DoorState::Open && pos != open_pos) {
        float open_dist = distance(pos, open_pos);
        float closed_dist = distance(pos, closed_pos);
        float openness = closed_dist / (open_dist + closed_dist);
        float total_dist = distance(closed_pos, open_pos);
        float vel = openness < 0.2 ? 2
                  : openness < 0.3 ? 0.3
                  : openness < 0.7 ? 1
                  :                  0.7;
        float next = openness + vel / total_dist;
        if (next >= 0.999) pos = open_pos;
        else pos = lerp(closed_pos, open_pos, next);
    }
    else if (state == DoorState::Stuck && pos != stuck_pos) {
         // For the time being, we are not supporting transitioning from open
         // to stuck or stuck to closed.
        float stuck_dist = distance(pos, stuck_pos);
        float closed_dist = distance(pos, closed_pos);
        float openness = closed_dist / (stuck_dist + closed_dist);
        float total_dist = distance(closed_pos, stuck_pos);
        float vel = openness < 0.8 ? 2 : 0.3;
        float next = openness + vel / total_dist;
        if (next >= 0.999) {
            pos = stuck_pos;
            if (data->open_sound) data->open_sound->stop();
        }
        else pos = lerp(closed_pos, stuck_pos, next);
    }
    else if (state == DoorState::Closed && pos != closed_pos) {
        float open_dist = distance(pos, open_pos);
        float closed_dist = distance(pos, closed_pos);
        float closedness = open_dist / (open_dist + closed_dist);
        float total_dist = distance(closed_pos, open_pos);
        float vel = closedness < 0.1 ? 2
                  : closedness < 0.2 ? 0.5
                  : closedness < 0.3 ? 1.2
                  : closedness < 0.4 ? 1.4
                  : closedness < 0.5 ? 1.6
                  : closedness < 0.6 ? 1.8
                  : closedness < 0.7 ? 2
                  : 2.4;
        float next = closedness + vel / total_dist;
        if (next >= 0.999) {
            pos = closed_pos;
            data->slam_sound->play();
            if (crush) {
                if (data->crush_sound) data->crush_sound->play();
                crush = false;
            }
        }
        else pos = lerp(open_pos, closed_pos, next);
    }
    else if (state == DoorState::Closed) {
        if (open_after && !--open_after) {
            set_state(DoorState::Open);
        }
    }
    else if (state == DoorState::Open) {
        if (close_after_these[0] && close_after_these[1] &&
            close_after_these[0]->state == DoorState::Closed &&
            close_after_these[1]->state == DoorState::Closed
        ) {
            if (close_after) {
                if (!--close_after) set_state(DoorState::Closed);
            }
            else if (auto v = find_verdant()) {
                if (v->pos.y < pos.y &&
                    distance(v->pos.x, pos.x) <= detection_dist
                ) {
                    close_after = 180;
                }
            }
        }
    }
}

void Door::Resident_draw () {
    draw_all_layers(data->frame, pos, Z::Door);
}

void Door::Resident_on_enter () {
    if (auto v = find_verdant()) {
        if (open_from_left && v->pos < pos) {
            state = DoorState::Open;
            pos = open_pos;
        }
        if (open_from_right && v->pos > pos) {
            state = DoorState::Open;
            pos = open_pos;
        }
    }
}

void Door::Resident_on_exit () {
    pos = state == DoorState::Open ? open_pos
        : state == DoorState::Stuck ? stuck_pos
        : state == DoorState::Closed ? closed_pos
        : (never(), open_pos);
}

void Door::Activatable_activate () {
    if (pos == closed_pos) set_state(closed_activate);
    else if (pos == stuck_pos) set_state(stuck_activate);
    else if (pos == open_pos) set_state(open_activate);
}

} using namespace vf;

AYU_DESCRIBE(vf::DoorState,
    values(
        value("closed", DoorState::Closed),
        value("stuck", DoorState::Stuck),
        value("open", DoorState::Open)
    )
)

AYU_DESCRIBE(vf::Door,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vf::Activatable", base<Activatable>(), include),
        attr("data", &Door::data),
        attr("closed_pos", &Door::closed_pos, optional),
        attr("stuck_pos", &Door::stuck_pos, optional),
        attr("open_pos", &Door::open_pos, optional),
        attr("state", &Door::state, optional),
        attr("closed_activate", &Door::closed_activate, optional),
        attr("stuck_activate", &Door::stuck_activate, optional),
        attr("open_activate", &Door::open_activate, optional),
        attr("open_after", &Door::open_after, optional),
        attr("close_after", &Door::close_after, optional),
        attr("open_from_left", &Door::open_from_left, optional),
        attr("open_from_right", &Door::open_from_right, optional),
        attr("crush", &Door::crush, optional),
        attr("troll", &Door::troll, optional),
        attr("detection_dist", &Door::detection_dist, optional),
        attr("no_troll_unless_open", &Door::no_troll_unless_open, optional),
        attr("close_after_these", &Door::close_after_these, optional)
    ),
    init<&Door::init>(10)
)

AYU_DESCRIBE(vf::DoorData,
    attrs(
        attr("hitbox", &DoorData::hitbox),
        attr("frame", &DoorData::frame),
        attr("open_sound", &DoorData::open_sound),
        attr("close_sound", &DoorData::close_sound),
        attr("slam_sound", &DoorData::slam_sound),
        attr("crush_sound", &DoorData::crush_sound)
    )
)
