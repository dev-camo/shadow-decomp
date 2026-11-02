/* Rotates a 3-float vector about the x or y axis by angle radians (behaviour inferred; the
 * original function names and filename are unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" {
void fn_8040D118(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);

    result->Set(vector->x, vector->y * cosine - vector->z * sine,
                vector->y * sine + vector->z * cosine);
}

void fn_8040D0AC(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);

    result->Set(vector->x * cosine + vector->z * sine, vector->y,
                -vector->x * sine + vector->z * cosine);
}
}
