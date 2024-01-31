#include "block.h"

#include "../dirt/ayu/resources/global.h"
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/resource-texture.h"
#include "../dirt/glow/texture-program.h"
#include "assets.h"
#include "camera.h"

namespace vf {

static glow::Texture* block_tex = null;

Block::Block () {
    layers_2 = Layers::Player_Block;
    if (!block_tex) {
        ayu::global(&block_tex);
        block_tex = assets()["block"][1];
    }
}

void Block::Resident_draw () {
    auto rect = world_to_screen(box + pos);
    glow::draw_texture(*block_tex, rect);
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include),
        attr("box", &Resident::box)
    )
)
