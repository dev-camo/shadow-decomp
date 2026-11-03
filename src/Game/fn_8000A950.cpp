/* True when |value| < 0.0001 (behaviour inferred). */
#include "Game/fn_8040_vector_types.h"

extern "C" bool fn_8000A950(float value) {
    return fn_8000A980(value) < 0.0001f;
}
