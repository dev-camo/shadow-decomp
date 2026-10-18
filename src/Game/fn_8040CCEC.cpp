#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" {
float fn_8040CCEC(const Fn8040Vector2 *vector) {
    return std::sqrt(vector->x * vector->x + vector->y * vector->y);
}
}
