#include "camera.h"

#include <numbers>
#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/gl.h"
#include "../../dirt/glow/program.h"
#include "../../dirt/glow/texture.h"
#include "../../dirt/iri/iri.h"
#include "render.h"

namespace vf {

static float transition_t = GNAN;
static glow::Texture world_tex;
static glow::Texture old_tex;
static GLuint world_fb;

struct ZoomProgram : glow::Program {
    void Program_after_link () override {
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
    }
};
static ZoomProgram* zoom_program = null;

struct TransitionProgram : glow::Program {
    TransitionType type = TransitionType::WipeLeft;
    Vec center = Vec(0, 0);
    int u_type;
    int u_t;
    int u_center;
    int u_side;
    void Program_after_link () override {
        u_type = glGetUniformLocation(id, "u_type");
        require(u_type != -1);
        u_t = glGetUniformLocation(id, "u_t");
        require(u_t != -1);
        u_center = glGetUniformLocation(id, "u_center");
        require(u_center != -1);
        u_side = glGetUniformLocation(id, "u_side");
        require(u_side != -1);
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
    }
};
static TransitionProgram* transition_program = null;

static void setup_camera () {
    world_tex = glow::Texture(GL_TEXTURE_2D);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        camera_size.x, camera_size.y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0
    );
     // Filtering mode for entire screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    old_tex = glow::Texture(GL_TEXTURE_2D);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        camera_size.x, camera_size.y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &world_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex, 0
    );
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        raise(e_General, "Failed to set up render to texture");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void begin_camera () {
    if (!zoom_program) {
        ayu::global(&zoom_program);
        zoom_program = ayu::ResourceRef(
            iri::constant("res:/vf/game/camera.ayu")
        )["zoom_program"][1];
    }
    if (!transition_program) {
        ayu::global(&transition_program);
        transition_program = ayu::ResourceRef(
            iri::constant("res:/vf/game/camera.ayu")
        )["transition_program"][1];
    }
    if (!world_fb) setup_camera();
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glViewport(0, 0, camera_size.x, camera_size.y);
    glEnable(GL_BLEND);
}

void end_camera () {
    commit_draws();
    if (defined(transition_t)) {
        transition_program->use();
         // Ease in and out a bit
        float t = (1.f - std::cos(transition_t * std::numbers::pi_v<float>)) / 2.f;
        if (transition_program->type == TransitionType::ApertureClose ||
            transition_program->type == TransitionType::ApertureOpen
        ) {
            if (transition_program->type == TransitionType::ApertureClose) {
                t = 1 - t;
            }
             // Find farthest screen corner and start from there
            float lb2 = distance2(transition_program->center, Vec(0, 0));
            float rb2 = distance2(transition_program->center, Vec(320, 0));
            float rt2 = distance2(transition_program->center, Vec(320, 180));
            float lt2 = distance2(transition_program->center, Vec(0, 180));
            t *= sqrt(max(lb2, rb2, rt2, lt2));
        }
        else {
            t = lerp(-0.1, 1.1, t);
        }
        glUniform1f(transition_program->u_t, t);
        glBindTexture(GL_TEXTURE_2D, old_tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void finish_frame () {
    commit_draws();
    zoom_program->use();
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(
        window_viewport.l, window_viewport.b,
        width(window_viewport), height(window_viewport)
    );
    glBindTexture(GL_TEXTURE_2D, world_tex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void set_transition_center (Vec center) {
    transition_program->use();
    glUniform2f(transition_program->u_center, center.x, center.y);
    transition_program->center = center;
}

void set_transition_type (TransitionType type) {
    transition_program->use();
    glUniform1i(transition_program->u_type, int(type));
    transition_program->type = type;
}

void set_transition_t (float t) {
    transition_t = t;
}

void set_transition_side (bool side) {
    transition_program->use();
    glUniform1i(transition_program->u_side, int(side));
}

void swap_world_tex () {
    using std::swap; swap(world_tex, old_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex, 0
    );
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        raise(e_General, "Failed to change framebuffer texture");
    }
}

void window_size_changed (IVec new_size) {
     // TODO: fill the blank areas
    if (slope(new_size) > slope(camera_size)) {
         // letterbox
        float zoom = new_size.x / camera_size.x;
        float height = camera_size.y * zoom;
        float margin = new_size.y - height;
        window_viewport = IRect(0, margin / 2, new_size.x, height);
    }
    else {
         // Pillarbox
        float zoom = new_size.y / camera_size.y;
        float width = camera_size.x * zoom;
        float margin = new_size.x - width;
        window_viewport = IRect(margin / 2, 0, width, new_size.y);
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::ZoomProgram,
    delegate(base<glow::Program>())
)

AYU_DESCRIBE(vf::TransitionProgram,
    delegate(base<glow::Program>())
)
