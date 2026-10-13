#ifndef RW_RPPTANK_H
#define RW_RPPTANK_H

#include "rw/rpworld.h"

/* RenderWare particle tank (plugin ID 0x12F): an atomic that draws up to a maximum number of
 * particles from per-particle data streams or per-tank constants. The data flag values follow the
 * sizes of the streams they select (lbl_8051BCA8); the type and field names are inferred from the
 * code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_PTANKPLUGIN 0x12F

/* data flags: the per-particle streams */
#define rpPTANKDFLAGPOSITION 0x1        /* RwV3d */
#define rpPTANKDFLAGCOLOR 0x2           /* RwRGBA */
#define rpPTANKDFLAGSIZE 0x4            /* RwV2d */
#define rpPTANKDFLAGMATRIX 0x8          /* RwMatrix */
#define rpPTANKDFLAGNORMAL 0x10         /* RwV3d */
#define rpPTANKDFLAG2DROTATE 0x20       /* RwReal */
#define rpPTANKDFLAGVTXCOLOR 0x40       /* an RwRGBA per corner */
#define rpPTANKDFLAGVTX2TEXCOORDS 0x80  /* two RwTexCoords */
#define rpPTANKDFLAGVTX4TEXCOORDS 0x100 /* an RwTexCoords per corner */
/* data flags: the constants used in place of a stream */
#define rpPTANKDFLAGCNSMATRIX 0x8000
#define rpPTANKDFLAGCNS2DROTATE 0x20000
#define rpPTANKDFLAGCNSVTXCOLOR 0x40000
#define rpPTANKDFLAGCNSVTX2TEXCOORDS 0x80000
#define rpPTANKDFLAGCNSVTX4TEXCOORDS 0x100000
/* data flags: the layout */
#define rpPTANKDFLAGUSECENTER 0x1000000  /* particles are placed by their center */
#define rpPTANKDFLAGARRAY 0x10000000     /* one array per stream */
#define rpPTANKDFLAGSTRUCTURE 0x20000000 /* the streams interleaved per particle */

/* lock flags */
#define rpPTANKLOCKWRITE 0x40000000

/* A locked stream: its first element and the distance between elements */
typedef struct RpPTankLockStruct {
    /* 0x0 */ RwUInt8 *data;
    /* 0x4 */ RwInt32 stride;
} RpPTankLockStruct;

/* The streams of a tank (0x50 bytes) */
typedef struct RpPTankData {
    /* 0x00 */ void *data;
    /* 0x04 */ RpPTankLockStruct dataPosition;
    /* 0x0C */ RpPTankLockStruct dataMatrix;
    /* 0x14 */ RpPTankLockStruct dataNormals;
    /* 0x1C */ RpPTankLockStruct dataSize;
    /* 0x24 */ RpPTankLockStruct dataColor;
    /* 0x2C */ RpPTankLockStruct dataVtxColor;
    /* 0x34 */ RpPTankLockStruct dataRotate;
    /* 0x3C */ RpPTankLockStruct dataVtx2TexCoords;
    /* 0x44 */ RpPTankLockStruct dataVtx4TexCoords;
    /* 0x4C */ RwV3d
        *cameraAxes; /* the right and up axes particles face while rendered (inferred) */
} RpPTankData;

/* How a tank's streams are laid out */
typedef struct RpPTankFormatDescriptor {
    /* 0x0 */ RwInt32 numClusters;
    /* 0x4 */ RwInt32 stride; /* of the interleaved particle structure */
    /* 0x8 */ RwUInt32 dataFlags;
} RpPTankFormatDescriptor;

/* The callbacks that allocate, create, instance and render a tank */
typedef void *(*RpPTankAllocCallBack)(RpPTankData *ptankGlobal, RwInt32 maxPCount,
                                      RwUInt32 dataFlags, RwUInt32 platFlags);
typedef RwBool (*RpPTankCreateCallBack)(RpAtomic *atomic, RpPTankData *ptankGlobal,
                                        RwInt32 maxPCount, RwUInt32 dataFlags, RwUInt32 platFlags);
typedef RwBool (*RpPTankInstanceCallBack)(RpAtomic *atomic, RpPTankData *ptankGlobal,
                                          RwInt32 actPCount, RwUInt32 instFlags);
typedef RwBool (*RpPTankRenderCallBack)(RpAtomic *atomic, RpPTankData *ptankGlobal,
                                        RwInt32 actPCount);

typedef struct RpPTankCallBacks {
    /* 0x0 */ RpPTankAllocCallBack alloc;
    /* 0x4 */ RpPTankCreateCallBack create;
    /* 0x8 */ RpPTankInstanceCallBack instance;
    /* 0xC */ RpPTankRenderCallBack render;
} RpPTankCallBacks;

/* A GameCube particle kernel: writes the vertices of count particles (hand-written assembly) */
typedef void (*PTankKernel)(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);

/* The tank of an atomic (0x138 bytes) */
typedef struct RpPTankAtomicExtPrv {
    /* 0x000 */ RwInt32 maxPCount;
    /* 0x004 */ RwInt32 actPCount;
    /* 0x008 */ RwUInt32 field_0x8;
    /* 0x00C */ void *rawdata;
    /* 0x010 */ RpAtomicCallBackRender defaultRenderCB;
    /* 0x014 */ RpPTankCallBacks ptankCallBacks;
    /* 0x024 */ RwUInt32 field_0x24;
    /* 0x028 */ PTankKernel kernel;
    /* 0x02C */ RwUInt32 field_0x2C;
    /* 0x030 */ RwUInt32 field_0x30;
    /* 0x034 */ RwUInt32 field_0x34;
    /* 0x038 */ RwUInt32 field_0x38;
    /* 0x03C */ RwUInt32 lockFlags; /* the streams locked for writing since the last unlock */
    /* 0x040 */ RwUInt32 instFlags; /* the streams to instance before the next render */
    /* 0x044 */ RwUInt32 platFlags;
    /* 0x048 */ RpPTankData publicData;
    /* 0x098 */ RpPTankFormatDescriptor format;
    /* 0x0A4 */ RwInt32 srcBlend;
    /* 0x0A8 */ RwInt32 dstBlend;
    /* 0x0AC */ RwBool vertexAlphaBlend;
    /* 0x0B0 */ RwV2d cCenter;
    /* 0x0B8 */ RwV2d cSize;
    /* 0x0C0 */ RwReal cRotate;
    /* 0x0C4 */ RwRGBA cColor;
    /* 0x0C8 */ RwRGBA cVtxColor[4];
    /* 0x0D8 */ RwTexCoords cVtxUV[4];
    /* 0x0F8 */ RwMatrix cMatrix;
} RpPTankAtomicExtPrv;

extern RwInt32 lbl_805F1448; /* atomic plugin offset of the tank */
extern void *lbl_805F1450;   /* the tank pipeline (GameCube) */

#define PTANKEXT(atomic) (*RWPLUGINOFFSET(RpPTankAtomicExtPrv *, atomic, lbl_805F1448))

/* the plugin and tank atomics */
RwBool fn_80447640(void); /* attaches the plugin */
RpAtomic *fn_80448074(RwInt32 maxPCount, RwUInt32 dataFlags, RwUInt32 platFlags);
RpAtomic *fn_804480C8(RwInt32 maxPCount, RwUInt32 dataFlags, RwUInt32 platFlags,
                      RpPTankCallBacks *callbacks);
RwBool fn_804483E8(RpAtomic *atomic); /* destroys a tank atomic */
RwBool fn_80448408(RpAtomic *atomic, RpPTankLockStruct *dst, RwUInt32 dataFlags,
                   RwUInt32 lockFlags);
RpAtomic *fn_80448598(RpAtomic *atomic); /* unlocks a tank */

/* the default callbacks (GameCube) */
RwBool fn_80448850(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 maxPCount,
                   RwUInt32 dataFlags, RwUInt32 platFlags);
RwBool fn_80448A1C(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 actPCount,
                   RwUInt32 instFlags);
RwBool fn_80448A24(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 actPCount);

/* the engine plugin constructor and destructor, which create and destroy the tank pipeline
 * (GameCube) */
void *fn_804485BC(void *object, RwInt32 offset, RwInt32 size);
void *fn_80448610(void *object, RwInt32 offset, RwInt32 size);

/* the lighting and render callbacks of the tank pipeline (GameCube) */
RpAtomic *fn_80448CB0(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
RpAtomic *fn_80448D04(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);

/* the particle kernels, by layout: matrices (with or without a center), the constant matrix, sizes
 * and rotations, or positions alone (GameCube, hand-written assembly) */
void fn_8044920C(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_804495D0(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_80449994(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_80449D5C(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044A168(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044A540(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044A93C(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044ADA8(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044B210(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);
void fn_8044B654(RwUInt16 count, RwUInt32 platFlags, RpPTankData *ptankGlobal);

/* GameCube platform flags: the kernel, texture coordinate and color sources and normals */
#define PTANKPLATFLAGCOLORSTREAMS 0x70000 /* colors come from a stream */
#define PTANKPLATFLAGCNSCOLOR 0x80000     /* the constant color */

#ifdef __cplusplus
}
#endif

#endif
