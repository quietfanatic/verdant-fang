#pragma once
#include "walker.h"

namespace vf {


inline float quadratic (float p, float v, float a, float t) {
    return p + v*t + a*(t*t);
}

 // Evaluate quadratic, but stopping when velocity reaches 0 instead of going
 // backwards.
inline float quadratic_until_stop (float p, float v, float a, float t) {
     // 0 = v + at
     // -v = at
     // -v/a = t
    if (a) {
        float peak_t = -(v / a);
        if (peak_t >= 0 && t > peak_t) t = peak_t;
    }
    return p + v*t + a*(t*t);
}

} // vf
