#include "blocks.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/image-texture.h"
#include "../../dirt/iri/iri.h"
#include "../game/render.h"

namespace vf {

Blocks::Blocks () {
    pos = {0, 0};
}

void Blocks::init () {
    for (auto& b : blocks) {
        b.layers_2 = Layers::Walker_Solid | Layers::Weapon_Solid;
        b.box.r = b.box.l + 32;
        b.box.t = b.box.b + 32;
    }
    hitboxes = blocks.reinterpret<Hitbox>();
}

void Blocks::Resident_draw () {
    for (auto& b : blocks) {
        draw_frame(*data, 0, pos + lb(b.box), Z::Blocks);
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    elems(
        elem(ref_func<float>([](Block& v)->float&{ return v.box.l; })),
        elem(ref_func<float>([](Block& v)->float&{ return v.box.b; }))
    )
)

AYU_DESCRIBE(vf::Blocks,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Blocks::data),
        attr("blocks", &Blocks::blocks)
    ),
    init<&Blocks::init>()
)
