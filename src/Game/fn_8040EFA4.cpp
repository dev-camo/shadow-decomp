/* Interpolation helpers for matrices, rotation vectors, positions and angles (the unit's original
 * file name is unknown). Function names stay fn_ placeholders; descriptions of behaviour are
 * inferred from the code. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). Locals that receive a call's result are declared in the order
 * that gives the original's stack layout. */
#include "Game/fn_8040_vector_types.h"

extern "C" {

/* Interpolates from one angle towards another by t, along the shorter way round: the difference
 * is wrapped into the range -pi to pi by fn_8040E99C (behaviour inferred). */
float fn_8040F314(float from, float to, float t) {
    float difference = fn_8040E99C(to - from);
    return from + difference * t;
}

/* Interpolates a position from one vector to another by t (0 to 1). */
void fn_8040F268(Fn8040Vector3 *result, const Fn8040Vector3 *from, const Fn8040Vector3 *to,
                 float t) {
    if (t <= 0.0f) {
        fn_800091EC(result, from);
    } else if (t >= 1.0f) {
        fn_800091EC(result, to);
    } else {
        Fn8040Vector3 step;
        Fn8040Vector3 difference;
        fn_8000AD64(&difference, to, from);
        fn_800091A4(&step, &difference, t);
        fn_8040CDD0(result, from, &step);
    }
}

/* Interpolates a rotation vector from one to another by t (0 to 1), through quaternions of the
 * given rotation order. */
Fn8040Vector3 fn_8040F16C(const Fn8040Vector3 *from, const Fn8040Vector3 *to, int order, float t) {
    if (t <= 0.0f) {
        return *from;
    }
    if (t >= 1.0f) {
        return *to;
    }

    Fn8040Vector4 fromRotation;
    Fn8040Vector4 toRotation;
    Fn8040Vector4 fromTemporary;
    Fn8040Vector4 toTemporary;
    fn_804102F4(&fromTemporary, from, order);
    fn_801B1DD4(&fromRotation, &fromTemporary);
    fn_804102F4(&toTemporary, to, order);
    fn_801B1DD4(&toRotation, &toTemporary);

    Fn8040Vector4 rotation;
    fn_8040C3EC(&rotation, &fromRotation, &toRotation, t);
    return fn_80410204(&rotation, order);
}

/* Interpolates a rotation matrix from one to another by t (0 to 1): the rotations through
 * quaternions, the translations linearly. The original stores each quaternion from fn_8040C55C in
 * a temporary and copies it with fn_801B1DD4. */
void fn_8040EFA4(Fn8040Matrix34 *result, Fn8040Matrix34 *from, Fn8040Matrix34 *to, float t) {
    if (t <= 0.0f) {
        fn_8040C148(result, from);
    } else if (t >= 1.0f) {
        fn_8040C148(result, to);
    } else {
        Fn8040Matrix34 interpolated;
        Fn8040Vector4 fromRotation;
        Fn8040Vector4 toRotation;
        Fn8040Vector4 fromTemporary;
        Fn8040Vector4 toTemporary;
        fn_8040C55C(&fromTemporary, from);
        fn_801B1DD4(&fromRotation, &fromTemporary);
        fn_8040C55C(&toTemporary, to);
        fn_801B1DD4(&toRotation, &toTemporary);

        Fn8040Vector4 rotation;
        fn_8040C3EC(&rotation, &fromRotation, &toRotation, t);

        Fn8040Matrix34 rotationMatrix;
        fn_8040B228(&rotationMatrix, &rotation);
        fn_8040C148(&interpolated, &rotationMatrix);

        float fromWeight = 1.0f - t;
        *fn_8040C17C(&interpolated, 0, 3) =
            fromWeight * fn_8040C168(from, 0, 3) + t * fn_8040C168(to, 0, 3);
        *fn_8040C17C(&interpolated, 1, 3) =
            fromWeight * fn_8040C168(from, 1, 3) + t * fn_8040C168(to, 1, 3);
        *fn_8040C17C(&interpolated, 2, 3) =
            fromWeight * fn_8040C168(from, 2, 3) + t * fn_8040C168(to, 2, 3);
        fn_8040C148(result, &interpolated);
    }
}
}
