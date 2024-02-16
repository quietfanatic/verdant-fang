#include "switch.h"

#include "../../dirt/glow/image-texture.h"
#include "../game/render.h"
#include "../game/sound.h"
#include "walker.h"

namespace vf {

struct SwitchData {
    Rect hitbox;
    Frame ready_frame;
    Frame cooldown_frame;
    Sound* activate_sound;
};

void Switch::init () {
    hb.layers_1 = Layers::Switch_Weapon;
    hb.box = data->hitbox;
    set_hitbox(hb);
}

void Switch::Resident_before_step () {
    if (timer) {
        if (!--timer) {
            if (active) {
                for (auto a : activate) {
                    a->Activatable_activate();
                }
            }
        }
    }
    else if (cooldown) cooldown--;
}

void Switch::Resident_on_collide (const Hitbox&, Resident& o, const Hitbox&) {
    if (active && !timer && !cooldown) {
        cooldown = 60;
        if (delay) timer = delay;
        else {
            for (auto a : activate) {
                a->Activatable_activate();
            }
        }
        expect(o.types & Types::Walker);
        auto& w = static_cast<Walker&>(o);
        w.do_recoil = true;;
        w.hit_sound = data->activate_sound;
    }
}

void Switch::Resident_draw () {
    draw_frame(
        !active || timer || cooldown ? data->cooldown_frame : data->ready_frame,
        0, pos, Z::Switch
    );
}

void Switch::Activatable_activate () {
    active = !active;
}

} using namespace vf;

AYU_DESCRIBE(vf::Activatable, attrs())

AYU_DESCRIBE(vf::Switch,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vf::Activatable", base<Activatable>(), include),
        attr("data", &Switch::data),
        attr("activate", &Switch::activate, optional),
        attr("delay", &Switch::delay, optional),
        attr("timer", &Switch::timer, optional),
        attr("cooldown", &Switch::cooldown, optional),
        attr("active", &Switch::active, optional)
    ),
    init<&Switch::init>()
)

AYU_DESCRIBE(vf::SwitchData,
    attrs(
        attr("hitbox", &SwitchData::hitbox),
        attr("ready_frame", &SwitchData::ready_frame),
        attr("cooldown_frame", &SwitchData::cooldown_frame),
        attr("activate_sound", &SwitchData::activate_sound)
    )
)
