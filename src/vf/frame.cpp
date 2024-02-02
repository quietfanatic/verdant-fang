#include "frame.h"

#include "../dirt/ayu/resources/global.h"
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/gl.h"
#include "../dirt/glow/program.h"
#include "../dirt/glow/texture.h"
#include "../dirt/iri/iri.h"
#include "camera.h"

namespace vf {

struct FrameProgram : glow::Program {
    int u_screen_rect = -1;
    int u_tex_rect = -1;

    void Program_after_link () override {
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
        require(u_screen_rect != -1);
        require(u_tex_rect != -1);
        require(u_tex != -1);
    }
};

static FrameProgram* program = null;

void draw_frame (Vec pos, const Frame& frame, const glow::Texture& tex, BVec flip) {
    require(!!tex);
    require(tex.target == GL_TEXTURE_RECTANGLE);

    if (!program) {
        ayu::global(&program);
        program = ayu::ResourceRef(
            iri::constant("res:/vf/frame.ayu")
        )["program"][1];
    }
    program->use();

    Rect screen_bounds = frame.bounds * Vec(flip.x ? -1 : 1, flip.y ? -1 : 1);
    Rect screen_rect = world_to_screen(screen_bounds + pos);
    Rect tex_rect = frame.bounds + frame.offset;
    glUniform1fv(program->u_screen_rect, 4, &screen_rect.l);
    glUniform1fv(program->u_tex_rect, 4, &tex_rect.l);
    glBindTexture(GL_TEXTURE_RECTANGLE, tex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} using namespace vf;

AYU_DESCRIBE(vf::Frame,
    elems(
        elem(&Frame::offset),
        elem(&Frame::bounds)
    )
)

AYU_DESCRIBE(vf::FrameProgram,
    delegate(base<glow::Program>())
)
