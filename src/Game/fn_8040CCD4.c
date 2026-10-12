#include "Game/fn_8040_vector_types.h"

#pragma fp_contract off
float fn_8040CCD4(Fn8040Vector2 *vector) {
    return vector->x * vector->x + vector->y * vector->y;
}
