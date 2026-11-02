/* Squared length of a four-float vector (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040_vector_types.h"

float fn_8040CA84(const Fn8040Vector4 *vector) {
    return vector->x * vector->x + vector->y * vector->y + vector->z * vector->z +
           vector->w * vector->w;
}
