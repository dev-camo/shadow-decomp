/* RenderWare UV animation plugin (plugin ID 0x135): the parametric keyframe scheme, whose transforms
 * are a rotation, two scales, a skew and a translation. The original file name is unknown. */
#include "rw/rpuvanim.h"
#include <math.h>

void fn_804535C8(void *result, void *voidIFrame) {
    UVANIMPARAMKEYFRAMEAPPLY((RwMatrix *)result, (RpUVAnimInterpFrame *)voidIFrame);
}

/* Interpolates between two keyframes, turning the shorter way round */
void fn_804536B4(void *voidOut, void *voidIn1, void *voidIn2, RwReal time, void *customData) {
    RpUVAnimInterpFrame *out = voidOut;
    RpUVAnimKeyFrame *in1 = voidIn1;
    RpUVAnimKeyFrame *in2 = voidIn2;
    RwReal alpha = (time - in1->time) / (in2->time - in1->time);
    RwReal turn = in2->data.param.theta - in1->data.param.theta;

    if (turn < -rwPI) {
        turn += 2.0f * rwPI;
    } else if (turn > rwPI) {
        turn -= 2.0f * rwPI;
    }
    out->data.param.theta = in1->data.param.theta + alpha * turn;
    out->data.param.s0 = in1->data.param.s0 + alpha * (in2->data.param.s0 - in1->data.param.s0);
    out->data.param.s1 = in1->data.param.s1 + alpha * (in2->data.param.s1 - in1->data.param.s1);
    out->data.param.skew =
        in1->data.param.skew + alpha * (in2->data.param.skew - in1->data.param.skew);
    out->data.param.x = in1->data.param.x + alpha * (in2->data.param.x - in1->data.param.x);
    out->data.param.y = in1->data.param.y + alpha * (in2->data.param.y - in1->data.param.y);
}

/* Blends two interpolated frames, turning the shorter way round */
void fn_80453770(void *voidOut, void *voidIn1, void *voidIn2, RwReal alpha) {
    RpUVAnimInterpFrame *out = voidOut;
    RpUVAnimInterpFrame *in1 = voidIn1;
    RpUVAnimInterpFrame *in2 = voidIn2;
    RwReal turn = in2->data.param.theta - in1->data.param.theta;

    if (turn < -rwPI) {
        turn += 2.0f * rwPI;
    } else if (turn > rwPI) {
        turn -= 2.0f * rwPI;
    }
    out->data.param.theta = in1->data.param.theta + alpha * turn;
    out->data.param.s0 = in1->data.param.s0 + alpha * (in2->data.param.s0 - in1->data.param.s0);
    out->data.param.s1 = in1->data.param.s1 + alpha * (in2->data.param.s1 - in1->data.param.s1);
    out->data.param.skew =
        in1->data.param.skew + alpha * (in2->data.param.skew - in1->data.param.skew);
    out->data.param.x = in1->data.param.x + alpha * (in2->data.param.x - in1->data.param.x);
    out->data.param.y = in1->data.param.y + alpha * (in2->data.param.y - in1->data.param.y);
}

/* the scheme does not support taking a start keyframe out or adding keyframes */
void fn_80453818(void *voidFrame, void *voidStart) {}

void fn_8045381C(void *voidOut, void *voidIn1, void *voidIn2) {}

/* Sets parametric keyframe data from a matrix: the rotation from its axes, then the scales, skew
 * and translation of the matrix with that rotation (about the texture's centre) undone */
RpUVAnimParamKeyFrameData *fn_80453820(RpUVAnimParamKeyFrameData *data, const RwMatrix *matrix) {
    RwMatrix unrotated = *matrix;
    RwV3d axis = {0.0f, 0.0f, 1.0f};

    data->theta = atan2(-unrotated.up.x, unrotated.right.x);
    fn_8047F59C(&unrotated, &lbl_8051BD40, rwCOMBINEPOSTCONCAT);
    fn_8047EFC8(&unrotated, &axis, 180.0f * (-data->theta / rwPI), rwCOMBINEPOSTCONCAT);
    fn_8047F59C(&unrotated, &lbl_8051BD4C, rwCOMBINEPOSTCONCAT);
    data->s0 = unrotated.right.x;
    data->skew = unrotated.right.y;
    data->s1 = unrotated.up.y;
    data->x = unrotated.pos.x;
    data->y = unrotated.pos.y;
    return data;
}
