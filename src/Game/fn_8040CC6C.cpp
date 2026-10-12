#include "Game/fn_8040_vector_types.h"

extern "C" {
extern float lbl_805FAD7C;
}

extern "C" void fn_8040CC6C(Fn8040Vector2 *vector) {
    float magnitude = fn_8040CCEC(vector);

    if (magnitude < lbl_805FAD7C) {
        vector->x = lbl_805F1358.x;
        vector->y = lbl_805F1358.y;
    } else {
        vector->x /= magnitude;
        vector->y /= magnitude;
    }
}
