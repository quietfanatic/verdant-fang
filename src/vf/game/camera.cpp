#include "camera.h"

#include "../../dirt/ayu/resources/global.h"
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/gl.h"
#include "../../dirt/glow/program.h"
#include "../../dirt/glow/texture.h"
#include "../../dirt/iri/iri.h"
#include "frame.h"

namespace vf {

static uint32 wipe_timer = 0;
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

struct WipeProgram : glow::Program {
    int u_wipe_pos;
    int u_wipe_dir;
    void Program_after_link () override {
        u_wipe_pos = glGetUniformLocation(id, "u_wipe_pos");
        require(u_wipe_pos != -1);
        u_wipe_dir = glGetUniformLocation(id, "u_wipe_dir");
        require(u_wipe_dir != -1);
        int u_tex = glGetUniformLocation(id, "u_tex");
        glUniform1i(u_tex, 0);
    }
};
static WipeProgram* wipe_program = null;

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
    if (!wipe_program) {
        ayu::global(&wipe_program);
        wipe_program = ayu::ResourceRef(
            iri::constant("res:/vf/game/camera.ayu")
        )["wipe_program"][1];
    }
    if (!world_fb) setup_camera();
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glViewport(0, 0, camera_size.x, camera_size.y);
    glEnable(GL_BLEND);
}

void end_camera () {
    draw_frames();
    if (wipe_timer) {
        wipe_timer -= 1;
        wipe_program->use();
        float wipe_t = float(wipe_timer) / wipe_duration;
         // Ease in and out a bit
        wipe_t = (1.f - std::cos(wipe_t * float(M_PI))) / 2.f;
        glUniform1f(
            wipe_program->u_wipe_pos,
            lerp(-0.1f, 1.1f, wipe_t)
        );
        glBindTexture(GL_TEXTURE_2D, old_tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
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

void start_transition_effect (Vec direction) {
    expect(length2(direction) == 1);
    wipe_program->use();
    glUniform1i(wipe_program->u_wipe_dir,
          direction.x > 0.4 ? 0
        : direction.y > 0.4 ? 1
        : direction.x < 0.4 ? 2
        :                     3
    );
}

void swap_world_tex () {
    using std::swap; swap(world_tex, old_tex);
    wipe_timer = wipe_duration;
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex, 0
    );
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        raise(e_General, "Failed to change framebuffer texture");
    }
}

void window_size_changed (IVec new_size) {
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

AYU_DESCRIBE(vf::WipeProgram,
    delegate(base<glow::Program>())
)
