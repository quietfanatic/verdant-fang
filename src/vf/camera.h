#pragma once
#include "common.h"

namespace vf {

constexpr Vec camera_size_screen = {320, 180};
constexpr Vec camera_size_world = {20, 11.25};
inline Rect window_viewport = {0, 0, 640, 360};

template <class T>
T world_to_screen (const T& world) {
    return world / camera_size_world - Vec(1, 1);
}

void begin_camera ();
void end_camera ();

} // vf
