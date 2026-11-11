/* Three-component vector helpers and the static zero vector lbl_805E07A0 (behaviour inferred; the
 * original function and file names are unknown). fn_8040CE60, fn_8040CE74 and the static
 * initialiser read the file's 0.0f literal, fn_8040CE74 also its 1e-6f literal. The functions are
 * defined in reverse address order: the compiler emits the last definition first. */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

/* The zero vector that fn_8040CE74 stores for a vector too short to normalise. The constructor
 * call puts it in .bss and initialises it at startup (a __sinit_ function and a .ctors entry). */
Fn8040Vector3 lbl_805E07A0(0.0f, 0.0f, 0.0f);

extern "C" {

/* In-place addition. */
void fn_8040CFF8(Fn8040Vector3 *result, const Fn8040Vector3 *value) {
    result->x += value->x;
    result->y += value->y;
    result->z += value->z;
}

/* Cross product. */
Fn8040Vector3 fn_8040CFB8(const Fn8040Vector3 &left, const Fn8040Vector3 &right) {
    return Fn8040Vector3(left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z,
                         left.x * right.y - left.y * right.x);
}

/* Dot product. */
float fn_8040CF90(const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    return left->x * right->x + left->y * right->y + left->z * right->z;
}

/* Length. The const reference is inferred from callers that pass a temporary copy (see the
 * declaration); the code is that of a pointer parameter. */
float fn_8040CF50(const Fn8040Vector3 &vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

/* Squared length. The const reference is inferred in the same way. */
float fn_8040CF2C(const Fn8040Vector3 &vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

/* Normalised copy. */
Fn8040Vector3 fn_8040CEE8(const Fn8040Vector3 &vector) {
    Fn8040Vector3 temporary = vector;
    fn_8040CE74(&temporary);
    return temporary;
}

/* Normalises the vector in place and returns its length, or replaces it by the zero vector
 * lbl_805E07A0 and returns 0.0f when its length is below 1e-6. */
float fn_8040CE74(Fn8040Vector3 *vector) {
    float magnitude = fn_8040CF50(*vector);

    if (magnitude < 1e-6f) {
        *vector = lbl_805E07A0;
        return 0.0f;
    }

    vector->x /= magnitude;
    vector->y /= magnitude;
    vector->z /= magnitude;
    return magnitude;
}

/* Clears the vector. */
void fn_8040CE60(Fn8040Vector3 *vector) {
    vector->z = 0.0f;
    vector->y = 0.0f;
    vector->x = 0.0f;
}

/* In-place scaling. */
void fn_8040CE38(Fn8040Vector3 *vector, float scale) {
    vector->x *= scale;
    vector->y *= scale;
    vector->z *= scale;
}

/* In-place subtraction. */
void fn_8040CE04(Fn8040Vector3 *vector, const Fn8040Vector3 *value) {
    vector->x -= value->x;
    vector->y -= value->y;
    vector->z -= value->z;
}

/* Sum by value. */
Fn8040Vector3 fn_8040CDD0(const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    return Fn8040Vector3(left->x + right->x, left->y + right->y, left->z + right->z);
}

/* Product with a scalar by value. */
Fn8040Vector3 fn_8040CDA8(const Fn8040Vector3 *vector, float factor) {
    return Fn8040Vector3(vector->x * factor, vector->y * factor, vector->z * factor);
}

/* Difference by value. */
Fn8040Vector3 fn_8040CD74(const Fn8040Vector3 *a, const Fn8040Vector3 *b) {
    return Fn8040Vector3(a->x - b->x, a->y - b->y, a->z - b->z);
}
}
