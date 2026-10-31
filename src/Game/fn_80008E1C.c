/* Sets the three components of a 3-float vector (behaviour inferred). The original names of the
 * function and the vector type are unknown. */
#include "Game/fn_8040_vector_types.h"

void fn_80008E1C(Fn8040Vector3 *vector, float x, float y, float z) {
    vector->x = x;
    vector->y = y;
    vector->z = z;
}
