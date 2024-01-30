#include "block.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/resource-texture.h"
#include "../dirt/glow/texture-program.h"
#include "textures.h"

namespace vf {

void Block::Resident_draw () {
    glow::ResourceTexture* tex = textures_res()["block"][1];
    auto rect = (box + pos) / Vec{320, 180};
    ayu::dump(rect);
    glow::draw_texture(*tex, rect);
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include)
    )
)
