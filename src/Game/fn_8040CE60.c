/* Clears a three-component vector (behaviour inferred; original name unknown). */
#include "Game/fn_8040_vector_types.h"

void fn_8040CE60(Fn8040Vector3 *vector) {
    vector->z = 0.0f;
    vector->y = 0.0f;
    vector->x = 0.0f;
}
