#include "frame.h"

#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/glow/gl.h"
#include "../../dirt/glow/program.h"
#include "../../dirt/glow/texture.h"
#include "../../dirt/iri/iri.h"
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

    struct Command {
        Rect screen_rect;
        Rect tex_rect;
        uint32 tex;
        float z;
    };
    static constexpr usize max_commands = 256;
    usize n_commands = 0;
    Command commands [max_commands];
};

static FrameProgram* frame_program = null;

void draw_frame (
    Vec pos, const Frame& frame, const glow::Texture& tex, Vec scale, float z
) {
    require(!!tex);
    require(tex.target == GL_TEXTURE_RECTANGLE);
    if (!frame_program) {
        ayu::global(&frame_program);
        frame_program = ayu::ResourceRef(
            iri::constant("res:/vf/game/frame.ayu")
        )["program"][1];
    }
    require(frame_program->n_commands < FrameProgram::max_commands);
    Rect screen_bounds = frame.bounds * scale;
    Rect screen_rect = world_to_screen(screen_bounds + pos);
    Rect tex_rect = frame.bounds + frame.offset;
    frame_program->commands[frame_program->n_commands++] = FrameProgram::Command(
        screen_rect, tex_rect, tex, z
    );
}

void draw_frames () {
    auto& program = *frame_program;
    program.use();
    uint8 indexes [FrameProgram::max_commands];
    for (usize i = 0; i < program.n_commands; i++) {
        indexes[i] = i;
    }
     // Sort by z.  We're using OpenGL's convention where +z points out of the
     // screen (towards viewer).
    std::stable_sort(indexes, indexes + program.n_commands, [&program](uint8 a, uint8 b){
        return program.commands[a].z < program.commands[b].z;
    });
    for (usize i = 0; i < program.n_commands; i++) {
        auto& cmd = program.commands[indexes[i]];
        draw_frame_internal(cmd.screen_rect, cmd.tex_rect, cmd.tex);
        glUniform1fv(program.u_screen_rect, 4, &cmd.screen_rect.l);
        glUniform1fv(program.u_tex_rect, 4, &cmd.tex_rect.l);
        glBindTexture(GL_TEXTURE_RECTANGLE, cmd.tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    program.n_commands = 0;
}

void draw_frame_internal (
    const Rect& screen_rect,
    const Rect& tex_rect,
    uint32 tex
) {
    auto& program = *frame_program;
    glUniform1fv(program.u_screen_rect, 4, &screen_rect.l);
    glUniform1fv(program.u_tex_rect, 4, &tex_rect.l);
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
