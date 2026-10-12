#include "Game/fn_8040_vector_types.h"

void fn_80411548(Fn8040Vector3 *destination, const Fn8040Vector3 *left,
                 const Fn8040Vector3 *right) {
    destination->x = left->x + right->x;
    destination->y = left->y + right->y;
    destination->z = left->z + right->z;
}
