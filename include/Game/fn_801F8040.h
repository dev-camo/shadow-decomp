#ifndef GAME_FN_801F8040_H
#define GAME_FN_801F8040_H

#include <cmath>

/* Sets *sine and *cosine to scale times the sine and cosine of angle (behaviour inferred). The
 * callers do not expand it: they call the copy at 0x801F8040 (the original name is unknown). */
inline void fn_801F8040(float *sine, float *cosine, float angle, float scale) {
    *sine = scale * std::sin(angle);
    *cosine = scale * std::cos(angle);
}

#endif
