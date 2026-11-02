/* Distance between two vectors (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" float fn_8040D1B4(const Fn8040Vector3 *a, const Fn8040Vector3 *b) {
    Fn8040Vector3 difference;
    fn_8040CD74(&difference, a, b);
    return fn_8040CF50(&difference);
}
