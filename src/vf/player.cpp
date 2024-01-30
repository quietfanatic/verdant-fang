#include "player.h"

#include "../dirt/iri/iri.h"
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/resource-texture.h"
#include "../dirt/glow/texture-program.h"
#include "block.h"
#include "camera.h"
#include "textures.h"

namespace vf {

static ayu::SharedResource player_tex;

Player::Player () {
    box = {-0.5, 0, 0.5, 2};
    layers_1 = Layers::Player_Block;
    if (!player_tex) {
        player_tex = ayu::SharedResource(
            iri::constant("vf:player_tex"),
            ayu::Dynamic::make<glow::Texture*>(
                textures_res()["block"][1]
            )
        );
    }
}

void Player::Resident_before_step () {
    vel += Vec(0, -0.01);
    pos += vel;
    floor = null;
}

void Player::Resident_collide (Resident& other) {
    expect(other.layers_2 & Layers::Player_Block);
    auto& block = static_cast<Block&>(other);

    Rect here = box + pos;
    Rect there = block.box + block.pos;
    Rect overlap = here & there;
    expect(proper(overlap));
    if (height(overlap) <= width(overlap)) {
        vel.y = 0;
        if (overlap.b == here.b) {
            pos.y += height(overlap);
            floor = &block;
        }
        else if (overlap.t == here.t) {
            pos.y -= height(overlap);
        }
        else never();
    }
    else {
        if (overlap.l == here.l) {
            pos.x += width(overlap);
        }
        else if (overlap.r == here.r) {
            pos.x -= width(overlap);
        }
        else never();
    }
}

void Player::Resident_draw () {
    auto tex = player_tex->value().as_known<glow::Texture*>();
    draw_texture(*tex, world_to_screen(box + pos));
}

} using namespace vf;

AYU_DESCRIBE(vf::Player,
    attrs(
        attr("vf::Resident", base<Resident>(), include),
        attr("vel", &Player::vel, optional)
    )
)
