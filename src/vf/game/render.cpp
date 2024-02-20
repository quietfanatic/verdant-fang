#include "render.h"

#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/glow/gl.h"
#include "../../dirt/glow/image.h"
#include "../../dirt/glow/image-texture.h"
#include "../../dirt/glow/program.h"
#include "../../dirt/glow/texture.h"
#include "../../dirt/iri/iri.h"
#include "camera.h"

namespace vf {

struct FrameProgram : glow::Program {
    int u_screen_rect = -1;
    int u_tex_rect = -1;
    int u_tint = -1;
    int u_fade = -1;

    void Program_after_link () override {
        u_screen_rect = glGetUniformLocation(id, "u_screen_rect");
        u_tex_rect = glGetUniformLocation(id, "u_tex_rect");
        u_tint = glGetUniformLocation(id, "u_tint");
        u_fade = glGetUniformLocation(id, "u_fade");
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
        require(u_screen_rect != -1);
        require(u_tex_rect != -1);
        require(u_tint != -1);
        require(u_fade != -1);
        require(u_tex != -1);
    }

    struct Command {
        Rect screen_rect;
        Rect tex_rect;
        uint32 tex;
        float z;
        glow::RGBA8 tint;
        float fade;
    };
    static constexpr usize max_commands = 256;
    usize n_commands = 0;
    Command commands [max_commands];
};

static FrameProgram* frame_program = null;

void Frame::init () {
#ifdef NDEBUG
     // Skip verifification in release builds
    if (bounds) return;
#endif
    if (target) {
         // Cache ImageRefs
        auto layers = UniqueArray<glow::ImageRef>(
            target->layers.size(),
            [&](usize i){ return target->layers[i].source; }
        );
         // Check that all the layers are the same size
        IVec layers_size;
        for (auto& l : layers) {
            if (layers_size && l.size != layers_size) {
                raise(e_General,
                    "LayeredTexture has inconsistent layer sizes"
                );
            }
            layers_size = l.size;
        }
         // The image data we're working with has not been flipped yet, so
         // transform the coordinate system.
        auto flipv = [&](IVec p){ return IVec(p.x, layers_size.y - p.y - 1); };
        IRect region;
        if (bounds) region = bounds + offset;
        else {
             // Find a non-transparent pixel
            IVec start = offset;
            for (;;) {
                if (!contains(IRect(IVec(-1, -1), layers_size + IVec(1, 1)), start)) {
                    raise(e_General,
                        "Didn't find non-transparent pixel between offset and end "
                        "of texture."
                    );
                }
                for (auto& l : layers) {
                    for (auto& corner : {
                        IVec(-1, -1), IVec(0, -1), IVec(0, 0), IVec(-1, 0)
                    }) {
                        IVec p = start + corner;
                        if (contains(IRect(IVec(0, 0), layers_size), p) &&
                            l[flipv(p)]
                        ) {
                            goto found_start;
                        }
                    }
                }
                start += target->search_direction;
            }
            found_start:
            region = {start, start};
        }
         // If bounds is not specified, gradually expand bounds until we're
         // surrounded by transparency.  If bounds is specified, just verify
         // that it's surrounded by transparency.
        check: for (auto& l : layers) {
             // Check left and right and corners
            auto yb = region.b > 0 ? region.b-1 : 0;
            auto ye = region.t < layers_size.y ? region.t+1 : layers_size.y;
            for (int32 y = yb; y != ye; y++) {
                if (region.l > 0 && l[flipv(IVec(region.l-1, y))]) {
                    if (bounds) goto bad_bounds;
                    region.l -= 1;
                    goto check;
                }
                if (region.r < layers_size.x && l[flipv(IVec(region.r, y))]) {
                    if (bounds) goto bad_bounds;
                    region.r += 1;
                    goto check;
                }
            }
             // Check bottom and top
            for (int32 x = region.l; x != region.r; x++) {
                if (region.b > 0 && l[flipv(IVec(x, region.b-1))]) {
                    if (bounds) goto bad_bounds;
                    region.b -= 1;
                    goto check;
                }
                if (region.t < layers_size.y && l[flipv(IVec(x, region.t))]) {
                    if (bounds) goto bad_bounds;
                    region.t += 1;
                    goto check;
                }
            }
        }
        expect(area(region));
         // Finally we're done, and we should have the smallest rectangle around
         // the start that's fully surrounded by transparent pixels.
        bounds = region - offset;
        return;
        bad_bounds:
        raise(e_General, "Frame bounds are not surrounded by transparency.");
    }
}

void draw_frame (
    const Frame& frame,
    uint32 layer,
    Vec pos,
    float z,
    Vec scale,
    glow::RGBA8 tint,
    float fade
) {
    expect(frame.bounds);
    Rect world_rect = pos + Rect(frame.bounds) * scale;
    Rect tex_rect = Rect(frame.bounds + frame.offset);
    if (layer >= frame.target->layers.size()) {
#ifndef NDEBUG
        never();
#else
        return;
#endif
    }
    auto& l = frame.target->layers[layer];
    draw_texture(l, world_rect, tex_rect, z + l.z_offset, tint, fade);
}

void draw_texture (
    const glow::Texture& tex,
    const Rect& world_rect,
    const Rect& tex_rect,
    float z,
    glow::RGBA8 tint,
    float fade
) {
    require(!!tex);
    require(tex.target == GL_TEXTURE_RECTANGLE);
    if (!frame_program) {
        ayu::global(&frame_program);
        frame_program = ayu::ResourceRef(
            iri::constant("res:/vf/game/render.ayu")
        )["program"][1];
    }
    require(frame_program->n_commands < FrameProgram::max_commands);
    Rect screen_rect = world_to_screen(world_rect);
    frame_program->commands[frame_program->n_commands++] = FrameProgram::Command(
        screen_rect, tex_rect, tex, z, tint, fade
    );
}

void draw_rectangle (
    const Rect& world_rect,
    glow::RGBA8 color,
    float z
) {
    static glow::PixelTexture* empty_tex = null;
    if (!empty_tex) {
        ayu::global(&empty_tex);
        empty_tex = ayu::ResourceRef(
            iri::constant("res:/vf/game/render.ayu")
        )["empty_tex"][1];
    }
    glow::RGBA8 tint = color; tint.a = 0xff;
    draw_texture(*empty_tex, world_rect, Rect(0, 0, 1, 1), z, tint, color.a / 255.f);
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
        glUniform1fv(program.u_screen_rect, 4, &cmd.screen_rect.l);
        glUniform1fv(program.u_tex_rect, 4, &cmd.tex_rect.l);
        glUniform4f(program.u_tint,
            cmd.tint.r / 255.f, cmd.tint.g / 255.f,
            cmd.tint.b / 255.f, cmd.tint.a / 255.f
        );
        glUniform1f(program.u_fade, cmd.fade);
        glBindTexture(GL_TEXTURE_RECTANGLE, cmd.tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    program.n_commands = 0;
}

} using namespace vf;

AYU_DESCRIBE(vf::TextureLayer,
    attrs(
        attr("glow::PixelTexture", base<glow::PixelTexture>(), include),
        attr("z_offset", &TextureLayer::z_offset, optional)
    )
)

AYU_DESCRIBE(vf::LayeredTexture,
    delegate(&LayeredTexture::layers),
    attrs(
        attr("layers", &LayeredTexture::layers),
        attr("search_direction", &LayeredTexture::search_direction, optional)
    )
)

AYU_DESCRIBE(vf::Frame,
    elems(
        elem(&Frame::target),
        elem(&Frame::offset),
        elem(&Frame::bounds, optional)
    ),
    attrs(
        attr("target", &Frame::target),
        attr("offset", &Frame::offset),
        attr("bounds", &Frame::bounds, optional)
    ),
    init<&Frame::init>()
)

AYU_DESCRIBE(vf::TransitionType,
    values(
        value("wipe_left", TransitionType::WipeLeft),
        value("wipe_right", TransitionType::WipeRight),
        value("wipe_up", TransitionType::WipeUp),
        value("wipe_down", TransitionType::WipeDown),
        value("aperture_close", TransitionType::ApertureClose),
        value("aperture_open", TransitionType::ApertureOpen)
    )
)

AYU_DESCRIBE(vf::FrameProgram,
    delegate(base<glow::Program>())
)
