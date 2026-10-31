/* Normalises a 2-float vector in place, or replaces it by the zero vector lbl_805F1358 when its
 * length is below 1e-6 (behaviour inferred; the original name is unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" void fn_8040CC6C(Fn8040Vector2 *vector) {
    float magnitude = fn_8040CCEC(vector);

    if (magnitude < lbl_805FAD7C) {
        vector->x = lbl_805F1358.x;
        vector->y = lbl_805F1358.y;
    } else {
        vector->x /= magnitude;
        vector->y /= magnitude;
    }
}
