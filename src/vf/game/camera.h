#pragma once
#include "common.h"

namespace vf {

constexpr Vec camera_size = {320, 180};
inline IRect window_viewport = {0, 0, 1280, 720};

template <class T>
T world_to_screen (const T& world) {
    return world / (camera_size / 2) - Vec(1, 1);
}

void begin_camera ();
void end_camera ();

 // Only four cardinal directions are implemented.
void start_transition (Vec direction);

void window_size_changed (IVec new_size);

} // vf
