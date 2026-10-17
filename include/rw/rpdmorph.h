#ifndef RW_RPDMORPH_H
#define RW_RPDMORPH_H

#include "rw/rpworld.h"
#include "layout_check.h"

/* RenderWare delta morph animations (plugin ID 0x122, chunk 0x1E). The type and field names are
 * inferred from the stream format and the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_DMORPHPLUGIN 0x122
#define rwID_DMORPHANIMATION 0x1E

/* RpDMorphTarget flags: the vertex streams a target has (the geometry flag bits) */
#define rpDMORPHTARGETPOSITIONS 0x2
#define rpDMORPHTARGETTEXCOORDS 0x4
#define rpDMORPHTARGETPRELIT 0x8
#define rpDMORPHTARGETNORMALS 0x10

/* The run length code of a delta morph target's vertices: each code byte is a run of as many
 * vertices as its low seven bits, which have deltas when its top bit is set. The deltas of those
 * vertices follow one another in the target's streams. The GameCube delta kernels receive the
 * address of this record and read the code and its length through it; the member meanings are
 * inferred from that use and from the stream reader. */
typedef struct rpDMorphRLE {
    /* 0x0 */ RwUInt8 *code;
    /* 0x4 */ RwUInt32 numCodeElements;
    /* 0x8 */ RwUInt32 numDataElements; /* the vertices that have deltas */
    /* 0xC */ RwUInt8 *data;            /* one allocation for the target's delta streams */
} rpDMorphRLE;
CHECK_SIZEOF(rpDMorphRLE, 0x10);

/* A delta morph target of a geometry (0x40 bytes) */
typedef struct RpDMorphTarget {
    /* 0x00 */ RpGeometry *geometry;
    /* 0x04 */ RwChar *name;
    /* 0x08 */ RwUInt32 flags;     /* rpDMORPHTARGET* */
    /* 0x0C */ RwUInt32 field_0xC; /* passed to fn_8045916C when the target is applied */
    /* 0x10 */ rpDMorphRLE rle;
    /* 0x20 */ RwV3d *positions;
    /* 0x24 */ RwV3d *normals;
    /* 0x28 */ RwRGBA *preLitLum;
    /* 0x2C */ RwTexCoords *texCoords;
    /* 0x30 */ RwSphere boundingSphere;
} RpDMorphTarget;
CHECK_SIZEOF(RpDMorphTarget, 0x40);

/* The delta morph data of a geometry (0x1C bytes) */
typedef struct RpDMorphGeometry {
    /* 0x00 */ RwUInt32 numDMorphTargets;
    /* 0x04 */ RpDMorphTarget *dmorphTargets;
    /* 0x08 */ RwReal *values; /* one per target, allocated after the targets */
    /* 0x0C */ RwUInt8 field_0xC[0x10];
} RpDMorphGeometry;
CHECK_SIZEOF(RpDMorphGeometry, 0x1C);

extern RwInt32 lbl_805F1428; /* geometry plugin offset */

#define DMORPHGEOMETRY(geometry) (*RWPLUGINOFFSET(RpDMorphGeometry *, geometry, lbl_805F1428))

#define rpDMORPHNOFRAME 0xFFFFFFFF

/* A key frame (0x14 bytes): the weight goes from startValue to endValue over duration */
typedef struct RpDMorphFrame {
    /* 0x00 */ RwReal startValue;
    /* 0x04 */ RwReal endValue;
    /* 0x08 */ RwReal duration;
    /* 0x0C */ RwReal recipDuration;
    /* 0x10 */ RwUInt32 nextFrame; /* index of the next frame, rpDMORPHNOFRAME for none */
} RpDMorphFrame;
CHECK_SIZEOF(RpDMorphFrame, 0x14);

/* The frames of one delta morph target */
typedef struct RpDMorphFrameList {
    /* 0x0 */ RwUInt32 numFrames;
    /* 0x4 */ RpDMorphFrame *frames;
} RpDMorphFrameList;
CHECK_SIZEOF(RpDMorphFrameList, 0x8);

/* An animation: a frame list per delta morph target, allocated after it */
typedef struct RpDMorphAnimation {
    /* 0x0 */ RwUInt32 numDMorphTargets;
    /* 0x4 */ RpDMorphFrameList *frameLists;
} RpDMorphAnimation;
CHECK_SIZEOF(RpDMorphAnimation, 0x8);

typedef RpAtomic *(*RpDMorphAtomicCallBack)(RpAtomic *atomic, void *data);

/* The animation state of an atomic (0x18 bytes, followed by its frameTimes and frames arrays) */
typedef struct RpDMorphInterpolator {
    /* 0x00 */ RpDMorphAnimation *animation;
    /* 0x04 */ RwReal time;
    /* 0x08 */ RpDMorphFrame **frames; /* the current frame of each target, NULL when done */
    /* 0x0C */ RwReal *frameTimes;     /* the time into it */
    /* 0x10 */ RpDMorphAtomicCallBack callBack; /* called when a frame list loops */
    /* 0x14 */ void *callBackData;
} RpDMorphInterpolator;
CHECK_SIZEOF(RpDMorphInterpolator, 0x18);

/* The delta morph data of an atomic (0xC bytes) */
typedef struct RpDMorphAtomic {
    /* 0x0 */ RwReal *values; /* the weight of each target */
    /* 0x4 */ RpDMorphInterpolator *interpolator;
    /* 0x8 */ RpAtomicCallBackRender renderCallBack; /* the atomic's own, called after morphing */
} RpDMorphAtomic;
CHECK_SIZEOF(RpDMorphAtomic, 0xC);

extern RwInt32 lbl_805F1430; /* atomic plugin offset */

#define DMORPHATOMIC(atomic) RWPLUGINOFFSET(RpDMorphAtomic, atomic, lbl_805F1430)

/* geometries */
RpGeometry *fn_8044382C(RpGeometry *geometry, RwUInt32 numDMorphTargets);
RpGeometry *fn_80443908(RpGeometry *geometry);
RpGeometry *fn_804439A8(RpGeometry *geometry, RwUInt32 index);
RwUInt32 fn_80443A54(const RpGeometry *geometry);

/* atomics */
RpAtomic *fn_80443A74(RpAtomic *atomic);
RpAtomic *fn_80444284(RpAtomic *atomic);
RpAtomic *fn_804442A4(RpAtomic *atomic, RwInt32 pipelineIndex);
RpAtomic *fn_80445CE0(RpAtomic *atomic, RwInt32 pipelineIndex); /* inferred: sets its pipeline */

/* plugin attachment and pipelines */
RwBool fn_804437B0(void);
RwBool fn_8044420C(void);
RwBool fn_80445C38(RwInt32 arg); /* inferred: creates the plugin's pipelines */
RwBool fn_80445C9C(void);        /* inferred: destroys them */

/* animations */
RpDMorphAnimation *fn_80442B98(RpDMorphAnimation *animation);
RpDMorphAnimation *fn_80442C24(RwStream *stream);

#ifdef __cplusplus
}
#endif

#endif
