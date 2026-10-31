#include "Game/fn_8040_vector_types.h"

extern void *lbl_805F13A8;
extern Fn8040Matrix34 *fn_80411700(void *owner);

void fn_8041157C(Fn8040Vector3 *destination, const Fn8040Matrix34 *matrix,
                 const Fn8040Vector3 *vector) {
    Fn8040Vector3 output;
    Fn8040Matrix34 matrix_copy;

    if (matrix == 0) {
        fn_8040C148(&matrix_copy, fn_80411700(lbl_805F13A8));
        matrix = &matrix_copy;
    }

    output.x = vector->x * matrix->values[0][0] + vector->y * matrix->values[0][1] +
               vector->z * matrix->values[0][2];
    output.y = vector->x * matrix->values[1][0] + vector->y * matrix->values[1][1] +
               vector->z * matrix->values[1][2];
    output.z = vector->x * matrix->values[2][0] + vector->y * matrix->values[2][1] +
               vector->z * matrix->values[2][2];

    fn_800091D0(destination, &output);
}
