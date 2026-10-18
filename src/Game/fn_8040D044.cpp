#include "Game/fn_8040_vector_types.h"

extern "C" {
void fn_8040D044(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);

    /* sine and cosine have their addresses taken, so using them directly reloads both after the
     * store to result->x; the original loads each once, as these copies do. */
    float sine_value = sine;
    float cosine_value = cosine;
    float x = vector->x;
    float y = vector->y;
    float z = vector->z;

    result->x = x * cosine_value - y * sine_value;
    result->y = x * sine_value + y * cosine_value;
    result->z = z;
}
}
