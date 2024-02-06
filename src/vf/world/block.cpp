#include "block.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../game/camera.h"

namespace vf {

Block::Block () {
    layers_2 |= Layers::Walker_Block;
    layers_2 |= Layers::Weapon_Block;
    pos = {0, 0};
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include),
        attr("bounds", &Resident::bounds)
    )
)
