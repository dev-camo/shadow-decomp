/* Axis-angle rotation matrix (behaviour inferred; original name and filename unknown). */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" void fn_800498A0(Fn8040Matrix34 *matrix, const Fn8040Vector3 *axis, float angle) {
    float x = axis->x;
    float y = axis->y;
    float z = axis->z;
    float sine = std::sin(angle);
    float cosine = std::cos(angle);
    float oneMinusCosine = 1.0f - cosine;

    *fn_8040C17C(matrix, 0, 0) = oneMinusCosine * (x * x) + cosine;
    *fn_8040C17C(matrix, 1, 0) = oneMinusCosine * (x * y) + z * sine;
    *fn_8040C17C(matrix, 2, 0) = oneMinusCosine * (z * x) - y * sine;
    *fn_8040C17C(matrix, 0, 1) = oneMinusCosine * (x * y) - z * sine;
    *fn_8040C17C(matrix, 1, 1) = oneMinusCosine * (y * y) + cosine;
    *fn_8040C17C(matrix, 2, 1) = oneMinusCosine * (y * z) + x * sine;
    *fn_8040C17C(matrix, 0, 2) = oneMinusCosine * (z * x) + y * sine;
    *fn_8040C17C(matrix, 1, 2) = oneMinusCosine * (y * z) - x * sine;
    *fn_8040C17C(matrix, 2, 2) = oneMinusCosine * (z * z) + cosine;
    *fn_8040C17C(matrix, 0, 3) = 0.0f;
    *fn_8040C17C(matrix, 1, 3) = 0.0f;
    *fn_8040C17C(matrix, 2, 3) = 0.0f;
}
