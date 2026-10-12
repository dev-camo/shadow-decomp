#include "Game/fn_8040_vector_types.h"

extern "C" {
extern float lbl_805FAD84;

float fn_8040CE74(Fn8040Vector3 *vector) {
    float magnitude = fn_8040CF50(vector);

    if (magnitude < lbl_805FAD84) {
        fn_800091D0(vector, &lbl_805E07A0);
        return lbl_805FAD80;
    }

    vector->x /= magnitude;
    vector->y /= magnitude;
    vector->z /= magnitude;
    return magnitude;
}
}
