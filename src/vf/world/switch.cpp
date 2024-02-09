#include "switch.h"
#include "../game/sound.h"

namespace vf {

struct SwitchData : TexAndFrame {
    Sound* activate_sfx;
};

void Switch::init () {
    hb.layers_1 = Layers::Switch_Weapon;
    hb.box = data->frame.bounds;
    hitboxes = Slice<Hitbox>(&hb, 1);
}

void Switch::Resident_before_step () {
    if (timer) {
        if (!--timer && target) target->Activatable_activate();
    }
    else if (cooldown) cooldown--;
}

void Switch::Resident_on_collide (const Hitbox&, Resident&, const Hitbox&) {
    if (!timer && !cooldown) {
        cooldown = 60;
        if (delay) timer = delay;
        else if (target) target->Activatable_activate();
        data->activate_sfx->play();
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
        attr("delay", &Switch::delay, optional),
        attr("timer", &Switch::timer, optional),
        attr("cooldown", &Switch::cooldown, optional)
    ),
    init<&Switch::init>()
)

AYU_DESCRIBE(vf::SwitchData,
    attrs(
        attr("vf::TexAndFrame", base<TexAndFrame>(), include),
        attr("activate_sfx", &SwitchData::activate_sfx)
    )
)
