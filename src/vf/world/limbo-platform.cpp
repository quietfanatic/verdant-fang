#include "../../dirt/ayu/reflection/describe.h"
#include "../game/render.h"
#include "../game/room.h"
#include "common.h"
#include "verdant.h"

namespace vf {

struct LimboPlatform : Resident {
    Frame* data;
    Hitbox hb;
    void init () {
        hb.layers_2 = Layers::Walker_Solid;
        hb.box = Rect(-24, -8, 24, 0);
        set_hitbox(hb);
    }
    void Resident_on_enter () override {
        if (auto v = find_verdant()) {
            pos = v->pos;
        }
    }
    void Resident_draw () override {
        draw_frame(*data, 0, pos, Z::Blocks);
    }
};

} using namespace vf;

AYU_DESCRIBE(vf::LimboPlatform,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &LimboPlatform::data)
    ),
    init<&LimboPlatform::init>()
)
