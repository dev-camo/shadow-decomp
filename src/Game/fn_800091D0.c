/* Two identical functions that copy a 3-float vector and return the destination (behaviour
 * inferred), at 0x800091D0 and 0x800091EC; the original names are unknown. Some callers pass the
 * result on, such as the first's to a second copy and the second's to fn_8040CF50. */
#include "Game/fn_8040_vector_types.h"

Fn8040Vector3 *fn_800091D0(Fn8040Vector3 *destination, const Fn8040Vector3 *source) {
    destination->x = source->x;
    destination->y = source->y;
    destination->z = source->z;
    return destination;
}

Fn8040Vector3 *fn_800091EC(Fn8040Vector3 *destination, const Fn8040Vector3 *source) {
    destination->x = source->x;
    destination->y = source->y;
    destination->z = source->z;
    return destination;
}
