#ifndef RW_RTANIM_H
#define RW_RTANIM_H

#include "rw/rwcore.h"

/* RenderWare animation toolkit: interpolation schemes, keyframe animations and the interpolators
 * that play them. The type and field names follow the RenderWare 3 RtAnim toolkit, whose layout
 * and behaviour the code matches (inferred). */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RtAnimAnimation RtAnimAnimation;
typedef struct RtAnimInterpolator RtAnimInterpolator;

typedef void (*RtAnimKeyFrameApplyCallBack)(void *result, void *voidIFrame);
typedef void (*RtAnimKeyFrameBlendCallBack)(void *voidOut, void *voidIn1, void *voidIn2,
                                            RwReal alpha);
typedef void (*RtAnimKeyFrameInterpolateCallBack)(void *voidOut, void *voidIn1, void *voidIn2,
                                                  RwReal time, void *customData);
typedef void (*RtAnimKeyFrameAddCallBack)(void *voidOut, void *voidIn1, void *voidIn2);
typedef void (*RtAnimKeyFrameMulRecipCallBack)(void *voidFrame, void *voidStart);
typedef RtAnimAnimation *(*RtAnimKeyFrameStreamReadCallBack)(RwStream *stream,
                                                             RtAnimAnimation *animation);
typedef RwBool (*RtAnimKeyFrameStreamWriteCallBack)(const RtAnimAnimation *animation,
                                                    RwStream *stream);
typedef RwInt32 (*RtAnimKeyFrameStreamGetSizeCallBack)(const RtAnimAnimation *animation);

/* Called when an interpolator passes its callback time or loops; returning NULL removes it */
typedef RtAnimInterpolator *(*RtAnimCallBack)(RtAnimInterpolator *animInstance, void *data);

/* An interpolation scheme (0x30 bytes) */
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

/* An animation (0x18 bytes, followed by its keyframes and custom data) */
struct RtAnimAnimation {
    /* 0x00 */ RtAnimInterpolatorInfo *interpInfo;
    /* 0x04 */ RwInt32 numFrames;
    /* 0x08 */ RwInt32 flags;
    /* 0x0C */ RwReal duration;
    /* 0x10 */ void *pFrames;
    /* 0x14 */ void *customData;
};

/* The start of every keyframe: the previous keyframe of its node and its time */
typedef struct RtAnimKeyFrameHeader {
    /* 0x0 */ void *prevFrame;
    /* 0x4 */ RwReal time;
} RtAnimKeyFrameHeader;

/* The start of every interpolated frame: the keyframes it is between */
typedef struct RtAnimInterpFrameHeader {
    /* 0x0 */ RtAnimKeyFrameHeader *keyFrame1;
    /* 0x4 */ RtAnimKeyFrameHeader *keyFrame2;
} RtAnimInterpFrameHeader;

/* An interpolator (0x4C bytes, followed by an interpolated frame per node) */
struct RtAnimInterpolator {
    /* 0x00 */ RtAnimAnimation *pCurrentAnim;
    /* 0x04 */ RwReal currentTime;
    /* 0x08 */ void *pNextFrame;
    /* 0x0C */ RtAnimCallBack pAnimCallBack;
    /* 0x10 */ void *pAnimCallBackData;
    /* 0x14 */ RwReal animCallBackTime;
    /* 0x18 */ RtAnimCallBack pAnimLoopCallBack;
    /* 0x1C */ void *pAnimLoopCallBackData;
    /* 0x20 */ RwInt32 maxInterpKeyFrameSize;
    /* 0x24 */ RwInt32 currentInterpKeyFrameSize;
    /* 0x28 */ RwInt32 currentAnimKeyFrameSize;
    /* 0x2C */ RwInt32 numNodes;
    /* 0x30 */ RwBool isSubInterpolator;
    /* 0x34 */ RwInt32 offsetInParent;
    /* 0x38 */ RtAnimInterpolator *parentAnimation;
    /* 0x3C */ RtAnimKeyFrameApplyCallBack keyFrameApplyCB;
    /* 0x40 */ RtAnimKeyFrameBlendCallBack keyFrameBlendCB;
    /* 0x44 */ RtAnimKeyFrameInterpolateCallBack keyFrameInterpolateCB;
    /* 0x48 */ RtAnimKeyFrameAddCallBack keyFrameAddCB;
};

/* An interpolator's interpolated frame of a node */
#define rtANIMGETINTERPFRAME(anim, nodeIndex)                                                      \
    ((void *)((RwUInt8 *)&(anim)[1] + (nodeIndex) * (anim)->currentInterpKeyFrameSize))

/* The schemes that can be registered */
#define rtANIMMAXINTERPOLATORS 16

/* the registered schemes and their number */
extern RtAnimInterpolatorInfo lbl_805E2740[rtANIMMAXINTERPOLATORS];
extern RwInt32 lbl_805F1540;

/* the toolkit (inferred) */
RwBool fn_80477344(RtAnimInterpolatorInfo *interpolatorInfo); /* registers a scheme */
RtAnimAnimation *fn_80477468(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags,
                             RwReal duration);                          /* creates an animation */
RwBool fn_8047759C(RtAnimAnimation *animation);                         /* destroys an animation */
RtAnimAnimation *fn_804775CC(RwStream *stream);                         /* reads an animation */
RwBool fn_80477860(const RtAnimAnimation *animation, RwStream *stream); /* writes an animation */
RwInt32 fn_80477978(const RtAnimAnimation *animation); /* an animation's stream size */
RwInt32 fn_804779A8(const RtAnimAnimation *animation); /* an animation's nodes */
RtAnimInterpolator *fn_804779DC(RwInt32 numNodes,
                                RwInt32 maxInterpKeyFrameSize); /* creates an interpolator */
void fn_80477A8C(RtAnimInterpolator *anim);                     /* destroys an interpolator */
RwBool fn_80477AB8(RtAnimInterpolator *anim, RtAnimAnimation *animation); /* sets the animation */
RwBool fn_80477BF0(RtAnimInterpolator *outAnim, RtAnimInterpolator *inAnim); /* copies frames */
RwBool fn_80477C2C(RtAnimInterpolator *anim, RwReal deltaTime); /* steps back in time */
RwBool fn_80477ED4(RtAnimInterpolator *anim, RwReal deltaTime); /* steps forward in time */
RwBool fn_804782E8(RtAnimInterpolator *anim, RwReal time);      /* sets the time */

#ifdef __cplusplus
}
#endif

#endif
