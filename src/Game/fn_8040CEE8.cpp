/* Normalised vector copy (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" void fn_8040CEE8(Fn8040Vector3 *result, const Fn8040Vector3 *vector) {
    Fn8040Vector3 temporary;
    fn_800091EC(&temporary, vector);
    fn_8040CE74(&temporary);
    fn_800091EC(result, &temporary);
}
