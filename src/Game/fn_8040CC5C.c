/* Clears a 2-float vector to the 0.0f literal in .sdata2 (behaviour inferred; the original name is
 * unknown). */
#include "Game/fn_8040_vector_types.h"

void fn_8040CC5C(Fn8040Vector2 *vector) {
    float zero = lbl_805FAD78;

    vector->y = zero;
    vector->x = zero;
}
