#ifndef RW_RPHANIM_H
#define RW_RPHANIM_H

#include "rw/rtanim.h"
#include "layout_check.h"

/* RenderWare hierarchical animation (plugin ID 0x11E): the standard keyframes and the hierarchies
 * they animate; the animations and interpolators come from the animation toolkit. The type and
 * field names are inferred from the stream format and the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

/* A quaternion */
typedef struct RtQuat {
    /* 0x0 */ RwV3d imag;
    /* 0xC */ RwReal real;
} RtQuat;
CHECK_SIZEOF(RtQuat, 0x10);

/* A standard keyframe: a rotation and translation at a time, after the frame it follows on the
 * same node (0x24 bytes) */
typedef struct RpHAnimKeyFrame {
    /* 0x00 */ struct RpHAnimKeyFrame *prevFrame;
    /* 0x04 */ RwReal time;
    /* 0x08 */ RtQuat q;
    /* 0x18 */ RwV3d t;
} RpHAnimKeyFrame;
CHECK_SIZEOF(RpHAnimKeyFrame, 0x24);

/* An interpolated frame: the header of the animation interpolator, then the same rotation and
 * translation as a keyframe */
typedef struct RpHAnimInterpFrame {
    /* 0x00 */ RpHAnimKeyFrame *keyFrame1;
    /* 0x04 */ RpHAnimKeyFrame *keyFrame2;
    /* 0x08 */ RtQuat q;
    /* 0x18 */ RwV3d t;
} RpHAnimInterpFrame;
CHECK_SIZEOF(RpHAnimInterpFrame, 0x24);

#define rpID_HANIMPLUGIN 0x11E

/* the ID the plugin's keyframe scheme is registered under with the animation toolkit (inferred) */
#define rpHANIMSTDKEYFRAMETYPEID 0x1

/* RpHAnimHierarchy flags */
#define rpHANIMHIERARCHYSUBHIERARCHY 0x1 /* shares its parent's matrices and node data */
#define rpHANIMHIERARCHYNOMATRICES 0x2
#define rpHANIMHIERARCHYLOCALSPACEMATRICES 0x4000 /* (inferred) the matrices are in local space */

/* A node of a hierarchy (0x10 bytes) */
typedef struct RpHAnimNodeInfo {
    /* 0x0 */ RwInt32 nodeID;
    /* 0x4 */ RwInt32 nodeIndex;
    /* 0x8 */ RwInt32 flags;
    /* 0xC */ RwFrame *pFrame;
} RpHAnimNodeInfo;
CHECK_SIZEOF(RpHAnimNodeInfo, 0x10);

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
CHECK_SIZEOF(RpHAnimHierarchy, 0x24);

/* The data the plugin keeps in each frame */
typedef struct RpHAnimFrameExtension {
    /* 0x0 */ RwInt32 id; /* -1 for none */
    /* 0x4 */ RpHAnimHierarchy *hierarchy;
} RpHAnimFrameExtension;
CHECK_SIZEOF(RpHAnimFrameExtension, 0x8);

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
RwBool fn_804466DC(const RtAnimAnimation *animation, RwStream *stream);
RwInt32 fn_80446780(const RtAnimAnimation *animation);
void fn_80446790(RpHAnimKeyFrame *frame, RpHAnimKeyFrame *start);
void fn_804468D0(RpHAnimKeyFrame *out, RpHAnimKeyFrame *in1, RpHAnimKeyFrame *in2);

#ifdef __cplusplus
}
#endif

#endif
