#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" {
float fn_8040CF50(const Fn8040Vector3 *vector) {
    return std::sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);
}
}
