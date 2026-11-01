/* Control of motions: fn_8042043C to the static initializer fn_804206A4 (see
 * Game/fn_804157E8_motion_control.h for the file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* fn_8042043C and fn_804204C8 read the curves through a local pointer.
 * Matching workaround: the same form as fn_80420308 and fn_804203AC, for the reason given in
 * Game/fn_80420308.cpp. */
void fn_8042043C(MotionReflectedVectorTrack *track) {
    MotionKeyCursor *curves = track->curves;

    if (curves[0].position != 0) {
        track->value.x = lbl_805F13D0(&curves[0]);
    }
    if (curves[1].position != 0) {
        track->value.y = lbl_805F13D0(&curves[1]);
    }
    if (curves[2].position != 0) {
        track->value.z = lbl_805F13D0(&curves[2]);
    }
}

void fn_804204C8(MotionScaledVectorTrack *track) {
    MotionKeyCursor *curves = track->curves;

    if (curves[0].position != 0) {
        track->value.x = lbl_805F13D0(&curves[0]);
    }
    if (curves[1].position != 0) {
        track->value.y = lbl_805F13D0(&curves[1]);
    }
    if (curves[2].position != 0) {
        track->value.z = lbl_805F13D0(&curves[2]);
    }
    if (track->scale > 0.0f) {
        track->value.x *= track->scale;
        track->value.y *= track->scale;
        track->value.z *= track->scale;
    }
}

void fn_80420590(Fn8040Matrix34 *result, const Fn8040Matrix34 *a, const Fn8040Matrix34 *b,
                 float t) {
    Fn8040Matrix34 blended;

    fn_8042062C(&blended, a, b, t);
    blended.values[0][3] = t * a->values[0][3] + (1.0f - t) * b->values[0][3];
    blended.values[1][3] = t * a->values[1][3] + (1.0f - t) * b->values[1][3];
    blended.values[2][3] = t * a->values[2][3] + (1.0f - t) * b->values[2][3];
    fn_8040C148(result, &blended);
}

/* Blends two matrices: their rotations as quaternions, interpolated, then normalised to unit
 * columns. */
void fn_8042062C(Fn8040Matrix34 *result, const Fn8040Matrix34 *a, const Fn8040Matrix34 *b,
                 float t) {
    Fn8040Vector4 blended;
    Fn8040Vector4 rotationB;
    Fn8040Vector4 rotationA;

    fn_804208EC(&rotationA, a);
    fn_804208EC(&rotationB, b);
    fn_80420724(&blended, &rotationA, &rotationB, t);
    fn_80420B60(result, &blended);
    fn_8041116C(result, result);
}
