#include "Game/fn_8040_vector_types.h"

float fn_8040CD20(Fn8040Vector2 *left, Fn8040Vector2 *right) {
    return left->x * right->x + left->y * right->y;
}

void fn_8040CD3C(Fn8040Vector2 *left, const Fn8040Vector2 *right) {
    left->x = left->x + right->x;
    left->y = left->y + right->y;
}
