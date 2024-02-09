#include "switch.h"

namespace vf {

void Switch::init () {
    hb.layers_1 = Layers::Switch_Weapon;
    hb.box = data->frame.bounds;
    hitboxes = Slice<Hitbox>(&hb, 1);
}

void Switch::Resident_before_step () {
    if (cooldown) cooldown--;
}

void Switch::Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) {
    if (!cooldown) {
        cooldown = 60;
        if (target) target->Activatable_activate();
    }
}

void Switch::Resident_draw () {
    draw_frame(pos, data->frame, data->tex, {1, 1}, Z::Switch);
}

} using namespace vf;

AYU_DESCRIBE(vf::Activatable, attrs())

AYU_DESCRIBE(vf::Switch,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Switch::data),
        attr("target", &Switch::target, optional),
        attr("cooldown", &Switch::cooldown, optional)
    ),
    init<&Switch::init>()
)
