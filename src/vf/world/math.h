#pragma once
#include "walker.h"

namespace vf {

inline float ease_out_sin (float t) {
    return std::sin(t * float(M_PI / 2));
}
inline float ease_in_sin (float t) {
    return 1 - std::sin((1 - t) * float(M_PI / 2));
}
inline float ease_in_out_sin (float t) {
    return (1 + std::sin((-1 + t*2) * float(M_PI / 2))) / 2;
}
inline float ease_in_quadratic (float t) {
    return t*t; // This one's easy
}
inline float ease_out_quadratic (float t) {
    return 1-(1-t)*(1-t);
}
inline float ease_in_cubic (float t) {
    return t*t*t;
}
inline float ease_out_cubic (float t) {
    return 1-(1-t)*(1-t)*(1-t);
}

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

 // Finds t of the first zero after t=0, or GNAN if there are no roots or both
 // roots are negative.
inline float positive_quadratic_root (float p, float v, float a) {
    float disc = v*v - a*p*4;
    if (disc >= 0) {
        float s = root2(disc);
        float r0 = (-v + s) / (2*a);
        float r1 = (-v - s) / (2*a);
        if (r0 >= 0 && r0 <= r1) return r0;
        else if (r1 >= 0) return r1;
        else return GNAN;
    }
    else return GNAN;
}

} // vf
