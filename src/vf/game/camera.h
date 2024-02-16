#pragma once
#include "common.h"

namespace vf {

constexpr Vec camera_size = {320, 180};
inline IRect window_viewport = {0, 0, 1280, 720};
constexpr uint32 transition_duration = 30;

template <class T>
T world_to_screen (const T& world) {
    return Rect(round(world)) / (camera_size / 2) - Vec(1, 1);
}

void begin_camera ();
void end_camera ();

enum class TransitionType {
    WipeLeft = 0,
    WipeRight = 1,
    WipeUp = 2,
    WipeDown = 3,
    ApertureClose = 4,
    ApertureOpen = 5
};

 // center is only used for apertures.  Center is in world coordinates.
void set_transition_center (Vec center);
void set_transition_type (TransitionType type);
 // Set to GNAN to disable
void set_transition_t (float t);
 // Depends on transition type
void set_transition_side (bool side);
 // You must do this before the area after the wipe is visible.
void swap_world_tex ();

void window_size_changed (IVec new_size);

} // vf
