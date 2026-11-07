/* Angle blend and matrix-to-angle helpers (behaviour inferred; original names and filename
 * unknown). */
#include "Game/SharedConstants.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"

extern "C" {

void fn_800494F8(UnkRotation3 *angles, const Fn8040Matrix34 *matrix) {
    float sine;
    float cosine;
    float initialAngle = fn_80016260(matrix->values[0][2], matrix->values[2][2]);
    angles->y = initialAngle;
    fn_80031568(&sine, &cosine, initialAngle);

    float angle;
    if (fn_8000A980(matrix->values[2][2]) > fn_8000A980(matrix->values[0][2])) {
        angle = fn_80016260(-matrix->values[1][2] * cosine, matrix->values[2][2]);
        if (cosine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    } else {
        angle = fn_80016260(-matrix->values[1][2] * sine, matrix->values[0][2]);
        if (sine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    }
    angles->x = angle;
    angles->z = fn_80016260(matrix->values[2][1] * sine - matrix->values[0][1] * cosine,
                            matrix->values[0][0] * cosine - matrix->values[2][0] * sine);
}

void fn_800493C4(UnkRotation3 *angles, const Fn8040Matrix34 *matrix) {
    float sine;
    float cosine;
    float initialAngle = fn_80016260(-matrix->values[0][1], matrix->values[1][1]);
    angles->z = initialAngle;
    fn_80031568(&sine, &cosine, initialAngle);

    float angle;
    if (fn_8000A980(matrix->values[1][1]) > fn_8000A980(matrix->values[0][1])) {
        angle = fn_80016260(matrix->values[2][1] * cosine, matrix->values[1][1]);
        if (cosine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    } else {
        angle = fn_80016260(matrix->values[2][1] * sine, -matrix->values[0][1]);
        if (sine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    }
    angles->x = angle;
    angles->y = fn_80016260(matrix->values[0][2] * cosine + matrix->values[1][2] * sine,
                            matrix->values[0][0] * cosine + matrix->values[1][0] * sine);
}

void fn_80049290(UnkRotation3 *angles, const Fn8040Matrix34 *matrix) {
    float sine;
    float cosine;
    float initialAngle = fn_80016260(matrix->values[1][0], matrix->values[0][0]);
    angles->z = initialAngle;
    fn_80031568(&sine, &cosine, initialAngle);

    float angle;
    if (fn_8000A980(matrix->values[0][0]) > fn_8000A980(matrix->values[1][0])) {
        angle = fn_80016260(-matrix->values[2][0] * cosine, matrix->values[0][0]);
        if (cosine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    } else {
        angle = fn_80016260(-matrix->values[2][0] * sine, matrix->values[1][0]);
        if (sine < 0.0f) {
            angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    }
    angles->y = angle;
    angles->x = fn_80016260(matrix->values[0][2] * sine - matrix->values[1][2] * cosine,
                            matrix->values[1][1] * cosine - matrix->values[0][1] * sine);
}

float fn_8004924C(float from, float to, float a, float b) {
    float difference = to - from;
    if (difference < -lbl_805FAD38) {
        difference += 2.0f * lbl_805FAD38;
    } else if (difference > lbl_805FAD38) {
        difference -= 2.0f * lbl_805FAD38;
    }
    return from + difference * a / (a + b);
}

} // extern "C"
