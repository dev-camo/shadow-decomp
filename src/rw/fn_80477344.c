/* RenderWare animation toolkit: registering interpolation schemes, creating, destroying and
 * streaming keyframe animations, and the interpolators that play them forwards and backwards.
 * The original file name is unknown. */
#include "rw/rtanim.h"
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000

/* The chunk ID of an animation and the error plugin ID of the toolkit (inferred) */
#define rwID_ANIMANIMATION 0x1B
#define rwID_RTANIM 0x1B7

/* The toolkit's error codes (inferred) */
#define E_RT_ANIM_INTERP_IDINUSE 0
#define E_RT_ANIM_INTERP_BLOCKFULL 1
#define E_RT_ANIM_INTERP_NOTFOUND 2

/* The stream format version */
#define rtANIMSTREAMCURRENTVERSION 0x100

/* Reports an error of the toolkit; args is the parenthesized error code */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_RTANIM;                                                             \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

RtAnimInterpolatorInfo lbl_805E2740[rtANIMMAXINTERPOLATORS];
RwInt32 lbl_805F1540;

RwBool fn_80477344(RtAnimInterpolatorInfo *interpolatorInfo) {
    if (lbl_805F1540 < rtANIMMAXINTERPOLATORS) {
        RwInt32 i;

        for (i = 0; i < lbl_805F1540; i++) {
            if (interpolatorInfo->typeID == lbl_805E2740[i].typeID) {
                RWERROR((E_RT_ANIM_INTERP_IDINUSE));
                return FALSE;
            }
        }
        lbl_805E2740[lbl_805F1540] = *interpolatorInfo;
        lbl_805F1540++;
        return TRUE;
    }
    RWERROR((E_RT_ANIM_INTERP_BLOCKFULL));
    return FALSE;
}

/* The registered scheme of a type */
static inline RtAnimInterpolatorInfo *AnimGetInterpolatorInfo(RwInt32 typeID) {
    RwInt32 i;

    for (i = 0; i < lbl_805F1540; i++) {
        if (typeID == lbl_805E2740[i].typeID) {
            return &lbl_805E2740[i];
        }
    }
    RWERROR((E_RT_ANIM_INTERP_NOTFOUND));
    return NULL;
}

RtAnimAnimation *fn_80477468(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags, RwReal duration) {
    RtAnimInterpolatorInfo *interpInfo = AnimGetInterpolatorInfo(typeID);
    RtAnimAnimation *animation;

    if (interpInfo == NULL) {
        return NULL;
    }
    animation = RwMalloc(sizeof(RtAnimAnimation) + numFrames * interpInfo->animKeyFrameSize +
                             interpInfo->customDataSize,
                         rwMEMHINTDUR_EVENT | rwID_ANIMANIMATION);
    animation->numFrames = numFrames;
    animation->duration = duration;
    animation->flags = flags;
    animation->interpInfo = interpInfo;
    animation->pFrames = &animation[1];
    if (interpInfo->customDataSize > 0) {
        animation->customData =
            (RwUInt8 *)animation->pFrames + numFrames * interpInfo->animKeyFrameSize;
    } else {
        animation->customData = NULL;
    }
    return animation;
}

RwBool fn_8047759C(RtAnimAnimation *animation) {
    RwFree(animation);
    return TRUE;
}

RtAnimAnimation *fn_804775CC(RwStream *stream) {
    RwInt32 version;
    RwInt32 typeID;
    RwInt32 numFrames;
    RwInt32 flags;
    RwReal duration;
    RtAnimInterpolatorInfo *interpInfo;
    RtAnimAnimation *animation;

    if (fn_8047D868(stream, &version, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (version != rtANIMSTREAMCURRENTVERSION) {
        return NULL;
    }
    if (fn_8047D868(stream, &typeID, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    interpInfo = AnimGetInterpolatorInfo(typeID);
    if (interpInfo == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &numFrames, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &flags, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D784(stream, &duration, sizeof(RwReal)) == NULL) {
        return NULL;
    }
    animation = fn_80477468(typeID, numFrames, flags, duration);
    if (animation == NULL) {
        return NULL;
    }
    interpInfo->keyFrameStreamReadCB(stream, animation);
    return animation;
}

RwBool fn_80477860(const RtAnimAnimation *animation, RwStream *stream) {
    RwInt32 version = rtANIMSTREAMCURRENTVERSION;

    fn_8047CEF0(stream, rwID_ANIMANIMATION, fn_80477978(animation), RW_STREAM_VERSION,
                RW_STREAM_BUILD);
    if (fn_8047D458(stream, &version, sizeof(RwInt32)) == NULL) {
        return FALSE;
    }
    if (fn_8047D458(stream, &animation->interpInfo->typeID, sizeof(RwInt32)) == NULL) {
        return FALSE;
    }
    if (fn_8047D458(stream, &animation->numFrames, sizeof(RwInt32)) == NULL) {
        return FALSE;
    }
    if (fn_8047D458(stream, &animation->flags, sizeof(RwInt32)) == NULL) {
        return FALSE;
    }
    if (fn_8047D350(stream, &animation->duration, sizeof(RwReal)) == NULL) {
        return FALSE;
    }
    animation->interpInfo->keyFrameStreamWriteCB(animation, stream);
    return TRUE;
}

/* The stream size of an animation: its header (version, type, frame count, flags and duration)
 * and the scheme's keyframe data */
RwInt32 fn_80477978(const RtAnimAnimation *animation) {
    return animation->interpInfo->keyFrameStreamGetSizeCB(animation) + sizeof(RwInt32) * 4 +
           sizeof(RwReal);
}

/* The nodes of an animation: its first keyframes, up to the first that follows one of them */
RwInt32 fn_804779A8(const RtAnimAnimation *animation) {
    RwInt32 numNodes = 0;
    RtAnimKeyFrameHeader *frame = animation->pFrames;
    RwInt32 frameSize = animation->interpInfo->animKeyFrameSize;

    while (frame->prevFrame != animation->pFrames) {
        frame = (RtAnimKeyFrameHeader *)((RwUInt8 *)frame + frameSize);
        numNodes++;
    }
    return numNodes;
}

RtAnimInterpolator *fn_804779DC(RwInt32 numNodes, RwInt32 maxInterpKeyFrameSize) {
    RtAnimInterpolator *anim =
        RwMalloc(sizeof(RtAnimInterpolator) + numNodes * maxInterpKeyFrameSize,
                 rwMEMHINTDUR_EVENT | rwID_ANIMANIMATION);

    anim->numNodes = numNodes;
    anim->pCurrentAnim = NULL;
    anim->pNextFrame = NULL;
    anim->currentTime = 0.0f;
    anim->pAnimCallBack = NULL;
    anim->animCallBackTime = -1.0f;
    anim->pAnimCallBackData = NULL;
    anim->pAnimLoopCallBack = NULL;
    anim->pAnimLoopCallBackData = NULL;
    anim->currentInterpKeyFrameSize = maxInterpKeyFrameSize;
    anim->currentAnimKeyFrameSize = -1;
    anim->maxInterpKeyFrameSize = maxInterpKeyFrameSize;
    anim->isSubInterpolator = FALSE;
    anim->offsetInParent = 0;
    anim->parentAnimation = anim;
    anim->keyFrameApplyCB = NULL;
    anim->keyFrameInterpolateCB = NULL;
    anim->keyFrameBlendCB = NULL;
    anim->keyFrameAddCB = NULL;
    return anim;
}

void fn_80477A8C(RtAnimInterpolator *anim) {
    RwFree(anim);
}

/* A keyframe of an animation */
#define ANIMKEYFRAME(anim, animation, index)                                                       \
    ((RtAnimKeyFrameHeader *)((RwUInt8 *)(animation)->pFrames +                                    \
                              (index) * (anim)->currentAnimKeyFrameSize))

/* Starts an animation: each node between its first two keyframes */
RwBool fn_80477AB8(RtAnimInterpolator *anim, RtAnimAnimation *animation) {
    RwInt32 i;
    RtAnimKeyFrameHeader *keyFrame1;
    RtAnimKeyFrameHeader *keyFrame2;
    RtAnimInterpFrameHeader *iFrame;
    RtAnimInterpolatorInfo *interpInfo;

    anim->pCurrentAnim = animation;
    anim->currentTime = 0.0f;
    interpInfo = animation->interpInfo;
    anim->currentInterpKeyFrameSize = interpInfo->interpKeyFrameSize;
    anim->currentAnimKeyFrameSize = interpInfo->animKeyFrameSize;
    anim->keyFrameApplyCB = interpInfo->keyFrameApplyCB;
    anim->keyFrameBlendCB = interpInfo->keyFrameBlendCB;
    anim->keyFrameInterpolateCB = interpInfo->keyFrameInterpolateCB;
    anim->keyFrameAddCB = interpInfo->keyFrameAddCB;
    for (i = 0; i < anim->numNodes; i++) {
        anim->keyFrameInterpolateCB(rtANIMGETINTERPFRAME(anim, i), ANIMKEYFRAME(anim, animation, i),
                                    ANIMKEYFRAME(anim, animation, i + anim->numNodes), 0.0f,
                                    animation->customData);
    }
    iFrame = rtANIMGETINTERPFRAME(anim, 0);
    keyFrame1 = animation->pFrames;
    keyFrame2 = ANIMKEYFRAME(anim, animation, anim->numNodes);
    for (i = 0; i < anim->numNodes; i++) {
        iFrame->keyFrame1 = keyFrame1;
        iFrame->keyFrame2 = keyFrame2;
        keyFrame1 = (RtAnimKeyFrameHeader *)((RwUInt8 *)keyFrame1 + anim->currentAnimKeyFrameSize);
        keyFrame2 = (RtAnimKeyFrameHeader *)((RwUInt8 *)keyFrame2 + anim->currentAnimKeyFrameSize);
        iFrame = (RtAnimInterpFrameHeader *)((RwUInt8 *)iFrame + anim->currentInterpKeyFrameSize);
    }
    anim->pNextFrame =
        (RwUInt8 *)animation->pFrames + anim->numNodes * anim->currentAnimKeyFrameSize * 2;
    return TRUE;
}

RwBool fn_80477BF0(RtAnimInterpolator *outAnim, RtAnimInterpolator *inAnim) {
    memcpy(rtANIMGETINTERPFRAME(outAnim, 0), rtANIMGETINTERPFRAME(inAnim, 0),
           outAnim->currentInterpKeyFrameSize * outAnim->numNodes);
    return TRUE;
}

/* Calls a callback, removing it if it returns NULL */
#define ANIMCALLBACK(callBack, data)                                                               \
    do {                                                                                           \
        if (anim->callBack != NULL && anim->callBack(anim, anim->data) == NULL) {                  \
            anim->callBack = NULL;                                                                 \
        }                                                                                          \
    } while (0)

RwBool fn_80477C2C(RtAnimInterpolator *anim, RwReal deltaTime) {
    RtAnimAnimation *animation;
    RwBool looped = FALSE;
    RwBool passed = anim->currentTime > anim->animCallBackTime &&
                    anim->currentTime - deltaTime <= anim->animCallBackTime;

    anim->currentTime -= deltaTime;
    animation = anim->pCurrentAnim;
    if (anim->currentTime < 0.0f) {
        RwReal duration = animation->duration;
        RwReal time;

        while (anim->currentTime < 0.0f) {
            anim->currentTime += duration;
        }
        time = anim->currentTime;
        fn_80477AB8(anim, animation);
        fn_80477ED4(anim, time);
        passed |= anim->currentTime < anim->animCallBackTime;
        looped = TRUE;
    } else {
        RwInt32 i = anim->numNodes;
        RtAnimInterpFrameHeader *iFrame = rtANIMGETINTERPFRAME(anim, 0);

        while (i-- != 0) {
            while (anim->currentTime < iFrame->keyFrame1->time) {
                iFrame->keyFrame2 = iFrame->keyFrame1;
                iFrame->keyFrame1 = iFrame->keyFrame1->prevFrame;
            }
            anim->keyFrameInterpolateCB(iFrame, iFrame->keyFrame1, iFrame->keyFrame2,
                                        anim->currentTime, anim->pCurrentAnim->customData);
            iFrame =
                (RtAnimInterpFrameHeader *)((RwUInt8 *)iFrame + anim->currentInterpKeyFrameSize);
        }
        anim->pNextFrame = NULL;
    }
    if (passed) {
        ANIMCALLBACK(pAnimCallBack, pAnimCallBackData);
    }
    if (looped) {
        ANIMCALLBACK(pAnimLoopCallBack, pAnimLoopCallBackData);
    }
    return TRUE;
}

RwBool fn_80477ED4(RtAnimInterpolator *anim, RwReal deltaTime) {
    if (deltaTime > 0.0f) {
        RtAnimAnimation *animation = anim->pCurrentAnim;
        RwInt32 numNodes = anim->numNodes;
        RwReal duration = animation->duration;
        RwBool passed;
        RwBool looped = FALSE;
        RwInt32 node;
        RwInt32 i;
        RwUInt8 *endFrame;
        RtAnimInterpFrameHeader *iFrame;

        passed = anim->currentTime < anim->animCallBackTime &&
                 anim->currentTime + deltaTime >= anim->animCallBackTime;
        anim->currentTime += deltaTime;
        if (anim->currentTime > duration) {
            RwReal time;

            while (anim->currentTime > duration) {
                anim->currentTime -= duration;
                passed |= anim->currentTime >= anim->animCallBackTime;
                looped = TRUE;
            }
            time = anim->currentTime;
            fn_80477AB8(anim, animation);
            anim->currentTime = time;
        }
        if (anim->pNextFrame == NULL) {
            RwReal time = anim->currentTime;

            fn_80477AB8(anim, animation);
            anim->currentTime = time;
        }

        node = 0;
        endFrame =
            (RwUInt8 *)animation->pFrames + animation->numFrames * anim->currentAnimKeyFrameSize;
        while ((RwUInt8 *)anim->pNextFrame < endFrame) {
            RtAnimInterpFrameHeader *iFrame;
            RtAnimKeyFrameHeader *prevFrame = ((RtAnimKeyFrameHeader *)anim->pNextFrame)->prevFrame;

            if (prevFrame->time > anim->currentTime) {
                break;
            }
            do {
                iFrame = rtANIMGETINTERPFRAME(anim, node);
                node++;
                if (node >= numNodes) {
                    node = 0;
                }
            } while (iFrame->keyFrame2 != prevFrame);
            iFrame->keyFrame1 = iFrame->keyFrame2;
            iFrame->keyFrame2 = anim->pNextFrame;
            anim->pNextFrame = (RwUInt8 *)anim->pNextFrame + anim->currentAnimKeyFrameSize;
        }

        iFrame = rtANIMGETINTERPFRAME(anim, 0);
        for (i = 0; i < numNodes; i++) {
            anim->keyFrameInterpolateCB(iFrame, iFrame->keyFrame1, iFrame->keyFrame2,
                                        anim->currentTime, anim->pCurrentAnim->customData);
            iFrame =
                (RtAnimInterpFrameHeader *)((RwUInt8 *)iFrame + anim->currentInterpKeyFrameSize);
        }
        if (passed) {
            ANIMCALLBACK(pAnimCallBack, pAnimCallBackData);
        }
        if (looped) {
            ANIMCALLBACK(pAnimLoopCallBack, pAnimLoopCallBackData);
        }
    }
    return TRUE;
}

RwBool fn_804782E8(RtAnimInterpolator *anim, RwReal time) {
    RwReal deltaTime = time - anim->currentTime;

    if (deltaTime < 0.0f) {
        fn_80477C2C(anim, -deltaTime);
    } else {
        fn_80477ED4(anim, deltaTime);
    }
    return TRUE;
}
