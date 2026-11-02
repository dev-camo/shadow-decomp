/* If squared length >= 1e-5, scales each component by 1.0f / sqrt(squared length);
 * otherwise sets all components to zero (behaviour inferred). */

#include <cmath>

#include "Game/fn_8040C908.h"

extern "C" void fn_8040C958(Fn8040Vector4 *value) {
    float squared =
        value->x * value->x + value->y * value->y + value->z * value->z + value->w * value->w;
    if (squared >= 1.0e-5f) {
        float inverse = 1.0f / std::sqrt(squared);
        value->x *= inverse;
        value->y *= inverse;
        value->z *= inverse;
        value->w *= inverse;
    } else {
        value->x = value->y = value->z = value->w = 0.0f;
    }
}
