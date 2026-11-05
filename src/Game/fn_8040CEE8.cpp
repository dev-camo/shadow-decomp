/* Normalised vector copy (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" Fn8040Vector3 fn_8040CEE8(const Fn8040Vector3 &vector) {
    Fn8040Vector3 temporary = vector;
    fn_8040CE74(&temporary);
    return temporary;
}
