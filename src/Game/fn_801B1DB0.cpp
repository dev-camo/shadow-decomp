/* Arc-cosine wrapper with a float result (behaviour inferred). */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" float fn_801B1DB0(float value) {
    return acos(value);
}
