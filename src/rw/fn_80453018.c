/* RenderWare UV animation plugin (plugin ID 0x135): setting keyframes from matrices, the keyframe
 * stream callbacks both schemes share, and the linear keyframe scheme. The original file name is
 * unknown. */
#include "rw/rpuvanim.h"

/* Sets a keyframe to a transform matrix in the animation's scheme. The original returns FALSE on
 * every path, whether or not the matrix conversion succeeds. */
RwBool fn_80453018(const RtAnimAnimation *animation, RpUVAnimKeyFrame *keyFrame,
                   RpUVAnimKeyFrame *prevFrame, RwReal time, const RwMatrix *matrix) {
    keyFrame->prevFrame = prevFrame;
    keyFrame->time = time;
    if (animation->interpInfo->typeID == lbl_8056D77C.typeID &&
        fn_80453594(&keyFrame->data.linear, matrix) != NULL) {
        return FALSE;
    }
    if (animation->interpInfo->typeID == lbl_8056D7AC.typeID &&
        fn_80453820(&keyFrame->data.param, matrix) != NULL) {
        return FALSE;
    }
    return FALSE;
}

/* Reads an animation's keyframes: the custom data, then each keyframe's time and data and the index
 * of the keyframe it follows */
RtAnimAnimation *fn_804530CC(RwStream *stream, RtAnimAnimation *animation) {
    RwInt32 flags;
    RwInt32 prevIndex;
    RpUVAnimCustomData *customData;
    RpUVAnimKeyFrame *frame;
    RwInt32 i;
    RpUVAnimKeyFrame *frames;

    if (fn_8047D868(stream, &flags, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    customData = fn_80454374(stream);
    if (customData == NULL) {
        return NULL;
    }
    animation->customData = customData;

    frames = animation->pFrames;
    frame = frames;
    for (i = 0; i < animation->numFrames; i++) {
        /* the time and the six reals of the data */
        if (fn_8047D784(stream, &frame->time, sizeof(RwReal) * 7) == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, &prevIndex, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        frame->prevFrame = &frames[prevIndex];
        frame++;
    }
    return animation;
}

/* Writes an animation's keyframes */
RwBool fn_804531B0(const RtAnimAnimation *animation, RwStream *stream) {
    RwInt32 flags = 0;
    RwInt32 prevIndex;
    RwInt32 numFrames;
    RpUVAnimKeyFrame *frame;
    RwInt32 i;
    RpUVAnimKeyFrame *frames;

    if (fn_8047D458(stream, &flags, sizeof(RwInt32)) == NULL) {
        return FALSE;
    }
    if (fn_8045441C(animation->customData, stream) == NULL) {
        return FALSE;
    }

    frames = animation->pFrames;
    numFrames = animation->numFrames;
    frame = frames;
    for (i = 0; i < numFrames; i++) {
        if (fn_8047D350(stream, &frame->time, sizeof(RwReal) * 7) == NULL) {
            return FALSE;
        }
        prevIndex = frame->prevFrame - frames;
        if (fn_8047D458(stream, &prevIndex, sizeof(RwInt32)) == NULL) {
            return FALSE;
        }
        frame++;
    }
    return TRUE;
}

/* The stream size of an animation's keyframes */
RwInt32 fn_804532A4(const RtAnimAnimation *animation) {
    RwInt32 size;

    size = fn_80454494(animation->customData) + sizeof(RwInt32);
    size += animation->numFrames * sizeof(RpUVAnimKeyFrame);
    return size;
}

void fn_804532E4(void *result, void *voidIFrame) {
    UVAnimLinearKeyFrameApply(result, voidIFrame);
}

void fn_80453350(void *voidOut, void *voidIn1, void *voidIn2, RwReal time, void *customData) {
    RpUVAnimInterpFrame *out = voidOut;
    RpUVAnimKeyFrame *in1 = voidIn1;
    RpUVAnimKeyFrame *in2 = voidIn2;
    RwReal alpha = (time - in1->time) / (in2->time - in1->time);
    RwInt32 i;

    for (i = 0; i < 6; i++) {
        out->data.linear.uv[i] =
            in1->data.linear.uv[i] + alpha * (in2->data.linear.uv[i] - in1->data.linear.uv[i]);
    }
}

void fn_804533E0(void *voidOut, void *voidIn1, void *voidIn2, RwReal alpha) {
    RpUVAnimInterpFrame *out = voidOut;
    RpUVAnimInterpFrame *in1 = voidIn1;
    RpUVAnimInterpFrame *in2 = voidIn2;
    RwInt32 i;

    for (i = 0; i < 6; i++) {
        out->data.linear.uv[i] =
            in1->data.linear.uv[i] + alpha * (in2->data.linear.uv[i] - in1->data.linear.uv[i]);
    }
}

/* Takes the start keyframe's transform out of a keyframe: multiplies by its inverse 2x2 matrix
 * and subtracts its translation */
void fn_8045345C(void *voidFrame, void *voidStart) {
    RpUVAnimKeyFrame *frame = voidFrame;
    RpUVAnimKeyFrame *start = voidStart;
    RwReal *uv = frame->data.linear.uv;
    const RwReal *startUV = start->data.linear.uv;
    RwReal determinant = startUV[0] * startUV[3] - startUV[1] * startUV[2];

    if (determinant != 0.0f) {
        RwReal recip = 1.0f / determinant;
        RwReal inverse0 = startUV[3] * recip;
        RwReal inverse1 = -startUV[1] * recip;
        RwReal inverse2 = -startUV[2] * recip;
        /* as in the original, the inverse's last element repeats the third instead of using
         * startUV[0] */
        RwReal inverse3 = -startUV[2] * recip;
        RwReal uv0 = uv[0];
        RwReal uv1 = uv[1];
        RwReal uv2 = uv[2];
        RwReal uv3 = uv[3];

        uv[0] = inverse0 * uv0 + inverse1 * uv2;
        uv[1] = inverse0 * uv1 + inverse1 * uv3;
        uv[2] = inverse2 * uv0 + inverse3 * uv2;
        uv[3] = inverse2 * uv1 + inverse3 * uv3;
    }
    uv[4] -= startUV[4];
    uv[5] -= startUV[5];
}

/* Applies a keyframe's transform after an interpolated frame's */
void fn_80453500(void *voidOut, void *voidIn1, void *voidIn2) {
    RpUVAnimInterpFrame *out = voidOut;
    RpUVAnimInterpFrame *in1 = voidIn1;
    RpUVAnimKeyFrame *in2 = voidIn2;

    out->data.linear.uv[0] = in1->data.linear.uv[0] * in2->data.linear.uv[0] +
                             in1->data.linear.uv[1] * in2->data.linear.uv[2];
    out->data.linear.uv[1] = in1->data.linear.uv[0] * in2->data.linear.uv[1] +
                             in1->data.linear.uv[1] * in2->data.linear.uv[3];
    out->data.linear.uv[2] = in1->data.linear.uv[2] * in2->data.linear.uv[0] +
                             in1->data.linear.uv[3] * in2->data.linear.uv[2];
    out->data.linear.uv[3] = in1->data.linear.uv[2] * in2->data.linear.uv[1] +
                             in1->data.linear.uv[3] * in2->data.linear.uv[3];
    out->data.linear.uv[4] = in1->data.linear.uv[4] + in2->data.linear.uv[4];
    out->data.linear.uv[5] = in1->data.linear.uv[5] + in2->data.linear.uv[5];
}

/* Sets linear keyframe data from a matrix's right and up axes and position */
RpUVAnimLinearKeyFrameData *fn_80453594(RpUVAnimLinearKeyFrameData *data, const RwMatrix *matrix) {
    data->uv[0] = matrix->right.x;
    data->uv[1] = matrix->right.y;
    data->uv[2] = matrix->up.x;
    data->uv[3] = matrix->up.y;
    data->uv[4] = matrix->pos.x;
    data->uv[5] = matrix->pos.y;
    return data;
}
