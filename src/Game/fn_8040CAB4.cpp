/* Four-component in-place addition (behaviour inferred). */
#include "Game/fn_8040CAB4.h"

void fn_8040CAB4(Fn8040Vector4 *destination, const Fn8040Vector4 *input) {
    destination->x += input->x;
    destination->y += input->y;
    destination->z += input->z;
    destination->w += input->w;
}
