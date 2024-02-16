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
    clear_hitboxes();
    for (auto& b : blocks) {
        b.hb.layers_2 = Layers::Walker_Solid | Layers::Weapon_Solid
                      | Layers::Projectile_Solid;
        b.hb.box = b.pos + Rect(data->bounds);
        add_hitbox(b.hb);
    }
}

void Blocks::Resident_draw () {
    for (auto& b : blocks) {
        draw_frame(*data, 0, pos + b.pos, Z::Blocks);
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    delegate(&Block::pos)
)

AYU_DESCRIBE(vf::Blocks,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("data", &Blocks::data),
        attr("blocks", &Blocks::blocks)
    ),
    init<&Blocks::init>()
)
