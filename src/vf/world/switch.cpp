#include "switch.h"

#include "../../dirt/glow/image-texture.h"
#include "../game/render.h"
#include "../game/sound.h"
#include "walker.h"

namespace vf {

struct SwitchData {
    Rect hitbox;
    Frame frame;
    Sound* activate_sfx;
};

void Switch::init () {
    hb.layers_1 = Layers::Switch_Weapon;
    hb.box = data->hitbox;
    hitboxes = Slice<Hitbox>(&hb, 1);
}

void Switch::Resident_before_step () {
    if (timer) {
        if (!--timer && target) target->Activatable_activate();
    }
    else if (cooldown) cooldown--;
}

void Switch::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    if (!timer && !cooldown) {
        cooldown = 60;
        if (delay) timer = delay;
        else if (target) target->Activatable_activate();
        expect(o.types & Types::Walker);
        auto& w = static_cast<Walker&>(o);
        w.recoil();
        w.hit_sound = data->activate_sfx;
    }
}

void Switch::Resident_draw () {
    draw_frame(data->frame, 0, pos, Z::Switch);
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
        attr("hitbox", &SwitchData::hitbox),
        attr("frame", &SwitchData::frame),
        attr("activate_sfx", &SwitchData::activate_sfx)
    )
)
