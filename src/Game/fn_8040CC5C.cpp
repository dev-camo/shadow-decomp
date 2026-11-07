/* Two-component vector helpers and the static zero vector lbl_805F1358 (behaviour inferred; the
 * original function and file names are unknown). fn_8040CC5C and the static initialiser read the
 * file's 0.0f literal, fn_8040CC6C its 1e-6f literal. The functions are defined in reverse address
 * order: the compiler emits the last definition first. */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

/* The zero vector that fn_8040CC6C stores for a vector too short to normalise. The constructor
 * call puts it in .sbss and initialises it at startup (a __sinit_ function and a .ctors entry). */
Fn8040Vector2 lbl_805F1358(0.0f, 0.0f);

extern "C" {

/* In-place addition. */
void fn_8040CD3C(Fn8040Vector2 *left, const Fn8040Vector2 *right) {
    left->x = left->x + right->x;
    left->y = left->y + right->y;
}

/* Dot product. */
float fn_8040CD20(Fn8040Vector2 *left, Fn8040Vector2 *right) {
    return left->x * right->x + left->y * right->y;
}

/* Length. */
float fn_8040CCEC(const Fn8040Vector2 *vector) {
    return std::sqrt(vector->x * vector->x + vector->y * vector->y);
}

/* Squared length. */
float fn_8040CCD4(Fn8040Vector2 *vector) {
    return vector->x * vector->x + vector->y * vector->y;
}

/* Normalises the vector in place, or replaces it by the zero vector lbl_805F1358 when its length is
 * below 1e-6. */
void fn_8040CC6C(Fn8040Vector2 *vector) {
    float magnitude = fn_8040CCEC(vector);

    if (magnitude < 1e-6f) {
        vector->x = lbl_805F1358.x;
        vector->y = lbl_805F1358.y;
    } else {
        vector->x /= magnitude;
        vector->y /= magnitude;
    }
}

/* Clears the vector. */
void fn_8040CC5C(Fn8040Vector2 *vector) {
    float zero = 0.0f;

    vector->y = zero;
    vector->x = zero;
}
}
