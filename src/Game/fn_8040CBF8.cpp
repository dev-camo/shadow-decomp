/* Two-component vector arithmetic (behaviour inferred; original names and filename unknown). */
#include "Game/fn_8040CBF8.h"

extern "C" {
void fn_8040CBF8(Fn8040Vector2 *result, const Fn8040Vector2 *left, const Fn8040Vector2 *right) {
    *result = Fn8040Vector2(left->x - right->x, left->y - right->y);
}

void fn_8040CC1C(Fn8040Vector2 *result, const Fn8040Vector2 *vector, float factor) {
    *result = Fn8040Vector2(vector->x * factor, vector->y * factor);
}

void fn_8040CC38(Fn8040Vector2 *result, const Fn8040Vector2 *left, const Fn8040Vector2 *right) {
    *result = Fn8040Vector2(left->x + right->x, left->y + right->y);
}
}
