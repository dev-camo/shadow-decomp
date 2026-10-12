#include "Game/fn_8040_vector_types.h"

extern float lbl_805FAD80;

void fn_8040D02C(void) {
    float initial_value = lbl_805FAD80;
    Fn8040Vector3 *vector = &lbl_805E07A0;

    vector->x = initial_value;
    vector->y = initial_value;
    vector->z = initial_value;
}

#pragma force_active on
#pragma section ".ctors$50"
__declspec(section ".ctors$50") extern void (*const fn_8040D02C_reference)(void) = fn_8040D02C;
#pragma force_active reset
