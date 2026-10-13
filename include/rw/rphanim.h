#ifndef RW_RPHANIM_H
#define RW_RPHANIM_H

#include "rw/rwcore.h"

/* RenderWare hierarchical animation (plugin ID 0x11E): the standard keyframes and the animation
 * objects they live in. The type and field names are inferred from the stream format and the code
 * that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

/* A quaternion */
typedef struct RtQuat {
    /* 0x0 */ RwV3d imag;
    /* 0xC */ RwReal real;
} RtQuat;

/* An animation: its frames and their interpolation scheme */
typedef struct RtAnimAnimation {
    /* 0x00 */ void *interpInfo;
    /* 0x04 */ RwInt32 numFrames;
    /* 0x08 */ RwInt32 flags;
    /* 0x0C */ RwReal duration;
    /* 0x10 */ void *pFrames;
    /* 0x14 */ void *customData;
} RtAnimAnimation;

/* A standard keyframe: a rotation and translation at a time, after the frame it follows on the
 * same node (0x24 bytes) */
typedef struct RpHAnimKeyFrame {
    /* 0x00 */ struct RpHAnimKeyFrame *prevFrame;
    /* 0x04 */ RwReal time;
    /* 0x08 */ RtQuat q;
    /* 0x18 */ RwV3d t;
} RpHAnimKeyFrame;

/* An interpolated frame: the header of the animation interpolator, then the same rotation and
 * translation as a keyframe */
typedef struct RpHAnimInterpFrame {
    /* 0x00 */ void *keyFrame1;
    /* 0x04 */ void *keyFrame2;
    /* 0x08 */ RtQuat q;
    /* 0x18 */ RwV3d t;
} RpHAnimInterpFrame;

#define rpID_HANIMPLUGIN 0x11E

/* RpHAnimHierarchy flags */
#define rpHANIMHIERARCHYSUBHIERARCHY 0x1 /* shares its parent's matrices and node data */
#define rpHANIMHIERARCHYNOMATRICES 0x2

/* The callbacks of a keyframe scheme. The toolkit is generic over schemes, so the keyframe callbacks
 * take untyped frames; each scheme's functions use their own frame types. */
typedef void (*RtAnimKeyFrameApplyCallBack)(void *result, void *voidIFrame);
typedef void (*RtAnimKeyFrameBlendCallBack)(void *voidOut, void *voidIn1, void *voidIn2,
                                            RwReal alpha);
typedef void (*RtAnimKeyFrameInterpolateCallBack)(void *voidOut, void *voidIn1, void *voidIn2,
                                                  RwReal time, void *customData);
typedef void (*RtAnimKeyFrameAddCallBack)(void *voidOut, void *voidIn1, void *voidIn2);
typedef void (*RtAnimKeyFrameMulRecipCallBack)(void *voidFrame, void *voidStart);
typedef RtAnimAnimation *(*RtAnimKeyFrameStreamReadCallBack)(RwStream *stream,
                                                             RtAnimAnimation *animation);
typedef RwBool (*RtAnimKeyFrameStreamWriteCallBack)(RtAnimAnimation *animation, RwStream *stream);
typedef RwInt32 (*RtAnimKeyFrameStreamGetSizeCallBack)(RtAnimAnimation *animation);

/* The callbacks of a keyframe scheme, as registered with the animation toolkit (0x30 bytes) */
typedef struct RtAnimInterpolatorInfo {
    /* 0x00 */ RwInt32 typeID;
    /* 0x04 */ RwInt32 interpKeyFrameSize;
    /* 0x08 */ RwInt32 animKeyFrameSize;
    /* 0x0C */ RtAnimKeyFrameApplyCallBack keyFrameApplyCB;
    /* 0x10 */ RtAnimKeyFrameBlendCallBack keyFrameBlendCB;
    /* 0x14 */ RtAnimKeyFrameInterpolateCallBack keyFrameInterpolateCB;
    /* 0x18 */ RtAnimKeyFrameAddCallBack keyFrameAddCB;
    /* 0x1C */ RtAnimKeyFrameMulRecipCallBack keyFrameMulRecipCB;
    /* 0x20 */ RtAnimKeyFrameStreamReadCallBack keyFrameStreamReadCB;
    /* 0x24 */ RtAnimKeyFrameStreamWriteCallBack keyFrameStreamWriteCB;
    /* 0x28 */ RtAnimKeyFrameStreamGetSizeCallBack keyFrameStreamGetSizeCB;
    /* 0x2C */ RwInt32 customDataSize;
} RtAnimInterpolatorInfo;

/* An animation interpolator; only the member used here is mapped */
typedef struct RtAnimInterpolator {
    /* 0x00 */ RwUInt8 field_0x0[0x20];
    /* 0x20 */ RwInt32 maxInterpKeyFrameSize;
} RtAnimInterpolator;

/* A node of a hierarchy (0x10 bytes) */
typedef struct RpHAnimNodeInfo {
    /* 0x0 */ RwInt32 nodeID;
    /* 0x4 */ RwInt32 nodeIndex;
    /* 0x8 */ RwInt32 flags;
    /* 0xC */ RwFrame *pFrame;
} RpHAnimNodeInfo;

/* A hierarchy of animated frames (0x24 bytes) */
typedef struct RpHAnimHierarchy {
    /* 0x00 */ RwInt32 flags; /* rpHANIMHIERARCHY* */
    /* 0x04 */ RwInt32 numNodes;
    /* 0x08 */ RwMatrix *pMatrixArray; /* 16-byte aligned, inside pMatrixArrayUnaligned */
    /* 0x0C */ void *pMatrixArrayUnaligned;
    /* 0x10 */ RpHAnimNodeInfo *pNodeInfo;
    /* 0x14 */ RwFrame *parentFrame;
    /* 0x18 */ struct RpHAnimHierarchy *parentHierarchy;
    /* 0x1C */ RwInt32 rootParentOffset;
    /* 0x20 */ RtAnimInterpolator *currentAnim;
} RpHAnimHierarchy;

/* The data the plugin keeps in each frame */
typedef struct RpHAnimFrameExtension {
    /* 0x0 */ RwInt32 id; /* -1 for none */
    /* 0x4 */ RpHAnimHierarchy *hierarchy;
} RpHAnimFrameExtension;

/* animation toolkit (inferred) */
RwBool fn_80477344(RtAnimInterpolatorInfo *interpolatorInfo);
RtAnimInterpolator *fn_804779DC(RwInt32 numNodes, RwInt32 maxInterpKeyFrameSize);
void fn_80477A8C(RtAnimInterpolator *interpolator);

/* hierarchies */
RpHAnimHierarchy *fn_804472AC(RwInt32 numNodes, RwUInt32 *nodeFlags, RwInt32 *nodeIDs,
                              RwInt32 flags, RwInt32 maxInterpKeyFrameSize);
RpHAnimHierarchy *fn_80447418(RpHAnimHierarchy *hierarchy);
RwInt32 fn_8044758C(RwFrame *frame);

/* the keyframe scheme */
void fn_80445CF4(RwMatrix *matrix, RpHAnimInterpFrame *frame);
void fn_80445E08(RpHAnimInterpFrame *out, RpHAnimKeyFrame *in1, RpHAnimKeyFrame *in2, RwReal time,
                 void *customData);
void fn_8044621C(RpHAnimInterpFrame *out, RpHAnimInterpFrame *in1, RpHAnimInterpFrame *in2,
                 RwReal alpha);
RtAnimAnimation *fn_80446620(RwStream *stream, RtAnimAnimation *animation);
RwBool fn_804466DC(RtAnimAnimation *animation, RwStream *stream);
RwInt32 fn_80446780(RtAnimAnimation *animation);
void fn_80446790(RpHAnimKeyFrame *frame, RpHAnimKeyFrame *start);
void fn_804468D0(RpHAnimKeyFrame *out, RpHAnimKeyFrame *in1, RpHAnimKeyFrame *in2);

#ifdef __cplusplus
}
#endif

#endif
