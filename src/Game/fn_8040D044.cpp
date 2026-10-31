/* Rotates a 3-float vector about the z axis by angle radians (behaviour inferred from the sine
 * and cosine of the angle; the original name is unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" {
void fn_8040D044(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);

    *result = Fn8040Vector3(vector->x * cosine - vector->y * sine,
                            vector->x * sine + vector->y * cosine, vector->z);
}
}
