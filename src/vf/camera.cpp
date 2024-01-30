#include "camera.h"

#include "../dirt/glow/gl.h"
#include "../dirt/glow/texture-program.h"

namespace vf {

static glow::Texture world_tex;
static GLuint world_fb = 0;

static void setup_rtt () {
    world_tex = glow::Texture(GL_TEXTURE_2D);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        camera_size_screen.x, camera_size_screen.y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0
    );
     // Filtering mode for screen.  May want to reconsider this.
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
}

void begin_camera () {
    if (!world_fb) setup_rtt();
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glViewport(0, 0, camera_size_screen.x, camera_size_screen.y);
}

void end_camera () {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(
        window_viewport.l, window_viewport.b,
        width(window_viewport), height(window_viewport)
    );
    glow::draw_texture(world_tex, Rect(-1, -1, 1, 1));
}

} // vf
