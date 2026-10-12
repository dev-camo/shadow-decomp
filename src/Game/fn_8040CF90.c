#include "Game/fn_8040_vector_types.h"

float fn_8040CF90(const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    return left->x * right->x + left->y * right->y + left->z * right->z;
}

void fn_8040CFB8(Fn8040Vector3 *result, const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    result->x = left->y * right->z - left->z * right->y;
    result->y = left->z * right->x - left->x * right->z;
    result->z = left->x * right->y - left->y * right->x;
}

void fn_8040CFF8(Fn8040Vector3 *result, const Fn8040Vector3 *value) {
    result->x += value->x;
    result->y += value->y;
    result->z += value->z;
}
