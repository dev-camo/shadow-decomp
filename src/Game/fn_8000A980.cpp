/* Float absolute-value wrapper (behaviour inferred). */
#include "Game/fn_8040_vector_types.h"

#include <cmath>

extern "C" float fn_8000A980(float value) {
    return fabsf(value);
}
