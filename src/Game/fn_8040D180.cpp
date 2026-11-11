#include "Game/fn_8040_vector_types.h"

extern "C" float fn_8040D180(const Fn8040Vector3 *a, const Fn8040Vector3 *b) {
    const Fn8040Vector3 &difference = *a - *b;
    return fn_8040CF2C(difference);
}
