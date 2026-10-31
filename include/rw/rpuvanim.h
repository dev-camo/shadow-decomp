#ifndef RW_RPUVANIM_H
#define RW_RPUVANIM_H

#include "rw/rphanim.h"
#include "rw/rpworld.h"
#include "rw/rtdict.h"
#include "layout_check.h"

/* RenderWare UV animation (plugin ID 0x135): animations of texture coordinate transforms, kept in a
 * dictionary by name, and up to eight of them playing on a material, each node of an animation
 * driving one of the material's two UV channel matrices. The type and field names are inferred
 * from the stream format and the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_UVANIMPLUGIN 0x135

#define rpUVANIMMAXSLOTS 8    /* animations playing on a material */
#define rpUVANIMNUMCHANNELS 2 /* UV channel matrices of a material */
#define rpUVANIMNAMELENGTH 32

/* keyframe scheme type IDs */
#define rpUVANIMLINEARKEYFRAMES 0x1C0
#define rpUVANIMPARAMKEYFRAMES 0x1C1

/* The keyframe types fn_8045449C creates animations of */
#define rpUVANIMLINEAR 0
#define rpUVANIMPARAM 1

/* A transform as the 2x2 matrix and translation (uv[4], uv[5]) of a linear keyframe */
typedef struct RpUVAnimLinearKeyFrameData {
    /* 0x00 */ RwReal uv[6];
} RpUVAnimLinearKeyFrameData;
CHECK_SIZEOF(RpUVAnimLinearKeyFrameData, 0x18);

/* A transform as the rotation, scales, skew and translation of a parametric keyframe */
typedef struct RpUVAnimParamKeyFrameData {
    /* 0x00 */ RwReal theta; /* radians */
    /* 0x04 */ RwReal s0;
    /* 0x08 */ RwReal s1;
    /* 0x0C */ RwReal skew;
    /* 0x10 */ RwReal x;
    /* 0x14 */ RwReal y;
} RpUVAnimParamKeyFrameData;
CHECK_SIZEOF(RpUVAnimParamKeyFrameData, 0x18);

typedef union RpUVAnimKeyFrameData {
    RpUVAnimLinearKeyFrameData linear;
    RpUVAnimParamKeyFrameData param;
} RpUVAnimKeyFrameData;
CHECK_SIZEOF(RpUVAnimKeyFrameData, 0x18);

/* A keyframe of either scheme (0x20 bytes) */
typedef struct RpUVAnimKeyFrame {
    /* 0x00 */ struct RpUVAnimKeyFrame *prevFrame;
    /* 0x04 */ RwReal time;
    /* 0x08 */ RpUVAnimKeyFrameData data;
} RpUVAnimKeyFrame;
CHECK_SIZEOF(RpUVAnimKeyFrame, 0x20);

/* An interpolated frame of either scheme (0x20 bytes) */
typedef struct RpUVAnimInterpFrame {
    /* 0x00 */ RpUVAnimKeyFrame *keyFrame1;
    /* 0x04 */ RpUVAnimKeyFrame *keyFrame2;
    /* 0x08 */ RpUVAnimKeyFrameData data;
} RpUVAnimInterpFrame;
CHECK_SIZEOF(RpUVAnimInterpFrame, 0x20);

/* The custom data of a UV animation (0x44 bytes, from a free list) */
typedef struct RpUVAnimCustomData {
    /* 0x00 */ RwChar name[rpUVANIMNAMELENGTH];
    /* 0x20 */ RwUInt32 nodeToUVChannelMap[8]; /* the UV channel each node drives */
    /* 0x40 */ RwUInt32 refCount;
} RpUVAnimCustomData;
CHECK_SIZEOF(RpUVAnimCustomData, 0x44);

/* The plugin data of a material (0x28 bytes) */
typedef struct RpUVAnimMaterialExtension {
    /* 0x00 */ RwMatrix *uv[rpUVANIMNUMCHANNELS]; /* only for the channels some node drives */
    /* 0x08 */ RtAnimInterpolator *interp[rpUVANIMMAXSLOTS];
} RpUVAnimMaterialExtension;
CHECK_SIZEOF(RpUVAnimMaterialExtension, 0x28);

#define rwPI 3.1415926535f

extern RtDictSchema lbl_8056D748;           /* the UV animation dictionary schema */
extern RtAnimInterpolatorInfo lbl_8056D77C; /* the linear keyframe scheme */
extern RtAnimInterpolatorInfo lbl_8056D7AC; /* the parametric keyframe scheme */
extern RwInt32 lbl_805F149C;                /* material plugin offset */

/* the translations a parametric transform's rotation is wrapped in, so that it turns about the
 * texture's centre (inferred) */
extern const RwV3d lbl_8051BD40; /* (-0.5, -0.5, 0) */
extern const RwV3d lbl_8051BD4C; /* (0.5, 0.5, 0) */

#define RPUVANIMMATERIALGETEXT(material)                                                           \
    ((RpUVAnimMaterialExtension *)((RwUInt8 *)(material) + lbl_805F149C))

/* keyframes common to both schemes: the stream callbacks (inferred) */
RtAnimAnimation *fn_804530CC(RwStream *stream, RtAnimAnimation *animation);
RwBool fn_804531B0(const RtAnimAnimation *animation, RwStream *stream);
RwInt32 fn_804532A4(const RtAnimAnimation *animation);

/* linear keyframe callbacks (inferred) */
void fn_804532E4(void *result, void *voidIFrame); /* apply */
void fn_80453350(void *voidOut, void *voidIn1, void *voidIn2, RwReal time, void *customData);
void fn_804533E0(void *voidOut, void *voidIn1, void *voidIn2, RwReal alpha); /* blend */
void fn_8045345C(void *voidFrame, void *voidStart);                          /* mul recip */
void fn_80453500(void *voidOut, void *voidIn1, void *voidIn2);               /* add */
RpUVAnimLinearKeyFrameData *fn_80453594(RpUVAnimLinearKeyFrameData *data, const RwMatrix *matrix);

/* parametric keyframe callbacks (inferred) */
void fn_804535C8(void *result, void *voidIFrame); /* apply */
void fn_804536B4(void *voidOut, void *voidIn1, void *voidIn2, RwReal time, void *customData);
void fn_80453770(void *voidOut, void *voidIn1, void *voidIn2, RwReal alpha); /* blend */
void fn_80453818(void *voidFrame, void *voidStart);                          /* mul recip */
void fn_8045381C(void *voidOut, void *voidIn1, void *voidIn2);               /* add */
RpUVAnimParamKeyFrameData *fn_80453820(RpUVAnimParamKeyFrameData *data, const RwMatrix *matrix);

/* custom data streams (inferred) */
RpUVAnimCustomData *fn_80454374(RwStream *stream);
RpUVAnimCustomData *fn_8045441C(RpUVAnimCustomData *customData, RwStream *stream);
RwInt32 fn_80454494(RpUVAnimCustomData *customData);

/* Sets a matrix to a linear keyframe's transform */
static inline void UVAnimLinearKeyFrameApply(RwMatrix *matrix, const RpUVAnimInterpFrame *frame) {
    matrix->right.x = frame->data.linear.uv[0];
    matrix->right.y = frame->data.linear.uv[1];
    matrix->right.z = 0.0f;
    matrix->up.x = frame->data.linear.uv[2];
    matrix->up.y = frame->data.linear.uv[3];
    matrix->up.z = 0.0f;
    matrix->at.x = 0.0f;
    matrix->at.y = 0.0f;
    matrix->at.z = 0.0f;
    matrix->pos.x = frame->data.linear.uv[4];
    matrix->pos.y = frame->data.linear.uv[5];
    matrix->pos.z = 0.0f;
    fn_8047EBA0(matrix);
}

/* Sets a matrix to a parametric keyframe's transform: the scales, skew and translation, then the
 * rotation about the texture's centre. A macro, so that the axis initializer is placed with the
 * function it expands in. */
#define UVANIMPARAMKEYFRAMEAPPLY(matrix, frame)                                                    \
    do {                                                                                           \
        RwV3d axis_ = {0.0f, 0.0f, 1.0f};                                                          \
                                                                                                   \
        (matrix)->right.x = (frame)->data.param.s0;                                                \
        (matrix)->right.y = (frame)->data.param.skew;                                              \
        (matrix)->right.z = 0.0f;                                                                  \
        (matrix)->up.x = 0.0f;                                                                     \
        (matrix)->up.y = (frame)->data.param.s1;                                                   \
        (matrix)->up.z = 0.0f;                                                                     \
        (matrix)->at.x = 0.0f;                                                                     \
        (matrix)->at.y = 0.0f;                                                                     \
        (matrix)->at.z = 0.0f;                                                                     \
        (matrix)->pos.x = (frame)->data.param.x;                                                   \
        (matrix)->pos.y = (frame)->data.param.y;                                                   \
        (matrix)->pos.z = 0.0f;                                                                    \
        fn_8047EBA0(matrix);                                                                       \
        fn_8047F59C((matrix), &lbl_8051BD40, rwCOMBINEPOSTCONCAT);                                 \
        fn_8047EFC8((matrix), &axis_, 180.0f * ((frame)->data.param.theta / rwPI),                 \
                    rwCOMBINEPOSTCONCAT);                                                          \
        fn_8047F59C((matrix), &lbl_8051BD4C, rwCOMBINEPOSTCONCAT);                                 \
    } while (0)

/* animations (inferred) */
RtAnimAnimation *fn_8045449C(const RwChar *name, RwUInt32 numNodes, RwUInt32 numFrames,
                             RwReal duration, RwUInt32 *nodeToUVChannelMap, RwInt32 keyFrameType);
/* release; destroys it with its last reference */
RwBool fn_8045459C(RtAnimAnimation *animation);
/* add a reference; returns the animation */
RtAnimAnimation *fn_80454618(RtAnimAnimation *animation);
/* name */
RwChar *fn_804548E4(RtAnimAnimation *animation);
/* sets a keyframe from a transform matrix */
RwBool fn_80453018(const RtAnimAnimation *animation, RpUVAnimKeyFrame *keyFrame,
                   RpUVAnimKeyFrame *prevFrame, RwReal time, const RwMatrix *matrix);

/* materials (inferred) */
RpMaterial *fn_8045462C(RpMaterial *material); /* applies the animations to the UV matrices */
RpMaterial *fn_804548EC(RpMaterial *material, RwReal time);
RpMaterial *fn_80454964(RpMaterial *material, RwReal deltaTime);
RwBool fn_804549DC(const RpMaterial *material); /* whether any animation plays on it */

/* attaches the plugin */
RwBool fn_80453A78(void);

#ifdef __cplusplus
}
#endif

#endif
