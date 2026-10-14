/* RenderWare hierarchical animation (plugin ID 0x11E): the standard keyframe scheme, which turns a
 * keyframe into a matrix, interpolates and blends keyframes (spherical interpolation of their
 * rotations, linear of their translations), streams them and offsets them. The original file name
 * is unknown. */
#include "rw/rphanim.h"
#include "rw/rwmath.h"

/* sin(x) by its Taylor series to x^13 */
static inline RwReal RwSin(RwReal x) {
    RwReal x2 = x * x;

    return x + x2 * x *
                   (-0.16666667f +
                    x2 * (0.008333334f +
                          x2 * (-0.0001984127f +
                                x2 * (0.0000027557314f +
                                      x2 * (-0.00000002505076f + x2 * 0.0000000001589691f)))));
}

/* Interpolates the rotation (spherically) and translation (linearly) of two keyframes by t. When
 * the rotations are more than half a turn apart, in2's is negated in place first. */
#define HAnimKeyFrameLerpMacro(out, in1, in2, weight)                                              \
    do {                                                                                           \
        RwReal cosOmega;                                                                           \
        RwReal scale2;                                                                             \
        RwReal scale1;                                                                             \
                                                                                                   \
        cosOmega = (in1)->q.imag.x * (in2)->q.imag.x + (in1)->q.imag.y * (in2)->q.imag.y +         \
                   (in1)->q.imag.z * (in2)->q.imag.z + (in1)->q.real * (in2)->q.real;              \
                                                                                                   \
        (out)->t.x = (weight) * ((in2)->t.x - (in1)->t.x) + (in1)->t.x;                            \
        (out)->t.y = (weight) * ((in2)->t.y - (in1)->t.y) + (in1)->t.y;                            \
        (out)->t.z = (weight) * ((in2)->t.z - (in1)->t.z) + (in1)->t.z;                            \
        if (cosOmega < 0.0f) {                                                                     \
            cosOmega = -cosOmega;                                                                  \
            (in2)->q.imag.x = -(in2)->q.imag.x;                                                    \
            (in2)->q.imag.y = -(in2)->q.imag.y;                                                    \
            (in2)->q.imag.z = -(in2)->q.imag.z;                                                    \
            (in2)->q.real = -(in2)->q.real;                                                        \
        }                                                                                          \
                                                                                                   \
        scale1 = 1.0f - (weight);                                                                  \
        scale2 = (weight);                                                                         \
        if (!(cosOmega >= 0.999f)) { /* NaN takes this path too */                                 \
            RwReal omega;                                                                          \
            RwReal recipSinOmega;                                                                  \
                                                                                                   \
            RwACosMacro(omega, cosOmega);                                                          \
            scale1 *= omega;                                                                       \
            scale2 *= omega;                                                                       \
            recipSinOmega = 1.0f / RwSin(omega);                                                   \
            scale1 = RwSin(scale1) * recipSinOmega;                                                \
            scale2 = RwSin(scale2) * recipSinOmega;                                                \
        }                                                                                          \
                                                                                                   \
        (out)->q.imag.x = scale1 * (in1)->q.imag.x + scale2 * (in2)->q.imag.x;                     \
        (out)->q.imag.y = scale1 * (in1)->q.imag.y + scale2 * (in2)->q.imag.y;                     \
        (out)->q.imag.z = scale1 * (in1)->q.imag.z + scale2 * (in2)->q.imag.z;                     \
        (out)->q.real = scale1 * (in1)->q.real + scale2 * (in2)->q.real;                           \
    } while (0)

/* Writes a keyframe as a matrix: its rotation (a unit quaternion) and its translation */
void fn_80445CF4(RwMatrix *matrix, RpHAnimInterpFrame *frame) {
    RwReal x = frame->q.imag.x;
    RwReal y = frame->q.imag.y;
    RwReal z = frame->q.imag.z;
    RwReal w = frame->q.real;
    RwV3d square;
    RwV3d cross;
    RwV3d wimag;

    square.x = x * x;
    square.y = y * y;
    square.z = z * z;
    cross.x = y * z;
    cross.y = z * x;
    cross.z = x * y;
    wimag.x = w * x;
    wimag.y = w * y;
    wimag.z = w * z;

    matrix->right.x = 1.0f - 2.0f * (square.y + square.z);
    matrix->right.y = 2.0f * (cross.z + wimag.z);
    matrix->right.z = 2.0f * (cross.y - wimag.y);
    matrix->up.x = 2.0f * (cross.z - wimag.z);
    matrix->up.y = 1.0f - 2.0f * (square.x + square.z);
    matrix->up.z = 2.0f * (cross.x + wimag.x);
    matrix->at.x = 2.0f * (cross.y + wimag.y);
    matrix->at.y = 2.0f * (cross.x - wimag.x);
    matrix->at.z = 1.0f - 2.0f * (square.x + square.y);
    matrix->pos.x = 0.0f;
    matrix->pos.y = 0.0f;
    matrix->pos.z = 0.0f;
    matrix->flags = rwMATRIXTYPEORTHONORMAL;

    matrix->pos.x = frame->t.x;
    matrix->pos.y = frame->t.y;
    matrix->pos.z = frame->t.z;
}

/* Interpolates between two keyframes at time */
void fn_80445E08(RpHAnimInterpFrame *out, RpHAnimKeyFrame *in1, RpHAnimKeyFrame *in2, RwReal time,
                 void *customData) {
    RwReal t = (time - in1->time) / (in2->time - in1->time);

    HAnimKeyFrameLerpMacro(out, in1, in2, t);
}

/* Blends two interpolated frames by alpha */
void fn_8044621C(RpHAnimInterpFrame *out, RpHAnimInterpFrame *in1, RpHAnimInterpFrame *in2,
                 RwReal alpha) {
    HAnimKeyFrameLerpMacro(out, in1, in2, alpha);
}

/* Reads the keyframes of an animation: each is its time, rotation and translation, then the byte
 * offset of the frame it follows */
RtAnimAnimation *fn_80446620(RwStream *stream, RtAnimAnimation *animation) {
    RpHAnimKeyFrame *frame;
    RwInt32 i;
    RpHAnimKeyFrame *frames;

    frames = animation->pFrames;
    frame = frames;

    for (i = 0; i < animation->numFrames; i++, frame++) {
        RwUInt32 prevOffset;

        if (fn_8047D784(stream, &frame->time, 8 * sizeof(RwReal)) == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, (RwInt32 *)&prevOffset, sizeof(prevOffset)) == NULL) {
            return NULL;
        }
        frame->prevFrame = &frames[prevOffset / sizeof(RpHAnimKeyFrame)];
    }
    return animation;
}

/* Writes the keyframes of an animation */
RwBool fn_804466DC(const RtAnimAnimation *animation, RwStream *stream) {
    RwInt32 numFrames;
    RpHAnimKeyFrame *frame;
    RwInt32 i;
    RpHAnimKeyFrame *frames;

    frames = animation->pFrames;
    frame = frames;
    numFrames = animation->numFrames;
    for (i = 0; i < numFrames; frame++, i++) {
        RwInt32 prevOffset;

        if (fn_8047D350(stream, &frame->time, 8 * sizeof(RwReal)) == NULL) {
            return FALSE;
        }
        prevOffset = (RwUInt8 *)frame->prevFrame - (RwUInt8 *)frames;
        if (fn_8047D458(stream, &prevOffset, sizeof(prevOffset)) == NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

/* The stream size of the keyframes of an animation */
RwInt32 fn_80446780(const RtAnimAnimation *animation) {
    RwUInt32 size = sizeof(RpHAnimKeyFrame);

    size *= animation->numFrames;
    return size;
}

/* Makes a keyframe relative to start: its rotation becomes start^-1 * rotation and its translation
 * the offset from start's */
void fn_80446790(RpHAnimKeyFrame *frame, RpHAnimKeyFrame *start) {
    RtQuat recip;
    RtQuat q;
    RwReal norm = start->q.imag.x * start->q.imag.x + start->q.imag.y * start->q.imag.y +
                  start->q.imag.z * start->q.imag.z + start->q.real * start->q.real;

    if (norm > 0.0f) {
        RwReal recipNorm = 1.0f / norm;

        recip.real = start->q.real * recipNorm;
        recip.imag.x = start->q.imag.x * -recipNorm;
        recip.imag.y = start->q.imag.y * -recipNorm;
        recip.imag.z = start->q.imag.z * -recipNorm;
    }

    q = frame->q;
    frame->q.real = recip.real * q.real -
                    (recip.imag.x * q.imag.x + recip.imag.y * q.imag.y + recip.imag.z * q.imag.z);
    frame->q.imag.x = recip.imag.y * q.imag.z - recip.imag.z * q.imag.y;
    frame->q.imag.y = recip.imag.z * q.imag.x - recip.imag.x * q.imag.z;
    frame->q.imag.z = recip.imag.x * q.imag.y - recip.imag.y * q.imag.x;
    frame->q.imag.x += q.imag.x * recip.real;
    frame->q.imag.y += q.imag.y * recip.real;
    frame->q.imag.z += q.imag.z * recip.real;
    frame->q.imag.x += recip.imag.x * q.real;
    frame->q.imag.y += recip.imag.y * q.real;
    frame->q.imag.z += recip.imag.z * q.real;

    frame->t.x -= start->t.x;
    frame->t.y -= start->t.y;
    frame->t.z -= start->t.z;
}

/* Adds two keyframes: out = in1 * in2 */
void fn_804468D0(RpHAnimKeyFrame *out, RpHAnimKeyFrame *in1, RpHAnimKeyFrame *in2) {
    out->q.real =
        in1->q.real * in2->q.real - (in1->q.imag.x * in2->q.imag.x + in1->q.imag.y * in2->q.imag.y +
                                     in1->q.imag.z * in2->q.imag.z);
    out->q.imag.x = in1->q.imag.y * in2->q.imag.z - in1->q.imag.z * in2->q.imag.y;
    out->q.imag.y = in1->q.imag.z * in2->q.imag.x - in1->q.imag.x * in2->q.imag.z;
    out->q.imag.z = in1->q.imag.x * in2->q.imag.y - in1->q.imag.y * in2->q.imag.x;
    out->q.imag.x += in2->q.imag.x * in1->q.real;
    out->q.imag.y += in2->q.imag.y * in1->q.real;
    out->q.imag.z += in2->q.imag.z * in1->q.real;
    out->q.imag.x += in1->q.imag.x * in2->q.real;
    out->q.imag.y += in1->q.imag.y * in2->q.real;
    out->q.imag.z += in1->q.imag.z * in2->q.real;

    out->t.x = in1->t.x + in2->t.x;
    out->t.y = in1->t.y + in2->t.y;
    out->t.z = in1->t.z + in2->t.z;
}
