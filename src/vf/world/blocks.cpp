#include "blocks.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/glow/image-texture.h"
#include "../../dirt/iri/iri.h"
#include "../game/frame.h"

namespace vf {

static glow::PixelTexture* block_tex = null;

Blocks::Blocks () {
    pos = {0, 0};
    if (!block_tex) {
        ayu::global(&block_tex);
        block_tex = ayu::ResourceRef(
            iri::constant("res:/vf/world/assets.ayu")
        )["block_tex"][1];
    }
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
        Frame fr = {{0, 0}, {0, 0, 32, 32}};
        draw_frame(pos + lb(b.box), fr, *block_tex, {1, 1}, Z::Blocks);
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
        attr("blocks", &Blocks::blocks)
    ),
    init<&Blocks::init>()
)
