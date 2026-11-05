/* Three-component vector arithmetic (behaviour inferred; original names and filename unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" {
Fn8040Vector3 fn_8040CD74(const Fn8040Vector3 *a, const Fn8040Vector3 *b) {
    return Fn8040Vector3(a->x - b->x, a->y - b->y, a->z - b->z);
}

Fn8040Vector3 fn_8040CDA8(const Fn8040Vector3 *vector, float factor) {
    return Fn8040Vector3(vector->x * factor, vector->y * factor, vector->z * factor);
}

Fn8040Vector3 fn_8040CDD0(const Fn8040Vector3 *left, const Fn8040Vector3 *right) {
    return Fn8040Vector3(left->x + right->x, left->y + right->y, left->z + right->z);
}

void fn_8040CE04(Fn8040Vector3 *vector, const Fn8040Vector3 *value) {
    vector->x -= value->x;
    vector->y -= value->y;
    vector->z -= value->z;
}

void fn_8040CE38(Fn8040Vector3 *vector, float scale) {
    vector->x *= scale;
    vector->y *= scale;
    vector->z *= scale;
}
}
