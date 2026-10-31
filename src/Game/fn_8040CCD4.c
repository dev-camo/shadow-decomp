/* Squared length of a 2-float vector (behaviour inferred; the original name is unknown). */
#include "Game/fn_8040_vector_types.h"

float fn_8040CCD4(Fn8040Vector2 *vector) {
    return vector->x * vector->x + vector->y * vector->y;
}
