/* 3-float vector dot product, cross product and in-place addition (behaviour inferred; the
 * original names are unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" {
float fn_8040CF90(const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    return left->x * right->x + left->y * right->y + left->z * right->z;
}

void fn_8040CFB8(Fn8040Vector3 *result, const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    *result = Fn8040Vector3(left->y * right->z - left->z * right->y,
                            left->z * right->x - left->x * right->z,
                            left->x * right->y - left->y * right->x);
}

void fn_8040CFF8(Fn8040Vector3 *result, const Fn8040Vector3 *value) {
    result->x += value->x;
    result->y += value->y;
    result->z += value->z;
}
}
