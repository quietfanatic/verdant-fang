#include "camera.h"

#include "../../dirt/glow/gl.h"
#include "../../dirt/glow/texture.h"
#include "frame.h"

namespace vf {

static glow::Texture world_tex;
static GLuint world_fb = 0;

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

static void setup_camera () {
    world_tex = glow::Texture(GL_TEXTURE_RECTANGLE);
    glTexImage2D(
        GL_TEXTURE_RECTANGLE, 0, GL_RGBA,
        camera_size.x, camera_size.y,
        0, GL_RGBA, GL_UNSIGNED_BYTE, 0
    );
     // Filtering mode for entire screen
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    glGenFramebuffers(1, &world_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, world_tex, 0
    );
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        raise(e_General, "Failed to set up render to texture");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void begin_camera () {
    if (!world_fb) setup_camera();
    glBindFramebuffer(GL_FRAMEBUFFER, world_fb);
    glViewport(0, 0, camera_size.x, camera_size.y);
    glEnable(GL_BLEND);
}

void end_camera () {
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(
        window_viewport.l, window_viewport.b,
        width(window_viewport), height(window_viewport)
    );
     // By changing viewport, this will now zoom.
    Frame frame {{0, 0}, {Vec(0, 0), camera_size}};
    draw_frame({0, 0}, frame, world_tex);
}

} // vf
