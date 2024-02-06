#include "block.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"

namespace vf {

Block::Block () {
    hb.layers_2 = Layers::Walker_Block | Layers::Weapon_Block;
    pos = {0, 0};
    hitboxes = Slice<Hitbox>(&hb, 1);
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include),
        attr("bounds", ref_func<Rect>(
            [](Block& v)->Rect&{ return v.hb.box; }
        ))
    )
)
