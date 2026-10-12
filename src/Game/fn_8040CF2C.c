#include "Game/fn_8040_vector_types.h"

#pragma fp_contract off
float fn_8040CF2C(Fn8040Vector3 *vector) {
    return vector->x * vector->x + vector->y * vector->y + vector->z * vector->z;
}
