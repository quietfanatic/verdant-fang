#include "door.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/image-texture.h"
#include "../game/frame.h"
#include "../game/sound.h"

namespace vf {

struct DoorData {
    glow::PixelTexture tex;
    Frame frame;
};

void Door::init () {
    hb.layers_2 = Layers::Walker_Solid | Layers::Weapon_Solid;
    hb.box = data->frame.bounds;
    hitboxes = Slice<Hitbox>(&hb, 1);
    Vec default_offset = Vec(0, height(data->frame.bounds));
    if (open) {
        if (!defined(open_pos)) open_pos = pos;
        if (!defined(closed_pos)) closed_pos = open_pos - default_offset;
    }
    else {
        if (!defined(closed_pos)) closed_pos = pos;
        if (!defined(open_pos)) open_pos = closed_pos + default_offset;
    }
}

void Door::Resident_before_step () {
    if (open && pos != open_pos) {
        float open_dist = distance(pos, open_pos);
        float closed_dist = distance(pos, closed_pos);
        float openness = closed_dist / (open_dist + closed_dist);
        float total_dist = distance(closed_pos, open_pos);
        float vel = openness < 0.2 ? 2
                  : openness < 0.3 ? 0.3
                  : openness < 0.7 ? 1
                  :                  0.7;
        float next = openness + vel / total_dist;
        if (next > 1) pos = open_pos;
        else pos = lerp(closed_pos, open_pos, next);
    }
    else if (!open && pos != closed_pos) {
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
        if (next > 1) pos = closed_pos;
        else pos = lerp(open_pos, closed_pos, next);
    }
}

void Door::Resident_draw () {
    draw_frame(pos, data->frame, data->tex, {1, 1}, Z::Door);
}

void Door::Resident_on_exit () {
    if (open) pos = open_pos;
    else pos = closed_pos;
}

void Door::Activatable_activate () {
    if (pos == closed_pos) {
        open = true;
        data->sound->play();
    }
    else if (pos == open_pos) {
        open = false;
        data->sound->play();
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Door,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vf::Activatable", base<Activatable>(), include),
        attr("data", &Door::data),
        attr("closed_pos", &Door::closed_pos, optional),
        attr("open_pos", &Door::open_pos, optional),
        attr("open", &Door::open, optional)
    ),
    init<&Door::init>()
)
