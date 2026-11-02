/* Quaternion product (inferred). */
#include "Game/fn_8040C908.h"

extern "C" Fn8040Vector4 fn_8040CA10(const Fn8040Vector4 *left, const Fn8040Vector4 *right) {
    return Fn8040Vector4(
        left->w * right->x + left->x * right->w + left->y * right->z - left->z * right->y,
        left->w * right->y + left->y * right->w + left->z * right->x - left->x * right->z,
        left->w * right->z + left->z * right->w + left->x * right->y - left->y * right->x,
        left->w * right->w - left->x * right->x - left->y * right->y - left->z * right->z);
}
