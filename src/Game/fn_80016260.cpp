/* Two-argument arctangent with a float result (behaviour inferred). */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" float fn_80016260(float y, float x) {
    return atan2(y, x);
}
