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

#ifdef __cplusplus
}
#endif

#endif
