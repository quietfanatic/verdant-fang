#include "block.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/resource-texture.h"
#include "../dirt/glow/texture-program.h"
#include "assets.h"
#include "camera.h"

namespace vf {

static ayu::SharedResource block_tex_ptr;

Block::Block () {
    layers_2 = Layers::Player_Block;
    if (!block_tex_ptr) {
        block_tex_ptr = ayu::SharedResource(
            iri::constant("vf:block_tex"),
            ayu::Dynamic::make<glow::Texture*>(
                assets()["block"][1]
            )
        );
    }
}

void Block::Resident_draw () {
    auto tex = block_tex_ptr->value().as_known<glow::Texture*>();
    auto rect = world_to_screen(box + pos);
    glow::draw_texture(*tex, rect);
}

} using namespace vf;

AYU_DESCRIBE(vf::Block,
    attrs(
        attr("Resident", base<Resident>(), include),
        attr("box", &Resident::box)
    )
)
