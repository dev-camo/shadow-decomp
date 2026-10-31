#ifndef RW_RWCORE_H
#define RW_RWCORE_H

#include <dolphin/types.h>
#ifdef __cplusplus
/* MSL's headers, which the C++ units built with cflags_csd or cflags_game_cpp search in place of
 * include/libc, have no stdarg.h. */
#include <cstdarg>
#else
#include <stdarg.h>
#endif
#include "layout_check.h"

/* RenderWare Graphics 3.7 (stream chunk version 0x37002), linked into the game. Only what the
 * recovered RenderWare code uses is declared. The functions keep placeholder names; their roles
 * are inferred from their behavior and from the RenderWare stream format. */

#ifdef __cplusplus
extern "C" {
#endif

/* RenderWare's basic types */
typedef int RwInt32;
typedef unsigned int RwUInt32;
typedef short RwInt16;
typedef unsigned short RwUInt16;
typedef unsigned char RwUInt8;
typedef signed char RwInt8;
typedef long long RwInt64;
typedef unsigned long long RwUInt64;
typedef float RwReal;
typedef RwInt32 RwBool;

/* A real's bits, which compare like the real by sign (RenderWare's volatile members keep each test
 * reading memory) */
typedef union RwSplitBits {
    RwReal nReal;
    volatile RwInt32 nInt;
    volatile RwUInt32 nUInt;
} RwSplitBits;
CHECK_SIZEOF(RwSplitBits, 0x4);
typedef char RwChar;

#define RW_STREAM_VERSION 0x37002
#define RW_STREAM_BUILD 0x37
#define rwLIBRARYBASEVERSION 0x35000 /* the oldest stream version that is read */

/* chunk IDs */
#define rwID_STRUCT 0x1
#define rwID_MATLIST 0x8
#define rwID_ATOMICSECT 0x9
#define rwID_PLANESECT 0xA
#define rwID_WORLD 0xB
#define rwID_EXTENSION 0x3
#define rwID_TEXTURE 0x6
#define rwID_COREPLUGIN 0x1   /* the core's plugin ID, also used for its errors */
#define rwCHUNKHEADERSIZE 12  /* the bytes of a chunk header in a stream */
#define rwID_PIPEMODULE 0x409 /* the pipeline module's ID */

/* memory hints: how long an allocation lives, whether it is resized, and the chunk ID of what it
 * is for in the low bits */
#define rwMEMHINTDUR_FUNCTION 0x10000
#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000
#define rwMEMHINTFLAG_RESIZABLE 0x1000000

/* the platforms platform-specific data is for */
typedef RwInt32 RwPlatformID;
#define rwID_GAMECUBE 6

/* errors */
#define E_RW_BADVERSION 0x80000004
#define E_RW_READ 0x8000001A          /* reading a stream failed */
#define E_RW_NOMEM 0x80000013         /* reported with the size that could not be allocated */
#define E_RW_NOERROR 0x80000000       /* no error is pending */
#define E_RW_NULLP 0x80000016         /* a required argument is NULL */
#define E_RW_PLUGINNOTINIT 0x80000018 /* used before its module is ready (inferred) */
#define E_RW_READTEXMASK 0x16         /* a texture could not be read by its name and mask */

/* pipeline errors of the core (inferred from where they are reported) */
#define E_RX_CYCLICPIPELINE 0x1C
#define E_RX_DEP_DEPENDENCIESMISMATCH 0x1D /* an output does not create a required cluster */
#define E_RX_DEP_DUPLICATECLUSTERDEFS 0x1E /* a node lists a cluster of interest twice */
#define E_RX_DEP_NULLCLUSTERDEF 0x1F       /* a cluster of interest has no definition */
#define E_RX_DEP_OUTOFMEMORY 0x20          /* the dependency pass ran out of scratch space */
#define E_RX_FRAGMENTEDPIPELINE 0x22       /* a node other than the entry point has no input */
#define E_RX_IM3DNOTACTIVE 0x23            /* no immediate mode 3D transform is active */
#define E_RX_INVALIDENTRYPOINT 0x24
#define E_RX_INVALIDPRIMTYPE 0x25
#define E_RX_NODETOOMANYCLUSTERSOFINTEREST 0x28
#define E_RX_NODETOOMANYOUTPUTS 0x29
#define E_RX_PIPELINETOOMANYNODES 0x2A
#define E_RX_TOOMANYVERTS 0x32
#define E_RX_UNLOCKEDPIPE 0x34 /* the pipeline must be locked */

typedef struct RwFreeList RwFreeList;

/* RwPrimitiveType */
#define rwPRIMTYPENAPRIMTYPE 0
#define rwPRIMTYPELINELIST 1
#define rwPRIMTYPEPOLYLINE 2
#define rwPRIMTYPETRILIST 3
#define rwPRIMTYPETRISTRIP 4
#define rwPRIMTYPETRIFAN 5
#define rwPRIMTYPEPOINTLIST 6
typedef struct RwCamera RwCamera;

/* render states (RwRenderStateSet/RwRenderStateGet) */
#define rwRENDERSTATETEXTURERASTER 1
#define rwRENDERSTATETEXTUREADDRESS 2
#define rwRENDERSTATETEXTUREADDRESSU 3
#define rwRENDERSTATETEXTUREADDRESSV 4
#define rwRENDERSTATETEXTUREPERSPECTIVE 5
#define rwRENDERSTATEZTESTENABLE 6
#define rwRENDERSTATESHADEMODE 7
#define rwRENDERSTATEZWRITEENABLE 8
#define rwRENDERSTATETEXTUREFILTER 9
#define rwRENDERSTATESRCBLEND 10
#define rwRENDERSTATEDESTBLEND 11
#define rwRENDERSTATEVERTEXALPHAENABLE 12
#define rwRENDERSTATEBORDERCOLOR 13
#define rwRENDERSTATEFOGENABLE 14
#define rwRENDERSTATEFOGCOLOR 15
#define rwRENDERSTATEFOGTYPE 16
#define rwRENDERSTATEFOGDENSITY 17
#define rwRENDERSTATECULLMODE 20
#define rwRENDERSTATEALPHATESTFUNCTION 29
#define rwRENDERSTATEALPHATESTFUNCTIONREF 30

/* render state values */
#define rwFILTERNEAREST 1
#define rwFILTERLINEAR 2
#define rwFILTERMIPNEAREST 3
#define rwFILTERMIPLINEAR 4
#define rwFILTERLINEARMIPNEAREST 5
#define rwFILTERLINEARMIPLINEAR 6
#define rwTEXTUREADDRESSWRAP 1
#define rwTEXTUREADDRESSMIRROR 2
#define rwTEXTUREADDRESSCLAMP 3
#define rwTEXTUREADDRESSBORDER 4
#define rwSHADEMODEGOURAUD 2
#define rwFOGTYPENAFOGTYPE 0
#define rwFOGTYPELINEAR 1
#define rwBLENDZERO 1
#define rwBLENDONE 2
#define rwBLENDSRCCOLOR 3
#define rwBLENDINVSRCCOLOR 4
#define rwBLENDSRCALPHA 5
#define rwBLENDINVSRCALPHA 6
#define rwBLENDDESTALPHA 7
#define rwBLENDINVDESTALPHA 8
#define rwBLENDDESTCOLOR 9
#define rwBLENDINVDESTCOLOR 10
#define rwBLENDSRCALPHASAT 11
#define rwCULLMODECULLNONE 1
#define rwCULLMODECULLBACK 2

/* a texture's filter mode and addressing in its filterAddressing word */
#define rwTEXTUREFILTERMODEMASK 0x000000FF
#define rwTEXTUREADDRESSINGUMASK 0x00000F00
#define rwTEXTUREADDRESSINGVMASK 0x0000F000
#define rwTEXTUREADDRESSINGMASK 0x0000FF00

/* A texture's filter mode and addressing; the getters return the (int-sized) RenderWare enum
 * values, so they compare signed */
#define RwTextureSetFilterMode(texture, filtering)                                                 \
    ((texture)->filterAddressing = ((texture)->filterAddressing & ~rwTEXTUREFILTERMODEMASK) |      \
                                   ((filtering) & rwTEXTUREFILTERMODEMASK))
#define RwTextureGetFilterMode(texture)                                                            \
    ((RwInt32)((texture)->filterAddressing & rwTEXTUREFILTERMODEMASK))
#define RwTextureSetAddressing(texture, addressing)                                                \
    ((texture)->filterAddressing = ((texture)->filterAddressing & ~rwTEXTUREADDRESSINGMASK) |      \
                                   ((((addressing) << 8) & rwTEXTUREADDRESSINGUMASK) |             \
                                    (((addressing) << 12) & rwTEXTUREADDRESSINGVMASK)))
#define RwTextureSetAddressingU(texture, addressing)                                               \
    ((texture)->filterAddressing = ((texture)->filterAddressing & ~rwTEXTUREADDRESSINGUMASK) |     \
                                   (((addressing) << 8) & rwTEXTUREADDRESSINGUMASK))
#define RwTextureSetAddressingV(texture, addressing)                                               \
    ((texture)->filterAddressing = ((texture)->filterAddressing & ~rwTEXTUREADDRESSINGVMASK) |     \
                                   (((addressing) << 12) & rwTEXTUREADDRESSINGVMASK))
#define RwTextureGetAddressingU(texture)                                                           \
    ((RwInt32)(((texture)->filterAddressing & rwTEXTUREADDRESSINGUMASK) >> 8))
#define RwTextureGetAddressingV(texture)                                                           \
    ((RwInt32)(((texture)->filterAddressing & rwTEXTUREADDRESSINGVMASK) >> 12))

/* raster formats */
#define rwRASTERFORMATDEFAULT 0x0000
#define rwRASTERFORMAT1555 0x0100
#define rwRASTERFORMAT565 0x0200
#define rwRASTERFORMAT4444 0x0300
#define rwRASTERFORMATLUM8 0x0400
#define rwRASTERFORMAT8888 0x0500
#define rwRASTERFORMAT888 0x0600
#define rwRASTERFORMAT555 0x0A00
#define rwRASTERFORMATPIXELFORMATMASK 0x0F00
#define rwRASTERFORMATAUTOMIPMAP 0x1000
#define rwRASTERFORMATPAL8 0x2000
#define rwRASTERFORMATPAL4 0x4000
#define rwRASTERFORMATMIPMAP 0x8000
#define rwRASTERFORMATPALMASK (rwRASTERFORMATPAL8 | rwRASTERFORMATPAL4)
#define rwRASTERFORMATMASK 0xFF00

/* the raster flag of rasters created without their own pixels */
#define rwRASTERDONTALLOCATE 0x80

#define RwRasterGetFormat(raster) (((raster)->cFormat & (rwRASTERFORMATMASK >> 8)) << 8)

typedef struct RwIm2DVertex RwIm2DVertex;

/* The device the engine renders with (0x38 bytes) */
typedef struct RwDevice {
    /* 0x00 */ RwReal gammaCorrection;
    /* 0x04 */ RwBool (*fpSystem)(RwInt32 request, void *out, void *inOut, RwInt32 numIn);
    /* 0x08 */ RwReal zBufferNear; /* the screen depth of 2D immediate mode vertices */
    /* 0x0C */ RwReal zBufferFar;
    /* 0x10 */ RwBool (*fpRenderStateSet)(RwInt32 state, void *value);
    /* 0x14 */ RwBool (*fpRenderStateGet)(RwInt32 state, void *value);
    /* 0x18 */ RwBool (*fpIm2DRenderLine)(RwIm2DVertex *vertices, RwInt32 numVertices,
                                          RwInt32 vert1, RwInt32 vert2);
    /* 0x1C */ RwBool (*fpIm2DRenderTriangle)(RwIm2DVertex *vertices, RwInt32 numVertices,
                                              RwInt32 vert1, RwInt32 vert2, RwInt32 vert3);
    /* 0x20 */ RwBool (*fpIm2DRenderPrimitive)(RwInt32 primType, RwIm2DVertex *vertices,
                                               RwInt32 numVertices);
    /* 0x24 */ RwBool (*fpIm2DRenderIndexedPrimitive)(RwInt32 primType, RwIm2DVertex *vertices,
                                                      RwInt32 numVertices, RwUInt16 *indices,
                                                      RwInt32 numIndices);
    /* 0x28 */ RwBool (*fpIm3DRenderLine)(RwInt32 vert1, RwInt32 vert2);
    /* 0x2C */ RwBool (*fpIm3DRenderTriangle)(RwInt32 vert1, RwInt32 vert2, RwInt32 vert3);
    /* 0x30 */ RwBool (*fpIm3DRenderPrimitive)(RwInt32 primType);
    /* 0x34 */ RwBool (*fpIm3DRenderIndexedPrimitive)(RwInt32 primType, RwUInt16 *indices,
                                                      RwInt32 numIndices);
} RwDevice;
CHECK_SIZEOF(RwDevice, 0x38);

/* The string functions the engine uses (0x44 bytes) */
typedef struct RwStringFunctions {
    /* 0x00 */ RwInt32 (*vecSprintf)(RwChar *buffer, const RwChar *format, ...);
    /* 0x04 */ RwInt32 (*vecVsprintf)(RwChar *buffer, const RwChar *format, va_list argptr);
    /* 0x08 */ RwChar *(*vecStrcpy)(RwChar *dst, const RwChar *src);
    /* 0x0C */ RwChar *(*vecStrncpy)(RwChar *dst, const RwChar *src, size_t n);
    /* 0x10 */ RwChar *(*vecStrcat)(RwChar *dst, const RwChar *src);
    /* 0x14 */ RwChar *(*vecStrncat)(RwChar *dst, const RwChar *src, size_t n);
    /* 0x18 */ RwChar *(*vecStrrchr)(const RwChar *string, RwInt32 findThis);
    /* 0x1C */ RwChar *(*vecStrchr)(const RwChar *string, RwInt32 findThis);
    /* 0x20 */ RwChar *(*vecStrstr)(const RwChar *string, const RwChar *findThis);
    /* 0x24 */ RwInt32 (*vecStrcmp)(const RwChar *string1, const RwChar *string2);
    /* 0x28 */ RwInt32 (*vecStrncmp)(const RwChar *string1, const RwChar *string2, size_t n);
    /* 0x2C */ RwInt32 (*vecStricmp)(const RwChar *string1, const RwChar *string2);
    /* 0x30 */ size_t (*vecStrlen)(const RwChar *string);
    /* 0x34 */ RwChar *(*vecStrupr)(RwChar *string);
    /* 0x38 */ RwChar *(*vecStrlwr)(RwChar *string);
    /* 0x3C */ RwChar *(*vecStrtok)(RwChar *string, const RwChar *delimit);
    /* 0x40 */ RwInt32 (*vecSscanf)(const RwChar *buffer, const RwChar *format, ...);
} RwStringFunctions;
CHECK_SIZEOF(RwStringFunctions, 0x44);

/* The memory functions the engine allocates with */
typedef struct RwMemoryFunctions {
    /* 0x0 */ void *(*rwmalloc)(RwUInt32 size, RwUInt32 hint);
    /* 0x4 */ void (*rwfree)(void *mem);
    /* 0x8 */ void *(*rwrealloc)(void *mem, RwUInt32 newSize, RwUInt32 hint);
    /* 0xC */ void *(*rwcalloc)(RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint);
} RwMemoryFunctions;
CHECK_SIZEOF(RwMemoryFunctions, 0x10);

/* A link of a doubly linked list */
typedef struct RwLLLink {
    struct RwLLLink *next;
    struct RwLLLink *prev;
} RwLLLink;
CHECK_SIZEOF(RwLLLink, 0x8);

/* A doubly linked list: its link is the head and the tail sentinel */
typedef struct RwLinkList {
    RwLLLink link;
} RwLinkList;
CHECK_SIZEOF(RwLinkList, 0x8);

/* A device's standard function (RwStandard): out, inOut and numIn depend on the function */
typedef RwBool (*RwStandardFunc)(void *out, void *inOut, RwInt32 numIn);

/* The engine globals; only the current camera and world, the frame counters, the open device's
 * near screen depth, render state and 2D indexed primitive functions, the standard functions,
 * the dirty frames, the string, memory and free list functions and the engine status are
 * mapped. */
typedef struct RwGlobals {
    /* 0x000 */ RwCamera *curCamera;
    /* 0x004 */ void *curWorld;
    /* 0x008 */ RwUInt16 renderFrame;
    /* 0x00A */ RwUInt16 lightFrame; /* lights are created one frame behind it */
    /* 0x00C */ RwUInt8 field_0xC[0x4];
    /* 0x010 */ RwDevice dOpenDevice;
    /* 0x048 */ RwStandardFunc stdFunc[0x1D];
    /* 0x0BC */ RwLinkList dirtyFrameList; /* the roots of hierarchies that need synchronizing */
    /* 0x0C4 */ RwStringFunctions stringFuncs;
    /* 0x108 */ RwMemoryFunctions memoryFuncs;
    /* 0x118 */ void *(*rwfreelistalloc)(RwFreeList *freeList, RwUInt32 hint);
    /* 0x11C */ RwFreeList *(*rwfreelistfree)(RwFreeList *freeList, void *mem);
    /* 0x120 */ RwUInt8 field_0x120[0x4];
    /* 0x124 */ RwInt32 engineStatus;      /* rwENGINESTATUS* */
    /* 0x128 */ RwUInt32 resArenaInitSize; /* the bytes of the resources heap (inferred) */
} RwGlobals;
CHECK_SIZEOF(RwGlobals, 0x12C);

extern RwGlobals *lbl_805F15B4; /* the engine instance */

#define RwMalloc(size, hint) (lbl_805F15B4->memoryFuncs.rwmalloc((size), (hint)))
#define RwFree(mem) (lbl_805F15B4->memoryFuncs.rwfree(mem))
#define RwRealloc(mem, newSize, hint)                                                              \
    (lbl_805F15B4->memoryFuncs.rwrealloc((mem), (newSize), (hint)))
#define RwCalloc(numObj, sizeObj, hint)                                                            \
    (lbl_805F15B4->memoryFuncs.rwcalloc((numObj), (sizeObj), (hint)))
#define RwFreeListAlloc(freeList, hint) (lbl_805F15B4->rwfreelistalloc((freeList), (hint)))
#define RwFreeListFree(freeList, mem) (lbl_805F15B4->rwfreelistfree((freeList), (mem)))
#define rwstrlen(string) (lbl_805F15B4->stringFuncs.vecStrlen(string))
#define rwstrcpy(dst, src) (lbl_805F15B4->stringFuncs.vecStrcpy((dst), (src)))
#define rwstrncpy(dst, src, n) (lbl_805F15B4->stringFuncs.vecStrncpy((dst), (src), (n)))
#define rwstrcat(dst, src) (lbl_805F15B4->stringFuncs.vecStrcat((dst), (src)))
#define rwstrncat(dst, src, n) (lbl_805F15B4->stringFuncs.vecStrncat((dst), (src), (n)))
#define rwstrrchr(string, findThis) (lbl_805F15B4->stringFuncs.vecStrrchr((string), (findThis)))
#define rwstrchr(string, findThis) (lbl_805F15B4->stringFuncs.vecStrchr((string), (findThis)))
#define rwstrcmp(string1, string2) (lbl_805F15B4->stringFuncs.vecStrcmp((string1), (string2)))
#define rwstricmp(string1, string2) (lbl_805F15B4->stringFuncs.vecStricmp((string1), (string2)))
#define rwstrupr(string) (lbl_805F15B4->stringFuncs.vecStrupr(string))
#define rwstrlwr(string) (lbl_805F15B4->stringFuncs.vecStrlwr(string))
#define rwsprintf lbl_805F15B4->stringFuncs.vecSprintf
#define rwsscanf lbl_805F15B4->stringFuncs.vecSscanf
#define RwRenderStateSet(state, value)                                                             \
    (lbl_805F15B4->dOpenDevice.fpRenderStateSet((state), (value)))
#define RwRenderStateGet(state, value)                                                             \
    (lbl_805F15B4->dOpenDevice.fpRenderStateGet((state), (value)))
#define RwIm2DGetNearScreenZ() (lbl_805F15B4->dOpenDevice.zBufferNear)
#define RwIm2DRenderIndexedPrimitive(primType, vertices, numVertices, indices, numIndices)         \
    (lbl_805F15B4->dOpenDevice.fpIm2DRenderIndexedPrimitive((primType), (vertices), (numVertices), \
                                                            (indices), (numIndices)))

/* A module's registration with the engine: where its globals are and how many engines have
 * opened it */
typedef struct RwModuleInfo {
    /* 0x0 */ RwInt32 globalsOffset;
    /* 0x4 */ RwInt32 numInstances;
} RwModuleInfo;
CHECK_SIZEOF(RwModuleInfo, 0x8);

/* plugin data at offset in an object (offsets are assigned at plugin registration) */
#define RWPLUGINOFFSET(type, base, offset) ((type *)((RwUInt8 *)(base) + (offset)))

typedef struct RwStream RwStream;

/* plugin callbacks: object construction, destruction and copying, and the plugin's stream chunk */
typedef void *(*RwPluginObjectConstructor)(void *object, RwInt32 offset, RwInt32 size);
typedef void *(*RwPluginObjectDestructor)(void *object, RwInt32 offset, RwInt32 size);
typedef void *(*RwPluginObjectCopy)(void *dst, const void *src, RwInt32 offset, RwInt32 size);
typedef RwStream *(*RwPluginDataChunkReadCallBack)(RwStream *stream, RwInt32 length, void *object,
                                                   RwInt32 offset, RwInt32 size);
typedef RwStream *(*RwPluginDataChunkWriteCallBack)(RwStream *stream, RwInt32 length,
                                                    const void *object, RwInt32 offset,
                                                    RwInt32 size);
typedef RwInt32 (*RwPluginDataChunkGetSizeCallBack)(const void *object, RwInt32 offset,
                                                    RwInt32 size);

typedef struct RwFrame RwFrame;

typedef RwFrame *(*RwFrameCallBack)(RwFrame *frame, void *data);

/* An error reported with fn_8047DB84: the reporting plugin and its error code. */
typedef struct RwError {
    RwInt32 pluginID;
    RwInt32 errorCode;
} RwError;
CHECK_SIZEOF(RwError, 0x8);

typedef struct RwV3d {
    RwReal x;
    RwReal y;
    RwReal z;
} RwV3d;
CHECK_SIZEOF(RwV3d, 0xC);

typedef struct RwBBox {
    RwV3d sup;
    RwV3d inf;
} RwBBox;
CHECK_SIZEOF(RwBBox, 0x18);

RwBBox *fn_8048349C(RwBBox *boundBox, const RwV3d *verts, RwInt32 numVerts); /* calculates */

typedef struct RwRGBA {
    RwUInt8 red;
    RwUInt8 green;
    RwUInt8 blue;
    RwUInt8 alpha;
} RwRGBA;
CHECK_SIZEOF(RwRGBA, 0x4);

typedef struct RwTexCoords {
    RwReal u;
    RwReal v;
} RwTexCoords;
CHECK_SIZEOF(RwTexCoords, 0x8);

typedef struct RwV2d {
    RwReal x;
    RwReal y;
} RwV2d;
CHECK_SIZEOF(RwV2d, 0x8);

typedef struct RwRGBAReal {
    RwReal red;
    RwReal green;
    RwReal blue;
    RwReal alpha;
} RwRGBAReal;
CHECK_SIZEOF(RwRGBAReal, 0x10);

/* converts a color with components 0 to 1 to bytes, rounding */
#define RwRGBAFromRwRGBAReal(o, i)                                                                 \
    do {                                                                                           \
        (o)->red = (RwUInt8)(RwInt32)((i)->red * 255.0f + 0.5f);                                   \
        (o)->green = (RwUInt8)(RwInt32)((i)->green * 255.0f + 0.5f);                               \
        (o)->blue = (RwUInt8)(RwInt32)((i)->blue * 255.0f + 0.5f);                                 \
        (o)->alpha = (RwUInt8)(RwInt32)((i)->alpha * 255.0f + 0.5f);                               \
    } while (0)

/* converts a color with byte components to components 0 to 1 */
#define RwRGBARealFromRwRGBA(o, i)                                                                 \
    do {                                                                                           \
        (o)->red = (RwReal)(i)->red * (1.0f / 255.0f);                                             \
        (o)->green = (RwReal)(i)->green * (1.0f / 255.0f);                                         \
        (o)->blue = (RwReal)(i)->blue * (1.0f / 255.0f);                                           \
        (o)->alpha = (RwReal)(i)->alpha * (1.0f / 255.0f);                                         \
    } while (0)

/* o = a * scale, by component */
#define RwRGBARealScale(o, a, scale)                                                               \
    do {                                                                                           \
        (o)->red = (a)->red * (scale);                                                             \
        (o)->green = (a)->green * (scale);                                                         \
        (o)->blue = (a)->blue * (scale);                                                           \
        (o)->alpha = (a)->alpha * (scale);                                                         \
    } while (0)

/* o = a + b, by component */
#define RwRGBARealAdd(o, a, b)                                                                     \
    do {                                                                                           \
        (o)->red = (a)->red + (b)->red;                                                            \
        (o)->green = (a)->green + (b)->green;                                                      \
        (o)->blue = (a)->blue + (b)->blue;                                                         \
        (o)->alpha = (a)->alpha + (b)->alpha;                                                      \
    } while (0)

/* A 16.16 fixed point number */
typedef RwInt32 RwFixed;

/* The header of every engine object: its type and the object it is attached to */
typedef struct RwObject {
    /* 0x0 */ RwUInt8 type;
    /* 0x1 */ RwUInt8 subType;
    /* 0x2 */ RwUInt8 flags;
    /* 0x3 */ RwUInt8 privateFlags;
    /* 0x4 */ void *parent;
} RwObject;
CHECK_SIZEOF(RwObject, 0x8);

/* A matrix: three axes and a position (0x40 bytes) */
typedef struct RwMatrix {
    /* 0x00 */ RwV3d right;
    /* 0x0C */ RwUInt32 flags;
    /* 0x10 */ RwV3d up;
    /* 0x1C */ RwUInt32 pad1;
    /* 0x20 */ RwV3d at;
    /* 0x2C */ RwUInt32 pad2;
    /* 0x30 */ RwV3d pos;
    /* 0x3C */ RwUInt32 pad3;
} RwMatrix;
CHECK_SIZEOF(RwMatrix, 0x40);

/* A camera; its parent object is its frame */

/* A rectangle */
typedef struct RwRect {
    RwInt32 x;
    RwInt32 y;
    RwInt32 w;
    RwInt32 h;
} RwRect;
CHECK_SIZEOF(RwRect, 0x10);

/* A raster; a sub-raster's parent is the raster that owns its pixels */
typedef struct RwRaster {
    /* 0x00 */ struct RwRaster *parent;
    /* 0x04 */ RwUInt8 *cpPixels;
    /* 0x08 */ RwUInt8 *palette;
    /* 0x0C */ RwInt32 width;
    /* 0x10 */ RwInt32 height;
    /* 0x14 */ RwInt32 depth;
    /* 0x18 */ RwInt32 stride;
    /* 0x1C */ RwInt16 nOffsetX;
    /* 0x1E */ RwInt16 nOffsetY;
    /* 0x20 */ RwUInt8 cType;
    /* 0x21 */ RwUInt8 cFlags;
    /* 0x22 */ RwUInt8 privateFlags;
    /* 0x23 */ RwUInt8 cFormat; /* the rwRASTERFORMAT* bits above the low byte */
    /* 0x24 */ RwUInt8 *originalPixels;
    /* 0x28 */ RwInt32 originalWidth;
    /* 0x2C */ RwInt32 originalHeight;
    /* 0x30 */ RwInt32 originalStride;
} RwRaster;
CHECK_SIZEOF(RwRaster, 0x34);

/* A free list: fixed size entries allocated in blocks. Each block is a link in blockList, then a
 * bitmap with a bit per entry, set while the entry is in use, then the entries at the alignment. */
struct RwFreeList {
    /* 0x00 */ RwUInt32 entrySize; /* rounded up to the alignment */
    /* 0x04 */ RwUInt32 entriesPerBlock;
    /* 0x08 */ RwUInt32 heapSize; /* the bytes of each block's bitmap */
    /* 0x0C */ RwUInt32 alignment;
    /* 0x10 */ RwLinkList blockList;
    /* 0x18 */ RwUInt32 flags;
    /* 0x1C */ RwLLLink link; /* in the list of all free lists */
};
CHECK_SIZEOF(struct RwFreeList, 0x24);

/* A plane: points p with dot(normal, p) == distance */
typedef struct RwPlane {
    /* 0x0 */ RwV3d normal;
    /* 0xC */ RwReal distance;
} RwPlane;
CHECK_SIZEOF(RwPlane, 0x10);

/* A plane of a camera's view frustum, with the corner of a box nearest to it (0x14 bytes) */
typedef struct RwFrustumPlane {
    /* 0x00 */ RwPlane plane;
    /* 0x10 */ RwUInt8 closestX; /* each 0 or 1: which of a box's two corners */
    /* 0x11 */ RwUInt8 closestY;
    /* 0x12 */ RwUInt8 closestZ;
    /* 0x13 */ RwUInt8 pad;
} RwFrustumPlane;
CHECK_SIZEOF(RwFrustumPlane, 0x14);

typedef void *(*RwObjectHasFrameSyncFunction)(void *object);

/* The header of an object attached to a frame: it is in the frame's object list, and its sync
 * function runs when the frame's hierarchy is synchronized */
typedef struct RwObjectHasFrame {
    /* 0x00 */ RwObject object;
    /* 0x08 */ RwLLLink lFrame;
    /* 0x10 */ RwObjectHasFrameSyncFunction sync;
} RwObjectHasFrame;
CHECK_SIZEOF(RwObjectHasFrame, 0x14);
typedef RwCamera *(*RwCameraBeginUpdateFunc)(RwCamera *camera);
typedef RwCamera *(*RwCameraEndUpdateFunc)(RwCamera *camera);

/* A camera; only the members in use are mapped */
struct RwCamera {
    /* 0x000 */ RwObject object; /* the parent is the camera's frame */
    /* 0x008 */ RwLLLink lFrame;
    /* 0x010 */ RwObjectHasFrameSyncFunction sync;
    /* 0x014 */ RwInt32 projectionType;
    /* 0x018 */ RwCameraBeginUpdateFunc beginUpdate;
    /* 0x01C */ RwCameraEndUpdateFunc endUpdate;
    /* 0x020 */ RwMatrix viewMatrix; /* from the world into the camera's view */
    /* 0x060 */ RwRaster *frameBuffer;
    /* 0x064 */ RwRaster *zBuffer;
    /* 0x068 */ RwV2d viewWindow;
    /* 0x070 */ RwV2d recipViewWindow;
    /* 0x078 */ RwV2d viewOffset;
    /* 0x080 */ RwReal nearPlane;
    /* 0x084 */ RwReal farPlane;
    /* 0x088 */ RwReal fogPlane;
    /* 0x08C */ RwReal zScale; /* maps a reciprocal depth into the device's depth range */
    /* 0x090 */ RwReal zShift;
    /* 0x094 */ RwFrustumPlane frustumPlanes[6];
    /* 0x10C */ RwBBox frustumBoundBox;
    /* 0x124 */ RwV3d frustumCorners[8];
};
CHECK_SIZEOF(struct RwCamera, 0x184);

/* Makes a list empty */
#define rwLinkListInitialize(list)                                                                 \
    do {                                                                                           \
        (list)->link.next = &(list)->link;                                                         \
        (list)->link.prev = &(list)->link;                                                         \
    } while (0)

/* Adds a link at the head of a list */
#define rwLinkListAddLLLink(list, linkvar)                                                         \
    do {                                                                                           \
        (linkvar)->next = (list)->link.next;                                                       \
        (linkvar)->prev = &(list)->link;                                                           \
        (list)->link.next->prev = (linkvar);                                                       \
        (list)->link.next = (linkvar);                                                             \
    } while (0)

/* Takes a link out of its list */
#define rwLinkListRemoveLLLink(linkvar)                                                            \
    do {                                                                                           \
        (linkvar)->prev->next = (linkvar)->next;                                                   \
        (linkvar)->next->prev = (linkvar)->prev;                                                   \
    } while (0)

/* The plugin extensions of an object type (0x18 bytes) */
typedef struct RwPluginRegistry {
    /* 0x00 */ RwInt32 sizeOfStruct; /* with the plugins' data */
    /* 0x04 */ RwInt32 origSizeOfStruct;
    /* 0x08 */ RwInt32 maxSizeOfStruct;
    /* 0x0C */ RwInt32 staticAlloc;
    /* 0x10 */ struct RwPluginRegEntry *firstRegEntry;
    /* 0x14 */ struct RwPluginRegEntry *lastRegEntry;
} RwPluginRegistry;
CHECK_SIZEOF(RwPluginRegistry, 0x18);

typedef RwBool (*RwPluginDataChunkAlwaysCallBack)(void *object, RwInt32 offset, RwInt32 size);
typedef RwBool (*RwPluginDataChunkRightsCallBack)(void *object, RwInt32 offset, RwInt32 size,
                                                  RwUInt32 extraData);

/* A plugin's registration in a registry (0x3C bytes) */
typedef struct RwPluginRegEntry {
    /* 0x00 */ RwInt32 offset; /* of the plugin's data in the object */
    /* 0x04 */ RwInt32 size;
    /* 0x08 */ RwUInt32 pluginID;
    /* 0x0C */ RwPluginDataChunkReadCallBack readCB;
    /* 0x10 */ RwPluginDataChunkWriteCallBack writeCB;
    /* 0x14 */ RwPluginDataChunkGetSizeCallBack getSizeCB;
    /* 0x18 */ RwPluginDataChunkAlwaysCallBack alwaysCB;
    /* 0x1C */ RwPluginDataChunkRightsCallBack rightsCB;
    /* 0x20 */ RwPluginObjectConstructor constructCB;
    /* 0x24 */ RwPluginObjectDestructor destructCB;
    /* 0x28 */ RwPluginObjectCopy copyCB;
    /* 0x2C */ void *errStrCB;
    /* 0x30 */ struct RwPluginRegEntry *nextRegEntry;
    /* 0x34 */ struct RwPluginRegEntry *prevRegEntry;
    /* 0x38 */ RwPluginRegistry *parentRegistry;
} RwPluginRegEntry;
CHECK_SIZEOF(RwPluginRegEntry, 0x3C);

/* plugin registries (inferred) */
/* each returns the registry, or NULL on failure */
/* constructs the plugins' data */
RwPluginRegistry *fn_8048209C(const RwPluginRegistry *registry, void *object);
/* reads an object's plugin data chunks */
RwPluginRegistry *fn_804816AC(RwPluginRegistry *registry, RwStream *stream, void *object);
/* calls the rights callback of the plugin with the ID */
RwPluginRegistry *fn_80481870(const RwPluginRegistry *registry, RwUInt32 pluginID, void *object,
                              RwUInt32 extraData);
/* destructs and copies the plugins' data */
RwPluginRegistry *fn_80482144(const RwPluginRegistry *registry, void *object);
RwPluginRegistry *fn_804821B0(const RwPluginRegistry *registry, void *dst, const void *src);
/* registering plugins: the plugin's offset in the object (or stream), or -1 */
RwInt32 fn_80481DC8(RwPluginRegistry *registry, RwInt32 size, RwUInt32 pluginID,
                    RwPluginObjectConstructor constructor, RwPluginObjectDestructor destructor,
                    RwPluginObjectCopy copy);
RwInt32 fn_804815F0(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkReadCallBack read, RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize);
RwInt32 fn_80481634(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkAlwaysCallBack callBack);
RwInt32 fn_80481670(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkRightsCallBack callBack);
RwInt32 fn_80481D98(RwPluginRegistry *registry, RwUInt32 pluginID); /* the plugin's offset */
/* the size of an object's plugin data chunks */
RwInt32 fn_804818FC(const RwPluginRegistry *registry, const void *object);
/* writes and skips an object's plugin data chunks */
const RwPluginRegistry *fn_80481980(const RwPluginRegistry *registry, RwStream *stream,
                                    const void *object);
const RwPluginRegistry *fn_80481AD4(const RwPluginRegistry *registry, RwStream *stream);
RwBool fn_80481B98(void); /* opens plugin registration */
RwBool fn_80481C58(void); /* closes plugin registration */

RwUInt32 fn_80485D64(void); /* the number of engine instances (inferred) */

/* The pipeline heap: a scratch heap that pipeline nodes allocate packet data from during one
 * execution. It is carved out of superblocks; every block, used or free, starts with a header. */
typedef struct rxHeapFreeBlock rxHeapFreeBlock;
typedef struct rxHeapSuperBlockDescriptor rxHeapSuperBlockDescriptor;
typedef struct rxHeapBlockHeader rxHeapBlockHeader;

/* An entry of the heap's free block table */
struct rxHeapFreeBlock {
    /* 0x0 */ RwUInt32 size;
    /* 0x4 */ rxHeapBlockHeader *ptr;
};
CHECK_SIZEOF(struct rxHeapFreeBlock, 0x8);

/* A superblock: one allocation the heap carves its blocks out of */
struct rxHeapSuperBlockDescriptor {
    /* 0x0 */ void *start;
    /* 0x4 */ RwUInt32 size;
    /* 0x8 */ rxHeapSuperBlockDescriptor *next;
};
CHECK_SIZEOF(struct rxHeapSuperBlockDescriptor, 0xC);

typedef struct RxHeap {
    /* 0x00 */ RwUInt32 superBlockSize; /* how much the heap grows by */
    /* 0x04 */ rxHeapSuperBlockDescriptor *head;
    /* 0x08 */ rxHeapBlockHeader *headBlock;
    /* 0x0C */ rxHeapFreeBlock *freeBlocks;
    /* 0x10 */ RwUInt32 entriesAlloced; /* of the free block table */
    /* 0x14 */ RwUInt32 entriesUsed;
    /* 0x18 */ RwBool dirty; /* whether anything was allocated since the last reset */
} RxHeap;
CHECK_SIZEOF(RxHeap, 0x1C);

/* The header of a heap block (0x20 bytes, so that block data stays 32-byte aligned) */
struct rxHeapBlockHeader {
    /* 0x00 */ rxHeapBlockHeader *prev;
    /* 0x04 */ rxHeapBlockHeader *next;
    /* 0x08 */ RwUInt32 size;
    /* 0x0C */ rxHeapFreeBlock *freeEntry; /* NULL while the block is in use */
    /* 0x10 */ RwUInt32 pad[4];
};
CHECK_SIZEOF(struct rxHeapBlockHeader, 0x20);

typedef struct RxPipeline RxPipeline;
typedef struct RxPipelineNode RxPipelineNode;
typedef struct RxNodeDefinition RxNodeDefinition;

/* A cluster definition: a named kind of per-vertex or per-primitive data that flows through a
 * pipeline */
typedef struct RxClusterDefinition {
    /* 0x0 */ RwChar *name;
    /* 0x4 */ RwUInt32 defaultStride;
    /* 0x8 */ RwUInt32 defaultAttributes;
    /* 0xC */ const RwChar *attributeSet;
} RxClusterDefinition;
CHECK_SIZEOF(RxClusterDefinition, 0x10);

/* What a node requires of a cluster on input (RxClusterValidityReq) */
#define rxCLREQ_DONTWANT 0
#define rxCLREQ_REQUIRED 1
#define rxCLREQ_OPTIONAL 2

/* What becomes of a cluster on one of a node's outputs (RxClusterValid) */
#define rxCLVALID_NOCHANGE 0
#define rxCLVALID_VALID 1
#define rxCLVALID_INVALID 2

/* One of a node's outputs: the state of its clusters of interest, and of all other clusters */
typedef struct RxOutputSpec {
    /* 0x0 */ RwChar *name;
    /* 0x4 */ RwInt32 *outputClusters; /* rxCLVALID_* per cluster of interest */
    /* 0x8 */ RwInt32 allOtherClusters;
} RxOutputSpec;
CHECK_SIZEOF(RxOutputSpec, 0xC);

/* A cluster a node is interested in */
typedef struct RxClusterRef {
    /* 0x0 */ RxClusterDefinition *clusterDef;
    /* 0x4 */ RwBool forcePresent; /* create the cluster even when no earlier node does */
    /* 0x8 */ RwUInt32 reserved;
} RxClusterRef;
CHECK_SIZEOF(RxClusterRef, 0xC);

/* A node's inputs and outputs */
typedef struct RxIoSpec {
    /* 0x00 */ RwUInt32 numClustersOfInterest;
    /* 0x04 */ RxClusterRef *clustersOfInterest;
    /* 0x08 */ RwInt32 *inputRequirements; /* rxCLREQ_* per cluster of interest */
    /* 0x0C */ RwUInt32 numOutputs;
    /* 0x10 */ RxOutputSpec *outputs;
} RxIoSpec;
CHECK_SIZEOF(RxIoSpec, 0x14);

/* A pipeline cluster: a cluster of interest as one pipeline uses it */
typedef struct RxPipelineCluster {
    /* 0x0 */ RxClusterDefinition *clusterRef;
    /* 0x4 */ RwUInt32 creationAttributes;
} RxPipelineCluster;
CHECK_SIZEOF(RxPipelineCluster, 0x8);

/* RxCluster flags */
#define rxCLFLAGS_CLUSTERVALID 0x0001
#define rxCLFLAGS_EXTERNAL 0x0002 /* the data is not the heap's, so it is not freed */

/* A cluster of a packet: its data, the elements allocated and used, and a cursor */
typedef struct RxCluster {
    /* 0x00 */ RwUInt16 flags; /* rxCLFLAGS_* */
    /* 0x02 */ RwUInt16 stride;
    /* 0x04 */ void *data;
    /* 0x08 */ void *currentData;
    /* 0x0C */ RwUInt32 numAlloced;
    /* 0x10 */ RwUInt32 numUsed;
    /* 0x14 */ RxPipelineCluster *clusterRef; /* NULL while the slot is empty */
    /* 0x18 */ RwUInt32 attributes;
} RxCluster;
CHECK_SIZEOF(RxCluster, 0x1C);

/* A packet: what passes from node to node during an execution. It has a cluster for each slot of
 * the pipeline, so it is allocated with numClusters clusters. */
typedef struct RxPacket {
    /* 0x00 */ RwUInt16 flags;
    /* 0x02 */ RwUInt16 numClusters;
    /* 0x04 */ RxPipeline *pipeline;
    /* 0x08 */ RwUInt32 *inputToClusterSlot;
    /* 0x0C */ RwUInt32 *slotsContinue;
    /* 0x10 */ RxPipelineCluster **slotClusterRefs;
    /* 0x14 */ RxCluster clusters[1];
} RxPacket;
CHECK_SIZEOF(RxPacket, 0x30);

/* The parameters a node body receives */
typedef struct RxPipelineNodeParam {
    /* 0x0 */ void *dataParam; /* the object being rendered */
    /* 0x4 */ RxHeap *heap;    /* the heap of the execution */
} RxPipelineNodeParam;
CHECK_SIZEOF(RxPipelineNodeParam, 0x8);

/* the node methods */
typedef RwBool (*RxNodeBodyFn)(RxPipelineNode *self, const RxPipelineNodeParam *params);
typedef RwBool (*RxNodeInitFn)(RxNodeDefinition *self);
typedef void (*RxNodeTermFn)(RxNodeDefinition *self);
typedef RwBool (*RxPipelineNodeInitFn)(RxPipelineNode *self);
typedef void (*RxPipelineNodeTermFn)(RxPipelineNode *self);
typedef RwBool (*RxPipelineNodeConfigFn)(RxPipelineNode *self, RxPipeline *pipeline);
typedef RwUInt32 (*RxConfigMsgHandlerFn)(RxPipelineNode *self, RwUInt32 msg, RwUInt32 intparam,
                                         void *ptrparam);

/* What a node does: its body runs for every execution; the node definition is initialised when
 * the first unlocked pipeline uses it and terminated after the last, and each pipeline node is
 * initialised and terminated with its pipeline */
typedef struct RxNodeMethods {
    /* 0x00 */ RxNodeBodyFn nodeBody;
    /* 0x04 */ RxNodeInitFn nodeInit;
    /* 0x08 */ RxNodeTermFn nodeTerm;
    /* 0x0C */ RxPipelineNodeInitFn pipelineNodeInit;
    /* 0x10 */ RxPipelineNodeTermFn pipelineNodeTerm;
    /* 0x14 */ RxPipelineNodeConfigFn pipelineNodeConfig;
    /* 0x18 */ RxConfigMsgHandlerFn configMsgHandler;
} RxNodeMethods;
CHECK_SIZEOF(RxNodeMethods, 0x1C);

/* RxNodeDefEditable */
#define rxNODEDEFCONST FALSE
#define rxNODEDEFEDITABLE TRUE

/* The definition of a pipeline node */
struct RxNodeDefinition {
    /* 0x00 */ RwChar *name;
    /* 0x04 */ RxNodeMethods nodeMethods;
    /* 0x20 */ RxIoSpec io;
    /* 0x34 */ RwUInt32 pipelineNodePrivateDataSize;
    /* 0x38 */ RwBool editable;       /* rxNODEDEF*: whether the definition was allocated */
    /* 0x3C */ RwInt32 InputPipesCnt; /* the unlocked pipelines that use it */
};
CHECK_SIZEOF(struct RxNodeDefinition, 0x40);

/* the requirements of a node during the dependency pass (inferred) */
typedef struct rxReq rxReq;

/* What the dependency pass keeps for a node while it sorts the pipeline */
typedef struct RxPipelineNodeTopSortData {
    /* 0x0 */ RwUInt32 numIns;
    /* 0x4 */ RwUInt32 numInsVisited;
    /* 0x8 */ rxReq *req; /* the requirements being propagated (inferred) */
} RxPipelineNodeTopSortData;
CHECK_SIZEOF(RxPipelineNodeTopSortData, 0xC);

/* The most outputs a node can have; a locked pipeline gives each node an output array of this
 * size in its superblock */
#define RXNODEMAXOUTPUTS 32
#define RXNODEMAXCLUSTERSOFINTEREST 32 /* the most clusters of interest a node can have */

/* A node of a pipeline. The nodes of an unlocked pipeline are sorted so that every node comes
 * before the nodes its outputs lead to; outputs are node indices, -1 for none. */
struct RxPipelineNode {
    /* 0x00 */ RxNodeDefinition *nodeDef;
    /* 0x04 */ RwUInt32 numOutputs;
    /* 0x08 */ RwUInt32 *outputs;
    /* 0x0C */ RxPipelineCluster **slotClusterRefs; /* the pipeline cluster of each packet slot */
    /* 0x10 */ RwUInt32 *slotsContinue;             /* which slots persist through each output */
    /* 0x14 */ void *privateData;
    /* 0x18 */ RwUInt32 *inputToClusterSlot; /* the packet slot of each cluster of interest */
    /* 0x1C */ RxPipelineNodeTopSortData *topSortData;
    /* 0x20 */ void *initializationData; /* kept so the node can be initialised again */
    /* 0x24 */ RwUInt32 initializationDataSize;
};
CHECK_SIZEOF(struct RxPipelineNode, 0x28);

/* The state of a pipeline's embedded packet (rxEmbeddedPacketState) */
#define rxPKST_PACKETLESS 0 /* not created */
#define rxPKST_UNUSED 1     /* created and destroyed */
#define rxPKST_INUSE 2      /* created and not yet destroyed */
#define rxPKST_PENDING 3    /* dispatched and not yet fetched */

/* The pipeline being executed and its node */
typedef struct RxExecutionContext {
    /* 0x00 */ RxPipeline *pipeline;
    /* 0x04 */ RxPipelineNode *currentNode;
    /* 0x08 */ RwInt32 exitCode;
    /* 0x0C */ RwUInt32 pad;
    /* 0x10 */ RxPipelineNodeParam params;
} RxExecutionContext;
CHECK_SIZEOF(RxExecutionContext, 0x18);

/* A cluster an unlocked pipeline needs on input, and the packet slot it occupies */
typedef struct RxPipelineRequiresCluster {
    /* 0x0 */ RxClusterDefinition *clusterDef;
    /* 0x4 */ RwInt32 rqdOrOpt; /* rxCLREQ_* */
    /* 0x8 */ RwUInt32 slotIndex;
} RxPipelineRequiresCluster;
CHECK_SIZEOF(RxPipelineRequiresCluster, 0xC);

/* A rendering pipeline */
struct RxPipeline {
    /* 0x00 */ RwBool locked; /* whether it is locked for editing */
    /* 0x04 */ RwUInt32 numNodes;
    /* 0x08 */ RxPipelineNode *nodes;
    /* 0x0C */ RwUInt32 packetNumClusterSlots;
    /* 0x10 */ RwInt32 embeddedPacketState; /* rxPKST_* */
    /* 0x14 */ RxPacket *embeddedPacket;
    /* 0x18 */ RwUInt32 numInputRequirements;
    /* 0x1C */ RxPipelineRequiresCluster *inputRequirements;
    /* 0x20 */ void *superBlock; /* one allocation for the nodes' arrays and private data */
    /* 0x24 */ RwUInt32 superBlockSize;
    /* 0x28 */ RwUInt32 entryPoint; /* the index of the first node to execute */
    /* 0x2C */ RwUInt32 pluginId;   /* the plugin that owns it, 0 for none */
    /* 0x30 */ RwUInt32 pluginData; /* the owner plugin's data */
};
CHECK_SIZEOF(struct RxPipeline, 0x34);

/* RxRenderStateVector flags */
#define rxRENDERSTATEFLAG_TEXTUREPERSPECTIVE 0x00000001
#define rxRENDERSTATEFLAG_ZTESTENABLE 0x00000002
#define rxRENDERSTATEFLAG_ZWRITEENABLE 0x00000004
#define rxRENDERSTATEFLAG_VERTEXALPHAENABLE 0x00000008
#define rxRENDERSTATEFLAG_FOGENABLE 0x00000010

/* A complete set of the render states that pipelines render with */
typedef struct RxRenderStateVector {
    /* 0x00 */ RwUInt32 Flags; /* rxRENDERSTATEFLAG_* */
    /* 0x04 */ RwInt32 ShadeMode;
    /* 0x08 */ RwInt32 SrcBlend;
    /* 0x0C */ RwInt32 DestBlend;
    /* 0x10 */ RwRaster *TextureRaster;
    /* 0x14 */ RwInt32 AddressModeU;
    /* 0x18 */ RwInt32 AddressModeV;
    /* 0x1C */ RwInt32 FilterMode;
    /* 0x20 */ RwRGBA BorderColor;
    /* 0x24 */ RwInt32 FogType;
    /* 0x28 */ RwRGBA FogColor;
} RxRenderStateVector;
CHECK_SIZEOF(RxRenderStateVector, 0x2C);

/* The pipeline module's globals in the engine: the free list pipelines are allocated from, the
 * default render states, and the current, generic and platform pipelines of atomics, world sectors
 * and materials */
typedef struct rwPipeGlobals {
    /* 0x00 */ RwFreeList *pipesFreeList;
    /* 0x04 */ RxRenderStateVector defaultRenderState;
    /* 0x30 */ RwLinkList allPipelines; /* cleared when the module opens */
    /* 0x38 */ RwUInt32 maxNodesPerPipe;
    /* 0x3C */ RxPipeline *currentAtomicPipeline;
    /* 0x40 */ RxPipeline *currentWorldSectorPipeline;
    /* 0x44 */ RxPipeline *currentMaterialPipeline;
    /* 0x48 */ RxPipeline *genericAtomicPipeline;
    /* 0x4C */ RxPipeline *genericWorldSectorPipeline;
    /* 0x50 */ RxPipeline *genericMaterialPipeline;
    /* 0x54 */ RxPipeline *platformAtomicPipeline;
    /* 0x58 */ RxPipeline *platformWorldSectorPipeline;
    /* 0x5C */ RxPipeline *platformMaterialPipeline;
} rwPipeGlobals;
CHECK_SIZEOF(rwPipeGlobals, 0x60);

extern RwInt32 lbl_805F1690; /* engine plugin offset of the pipeline module's globals */

#define RXPIPELINEGLOBAL(var) (RWPLUGINOFFSET(rwPipeGlobals, lbl_805F15B4, lbl_805F1690)->var)

/* the heap pipelines execute with (inferred) */
extern RxHeap *lbl_805F169C;

/* the pipeline heap (inferred from their use) */
RxHeap *fn_8049F168(RwUInt32 size);        /* creates a heap of at least size bytes */
void fn_8049F0CC(RxHeap *heap);            /* destroys it */
RwBool fn_8049ECFC(RxHeap *heap);          /* frees every block; FALSE on failure */
void fn_8049EA2C(RxHeap *heap, void *ptr); /* frees a block */

/* Resets a heap if anything was allocated from it since its last reset; FALSE on failure */
#define RxHeapReset(heap) ((heap)->dirty == FALSE ? TRUE : fn_8049ECFC(heap))

/* destroys a packet (inferred) */
void fn_8049B14C(RxPacket *packet);

/* opening and closing the pipeline library (inferred); each returns TRUE on success */
RwBool fn_8049B258(void);
RwBool fn_8049B1F8(void);

/* creating, executing and destroying pipelines (inferred from their use) */
RxPipeline *fn_8049B76C(void);          /* creates a pipeline */
void fn_8049B800(RxPipeline *pipeline); /* destroys a pipeline */
/* destroys a node of a pipeline; in a locked pipeline the node is also removed from the node
 * array */
RxPipelineNode *fn_8049B350(RxPipelineNode *node, RxPipeline *pipeline);
RxHeap *fn_8049B66C(void); /* the heap pipelines execute with */
/* executes a pipeline on an object, first resetting the heap if heapReset is set; NULL on failure */
RxPipeline *fn_8049B674(RxPipeline *pipeline, void *data, RwBool heapReset);

/* editing pipelines (inferred from their use) */
RxPipeline *fn_8049CCB8(RxPipeline *pipeline); /* locks a pipeline for editing */
RxPipeline *fn_8049C1E0(RxPipeline *pipeline); /* unlocks it */
/* adds a fragment of the NULL-terminated node definitions to a locked pipeline */
RxPipeline *fn_8049D0D0(RxPipeline *pipeline, RwUInt32 *firstIndex, RxNodeDefinition *nodeDef, ...);
/* finds a pipeline node by name */
RxPipelineNode *fn_8049CFCC(RxPipeline *pipeline, const RwChar *name, RxPipelineNode *start,
                            RwInt32 *nodeIndex);
/* connects an unconnected output of a node of a locked pipeline to another of its nodes; NULL on
 * failure */
RxPipeline *fn_8049D368(RxPipeline *pipeline, RwUInt32 *output, RxPipelineNode *input);

/* the dependency pass of unlocking a pipeline (inferred): scratch space taken from the top and
 * the bottom of the free part of the superblock (NULL when it runs out), the number of distinct
 * cluster definitions of the nodes, and the pass itself, which returns 0 or the error code of
 * the problem it found */
void *fn_8049B8BC(RwUInt32 size);
void *fn_8049B92C(RwUInt32 size);
RwUInt32 fn_8049B9A4(RxPipeline *pipeline);
RwUInt32 fn_8049E9AC(RxPipeline *pipeline);

/* sorts numElements records of elementSize bytes into ascending order of the unsigned key word at
 * keyOffset in each, whose values lie between minKey and maxKey (inferred) */
void fn_8049F5B4(void *base, RwUInt32 numElements, RwUInt32 elementSize, RwUInt32 keyOffset,
                 RwUInt32 minKey, RwUInt32 maxKey);

/* sets a render state vector to the default render states; NULL on failure (inferred) */
RxRenderStateVector *fn_8049F2A4(RxRenderStateVector *rsvp);

typedef struct RwResEntry RwResEntry;
typedef void (*RwResEntryDestroyNotify)(RwResEntry *resEntry);

/* A resource entry: instanced data that the engine caches for an object, which follows it */
struct RwResEntry {
    /* 0x00 */ RwLLLink link; /* in the resources' use order */
    /* 0x08 */ RwInt32 size;
    /* 0x0C */ void *owner;
    /* 0x10 */ RwResEntry **ownerRef;
    /* 0x14 */ RwResEntryDestroyNotify destroyNotify;
};
CHECK_SIZEOF(struct RwResEntry, 0x18);

/* The resources module's globals in the engine. Entries are in one of two lists, used this frame
 * or before it; the lists swap roles each frame. */
typedef struct RwResourcesGlobals {
    /* 0x00 */ RwUInt32 maxSize;     /* the bytes of the heap */
    /* 0x04 */ RwUInt32 currentSize; /* the bytes of the entries in the heap */
    /* 0x08 */ RwUInt32 reusageSize; /* the bytes of entries used this frame that were freed */
    /* 0x0C */ void *memHeap;        /* NULL when the heap size is 0 */
    /* 0x10 */ RwLinkList entriesA;
    /* 0x18 */ RwLinkList entriesB;
    /* 0x20 */ RwLLLink *freeEntries; /* the head of the list of entries used before this frame */
    /* 0x24 */ RwLLLink *usedEntries; /* the head of the list used this frame, newest first */
} RwResourcesGlobals;
CHECK_SIZEOF(RwResourcesGlobals, 0x28);

extern RwModuleInfo lbl_805F1578; /* the resources module */

#define RWRESOURCESGLOBAL(var)                                                                     \
    (RWPLUGINOFFSET(RwResourcesGlobals, lbl_805F15B4, lbl_805F1578.globalsOffset)->var)

/* Moves a resource entry to the front of the use order */
#define RwResourcesUseResEntry(entry)                                                              \
    do {                                                                                           \
        if ((entry)->link.next != NULL) {                                                          \
            (entry)->link.prev->next = (entry)->link.next;                                         \
            (entry)->link.next->prev = (entry)->link.prev;                                         \
            (entry)->link.next = RWRESOURCESGLOBAL(usedEntries)->next;                             \
            (entry)->link.prev = RWRESOURCESGLOBAL(usedEntries);                                   \
            RWRESOURCESGLOBAL(usedEntries)->next->prev = &(entry)->link;                           \
            RWRESOURCESGLOBAL(usedEntries)->next = &(entry)->link;                                 \
        }                                                                                          \
    } while (0)

/* resources (inferred) */
RwResEntry *fn_80480878(void *owner, RwResEntry **ownerRef, RwInt32 size,
                        RwResEntryDestroyNotify destroyNotify); /* allocates an entry */
RwBool fn_8048071C(RwResEntry *entry);                          /* frees an entry */
void fn_804807CC(void);                                         /* starts a frame */
RwBool fn_80480B08(void);                                       /* frees all entries */

/* the resources heap (inferred) */
RwBool fn_80482984(void *heap, RwUInt32 size); /* initializes a heap in size bytes at heap */
RwBool fn_804829D4(void *heap);                /* destroys a heap */
void fn_804829DC(void *mem);                   /* frees a heap block */
void *fn_80482A94(void *heap, RwUInt32 size);  /* allocates a heap block */

/* The characters of a texture name, with its terminator */
#define rwTEXTUREBASENAMELENGTH 32

typedef struct RwTexDictionary RwTexDictionary;

/* A texture: its raster, the dictionary it is in, its names, filtering and addressing, and its
 * reference count (0x58 bytes) */
typedef struct RwTexture {
    /* 0x00 */ RwRaster *raster;
    /* 0x04 */ RwTexDictionary *dict;
    /* 0x08 */ RwLLLink lInDictionary;
    /* 0x10 */ RwChar name[rwTEXTUREBASENAMELENGTH];
    /* 0x30 */ RwChar mask[rwTEXTUREBASENAMELENGTH];
    /* 0x50 */ RwUInt32 filterAddressing; /* the filter mode in the low byte */
    /* 0x54 */ RwInt32 refCount;
} RwTexture;
CHECK_SIZEOF(RwTexture, 0x58);

/* A texture dictionary: the textures in it and its link in the list of dictionaries (0x18 bytes) */
struct RwTexDictionary {
    /* 0x00 */ RwObject object;
    /* 0x08 */ RwLinkList texturesInDict;
    /* 0x10 */ RwLLLink lInInstance;
};
CHECK_SIZEOF(struct RwTexDictionary, 0x18);

typedef struct RwLine {
    RwV3d start;
    RwV3d end;
} RwLine;
CHECK_SIZEOF(RwLine, 0x18);

/* A frame (while a hierarchy is being cloned, each frame's root holds its clone) */
struct RwFrame {
    /* 0x00 */ RwObject object;
    /* 0x08 */ RwLLLink inDirtyListLink;
    /* 0x10 */ RwMatrix modelling;
    /* 0x50 */ RwMatrix ltm;
    /* 0x90 */ RwLinkList objectList;
    /* 0x98 */ struct RwFrame *child;
    /* 0x9C */ struct RwFrame *next;
    /* 0xA0 */ struct RwFrame *root;
};
CHECK_SIZEOF(struct RwFrame, 0xA4);

/* The squared errors a matrix may have and still count as normal, orthogonal or the identity */
typedef struct RwMatrixTolerance {
    /* 0x0 */ RwReal Normal;
    /* 0x4 */ RwReal Orthogonal;
    /* 0x8 */ RwReal Identity;
} RwMatrixTolerance;
CHECK_SIZEOF(RwMatrixTolerance, 0xC);

/* optimises a matrix's flags, with a tolerance or the default one for NULL (inferred) */
RwMatrix *fn_8047E990(RwMatrix *matrix, const RwMatrixTolerance *tolerance);

#define rwMATRIXTYPEORTHONORMAL 0x3
#define rwMATRIXINTERNALIDENTITY 0x20000

#define RwMatrixSetIdentity(m)                                                                     \
    do {                                                                                           \
        (m)->right.x = (m)->up.y = (m)->at.z = 1.0;                                                \
        (m)->right.y = (m)->right.z = (m)->up.x = 0.0;                                             \
        (m)->up.z = (m)->at.x = (m)->at.y = 0.0;                                                   \
        (m)->pos.x = (m)->pos.y = (m)->pos.z = 0.0;                                                \
        (m)->flags |= rwMATRIXINTERNALIDENTITY | rwMATRIXTYPEORTHONORMAL;                          \
    } while (0)

/* how a matrix operation combines with the matrix (inferred) */
#define rwCOMBINEREPLACE 0
#define rwCOMBINEPRECONCAT 1
#define rwCOMBINEPOSTCONCAT 2

#define rwPARALLEL 2 /* the camera projection */

/* matrices (inferred) */
RwMatrix *fn_8047F994(void);             /* create */
RwBool fn_8047F958(RwMatrix *matrix);    /* destroy */
RwMatrix *fn_8047EBA0(RwMatrix *matrix); /* clears the identity and orthonormal flags */
RwMatrix *fn_8047F59C(RwMatrix *matrix, const RwV3d *translation, RwInt32 combine);
RwMatrix *fn_8047F398(RwMatrix *matrix, const RwV3d *scale, RwInt32 combine);
RwMatrix *fn_8047EBB8(RwMatrix *dst, const RwMatrix *matrixA,
                      const RwMatrix *matrixB);               /* multiply */
RwMatrix *fn_8047F0B0(RwMatrix *dst, const RwMatrix *matrix); /* invert */
RwMatrix *fn_8047EFC8(RwMatrix *matrix, const RwV3d *axis, RwReal angle, RwInt32 combine);
RwMatrix *fn_8047F72C(RwMatrix *matrix, const RwMatrix *transform, RwInt32 combine);
void fn_8047E100(RwMatrix *dst, const RwMatrix *matrixA, const RwMatrix *matrixB); /* multiply */
RwMatrix *fn_8047E298(RwMatrix *dst, const RwMatrix *matrix);    /* orthonormalizes */
RwReal fn_8047E704(const RwMatrix *matrix);                      /* the determinant */
RwReal fn_8047E750(const RwMatrix *matrix);                      /* the orthogonal error */
RwReal fn_8047E7A8(const RwMatrix *matrix);                      /* the normal error */
void *fn_8047E828(void *instance, RwInt32 offset, RwInt32 size); /* closes the module */
void *fn_8047E88C(void *instance, RwInt32 offset, RwInt32 size); /* opens the module */
RwBool fn_8047E960(const RwMatrixTolerance *tolerance);          /* sets the default tolerance */
RwMatrix *fn_8047ECB4(RwMatrix *dst, const RwMatrix *matrix);    /* orthonormalizes */
RwMatrix *fn_8047ECD4(RwMatrix *matrix, const RwV3d *unitAxis, RwInt32 combine,
                      RwReal oneMinusCosine, RwReal sine); /* rotates */

typedef struct RwSphere {
    RwV3d center;
    RwReal radius;
} RwSphere;
CHECK_SIZEOF(RwSphere, 0x10);

/* A stream chunk's header, decoded (0x14 bytes) */
typedef struct RwChunkHeaderInfo {
    /* 0x00 */ RwUInt32 type;
    /* 0x04 */ RwUInt32 length;
    /* 0x08 */ RwUInt32 version;
    /* 0x0C */ RwUInt32 buildNum;
    /* 0x10 */ RwBool isComplex; /* whether the chunk holds other chunks */
} RwChunkHeaderInfo;
CHECK_SIZEOF(RwChunkHeaderInfo, 0x14);

/* streams */
RwBool fn_8047CCF8(const RwChunkHeaderInfo *chunkHeaderInfo); /* whether a chunk is complex */
RwBool fn_8047CDB4(RwStream *stream, RwUInt32 *type, RwUInt32 *length, RwUInt32 *version,
                   RwUInt32 *buildNum); /* reads a chunk header */
RwStream *fn_8047CEF0(RwStream *stream, RwInt32 type, RwInt32 size, RwUInt32 version,
                      RwUInt32 build);
RwInt32 fn_8047CF50(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut);
void *fn_8047D158(void *mem, RwUInt32 size);
void *fn_8047D1C8(void *mem, RwUInt32 size);
void *fn_8047D254(void *mem, RwUInt32 size);
void *fn_8047D2C4(void *mem, RwUInt32 size);
/* Converts reals read from a stream to the engine real type, which already is a 32-bit float */
#define RwMemFloat32ToReal(mem, size) (mem)
RwStream *fn_8047D350(RwStream *stream, const RwReal *reals, RwUInt32 size);
RwStream *fn_8047D458(RwStream *stream, const RwInt32 *ints, RwUInt32 size);
RwStream *fn_8047D560(RwStream *stream, const RwInt16 *ints, RwUInt32 size);
RwStream *fn_8047D684(RwStream *stream, RwInt16 *ints, RwUInt32 size);
RwStream *fn_8047D784(RwStream *stream, RwReal *reals, RwUInt32 size);
RwStream *fn_8047D868(RwStream *stream, RwInt32 *ints, RwUInt32 size);
RwStream *fn_8047D94C(RwStream *stream, RwChunkHeaderInfo *chunkHeaderInfo);

/* RwStreamType */
#define rwSTREAMFILE 1     /* an open file */
#define rwSTREAMFILENAME 2 /* a file the stream opens by name */
#define rwSTREAMMEMORY 3
#define rwSTREAMCUSTOM 4

/* RwStreamAccessType */
#define rwSTREAMREAD 1
#define rwSTREAMWRITE 2
#define rwSTREAMAPPEND 3

/* A block of memory a memory stream reads, or receives what one wrote */
typedef struct RwMemory {
    /* 0x0 */ RwUInt8 *start;
    /* 0x4 */ RwUInt32 length;
} RwMemory;
CHECK_SIZEOF(RwMemory, 0x8);

/* A memory stream's state */
typedef struct RwStreamMemory {
    /* 0x0 */ RwUInt32 position;
    /* 0x4 */ RwUInt32 nSize; /* the bytes of memBlock */
    /* 0x8 */ RwUInt8 *memBlock;
} RwStreamMemory;
CHECK_SIZEOF(RwStreamMemory, 0xC);

/* A custom stream's functions and their data (0x14 bytes) */
typedef struct RwStreamCustom {
    /* 0x00 */ RwBool (*sfnclose)(void *data);
    /* 0x04 */ RwUInt32 (*sfnread)(void *data, void *buffer, RwUInt32 length);
    /* 0x08 */ RwBool (*sfnwrite)(void *data, const void *buffer, RwUInt32 length);
    /* 0x0C */ RwBool (*sfnskip)(void *data, RwUInt32 offset);
    /* 0x10 */ void *data;
} RwStreamCustom;
CHECK_SIZEOF(RwStreamCustom, 0x14);

/* A stream (0x24 bytes) */
struct RwStream {
    /* 0x00 */ RwInt32 type;       /* RwStreamType */
    /* 0x04 */ RwInt32 accessType; /* RwStreamAccessType */
    /* 0x08 */ RwInt32 position;
    /* 0x0C */ union {
        RwStreamMemory memory;
        void *fpFile; /* a file or file name stream's file */
        RwStreamCustom custom;
    } Type;
    /* 0x20 */ RwBool rwOwned; /* whether the stream is from the stream free list */
};
CHECK_SIZEOF(struct RwStream, 0x24);

RwUInt32 fn_80480D4C(RwStream *stream, void *buffer, RwUInt32 length);
RwStream *fn_80480ED0(RwStream *stream, const void *buffer, RwUInt32 length);
RwStream *fn_804810FC(RwStream *stream, RwUInt32 offset);
RwBool fn_8048124C(RwStream *stream, void *data); /* close; data receives a written memory block */
RwStream *fn_80481368(RwInt32 type, RwInt32 accessType, const void *data); /* open */

/* frames */
RwMatrix *RwFrameGetLTM(RwFrame *frame);

/* maths and intersection tests (inferred from their use) */
RwReal fn_80482340(RwReal x);                    /* sqrt(x) */
RwReal fn_80482394(RwReal x);                    /* 1 / sqrt(x) */
RwReal fn_8048222C(RwV3d *out, const RwV3d *in); /* normalizes, returning the length */
RwReal fn_80482300(const RwV3d *in);             /* the length of a vector */

/* whether a triangle meets a box */
RwBool fn_8047BDD0(const RwBBox *box, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2);

/* whether a triangle meets a sphere, with the triangle's normal and the distance of the sphere's
 * center from its plane */
RwBool fn_8047B720(const RwSphere *sphere, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2,
                   RwV3d *normal, RwReal *distance);

/* whether a sphere is in a camera's view frustum (inferred) */
RwInt32 fn_804856C8(const RwCamera *camera, const RwSphere *sphere);

/* free lists (inferred) */
void fn_8047FA28(RwBool preallocate); /* whether free lists may preallocate blocks */
RwFreeList *fn_8047FA30(RwUInt32 entrySize, RwUInt32 entriesPerBlock, RwUInt32 alignment,
                        RwUInt32 numBlocksToPreallocate, RwFreeList *freeList, RwUInt32 hint);
RwFreeList *fn_8047FBD8(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwUInt32 hint); /* create */
RwFreeList *fn_8047FC04(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwInt32 numBlocksToPreallocate, RwFreeList *freeList, RwUInt32 hint);
void fn_8047FC24(RwFreeList *freeList); /* frees the blocks, and the free list unless static */
RwBool fn_8047FCDC(RwFreeList *freeList);
void *fn_8047FDB0(RwFreeList *freeList, RwUInt32 hint);   /* allocates an entry */
RwFreeList *fn_8047FF50(RwFreeList *freeList, void *mem); /* frees an entry */

/* The engine's status (RwEngineStatus) */
#define rwENGINESTATUSIDLE 0    /* not initialized */
#define rwENGINESTATUSINITED 1  /* initialized; can be opened */
#define rwENGINESTATUSOPENED 2  /* opened on a device; can be started */
#define rwENGINESTATUSSTARTED 3 /* started; can render */

/* The parameters an engine is opened with */
typedef struct RwEngineOpenParams {
    void *displayID; /* the device's display (inferred) */
} RwEngineOpenParams;
CHECK_SIZEOF(RwEngineOpenParams, 0x4);

/* A video mode (0x18 bytes) */
typedef struct RwVideoMode {
    /* 0x00 */ RwInt32 width;
    /* 0x04 */ RwInt32 height;
    /* 0x08 */ RwInt32 depth;
    /* 0x0C */ RwInt32 flags; /* rwVIDEOMODE* */
    /* 0x10 */ RwInt32 refRate;
    /* 0x14 */ RwInt32 format;
} RwVideoMode;
CHECK_SIZEOF(RwVideoMode, 0x18);

/* The device the engine opens on (inferred) */
RwDevice *fn_804950A0(void);
/* registers the device's plugins (inferred) */
RwBool fn_80493E64(void);
/* sets the gamma correction (inferred) */
RwBool fn_80489830(RwReal gamma);
/* the pipeline library's plugin attach step (inferred): it has nothing to attach and returns TRUE,
 * which the engine ORs into its plugin offsets */
RwBool fn_8049B140(void);

/* the opening and closing callbacks of the core modules the engine registers (inferred from their
 * module IDs) */
void *fn_80482548(void *instance, RwInt32 offset, RwInt32 size); /* closes the vector module */
void *fn_804825EC(void *instance, RwInt32 offset, RwInt32 size); /* opens the vector module */
void *fn_80486888(void *instance, RwInt32 offset, RwInt32 size); /* opens the frame module */
void *fn_8048692C(void *instance, RwInt32 offset, RwInt32 size); /* closes the frame module */
void *fn_80480C6C(void *instance, RwInt32 offset, RwInt32 size); /* opens the stream module */
void *fn_80480CF8(void *instance, RwInt32 offset, RwInt32 size); /* closes the stream module */
void *fn_80485424(void *instance, RwInt32 offset, RwInt32 size); /* opens the camera module */
void *fn_804853C0(void *instance, RwInt32 offset, RwInt32 size); /* closes the camera module */
void *fn_80487914(void *instance, RwInt32 offset, RwInt32 size); /* opens the image module */
void *fn_80487BB0(void *instance, RwInt32 offset, RwInt32 size); /* closes the image module */
void *fn_8048A02C(void *instance, RwInt32 offset, RwInt32 size); /* opens the raster module */
void *fn_80489FC0(void *instance, RwInt32 offset, RwInt32 size); /* closes the raster module */
void *fn_8048D4DC(void *instance, RwInt32 offset, RwInt32 size); /* opens the texture module */
void *fn_8048D300(void *instance, RwInt32 offset, RwInt32 size); /* closes the texture module */
void *fn_8049B0CC(void *instance, RwInt32 offset, RwInt32 size); /* opens the pipeline module */
void *fn_8049B110(void *instance, RwInt32 offset, RwInt32 size); /* closes the pipeline module */
void *fn_80482BC4(void *instance, RwInt32 offset, RwInt32 size); /* opens the string module */
void *fn_80482C50(void *instance, RwInt32 offset, RwInt32 size); /* closes the string module */
void *fn_8049AFF8(void *instance, RwInt32 offset, RwInt32 size); /* opens the immediate module */
void *fn_8049AF94(void *instance, RwInt32 offset, RwInt32 size); /* closes the immediate module */
void *fn_8048053C(void *instance, RwInt32 offset, RwInt32 size); /* opens the resources module */
void *fn_80480690(void *instance, RwInt32 offset, RwInt32 size); /* closes the resources module */
RwBool fn_80482E20(void); /* sets the engine's string functions */
void fn_80482F60(void);   /* does nothing */

/* memory (inferred) */
RwBool fn_80480274(const RwMemoryFunctions *memFuncs); /* open; NULL uses the C library */
void fn_8048038C(void);                                /* close */
RwMemoryFunctions *fn_80480530(void);                  /* the engine's memory functions */

RwBool fn_80483444(const RwChar *path); /* whether a file path is absolute (inferred) */

/* frames (inferred) */
extern RwPluginRegistry lbl_8056DFE8; /* the frame plugin registry */
RwInt32 fn_804836EC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB);
RwInt32 fn_804878D4(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwFrame *fn_8048722C(RwFrame *frame);
RwFrame *fn_80486C5C(RwFrame *root);                   /* clones a hierarchy */
RwFrame *fn_80486CA4(RwFrame *root);                   /* forgets the clones made of a hierarchy */
RwBool fn_80486CF0(RwFrame *frame);                    /* whether the frame's hierarchy is dirty */
RwBool fn_80487208(RwFrame *frame);                    /* destroys a hierarchy */
RwFrame *fn_80486D00(void);                            /* create */
RwFrame *fn_804872D8(RwFrame *frame);                  /* the root */
RwFrame *fn_804872E0(RwFrame *parent, RwFrame *child); /* adds a child */
RwFrame *fn_80487348(RwFrame *parent, RwFrame *child); /* adds a child and marks it dirty */
RwBool fn_80486E2C(RwFrame *frame);                    /* destroy */
RwFrame *fn_80487448(RwFrame *oldChild);               /* removes a child from its parent */
RwFrame *fn_8048752C(RwFrame *frame, RwFrameCallBack callBack,
                     void *data); /* calls a function for each child, until it returns NULL */
RwFrame *fn_804875B0(RwFrame *frame, const RwV3d *translation, RwInt32 combine);  /* translate */
RwFrame *fn_8048764C(RwFrame *frame, const RwV3d *scale, RwInt32 combine);        /* scale */
RwFrame *fn_804876E8(RwFrame *frame, const RwMatrix *transform, RwInt32 combine); /* transform */
RwFrame *fn_80487784(RwFrame *frame, const RwV3d *axis, RwReal angle, RwInt32 combine); /* rotate */
RwFrame *fn_80487820(RwFrame *frame); /* sets the modelling matrix to the identity */
void fn_8048B5D8(RwFrame *root);      /* synchronizes a dirty hierarchy's LTMs */

/* A frame's private flags: what of its hierarchy (on the root) or its subtree needs
 * synchronizing */
#define rwFRAMEPRIVATEHIERARCHYSYNCLTM 0x01
#define rwFRAMEPRIVATEHIERARCHYSYNCOBJ 0x02
#define rwFRAMEPRIVATESUBTREESYNCLTM 0x04
#define rwFRAMEPRIVATESUBTREESYNCOBJ 0x08

/* The frames of a stream (8 bytes) */
typedef struct RwFrameList {
    /* 0x0 */ RwFrame **frames;
    /* 0x4 */ RwInt32 numFrames;
} RwFrameList;
CHECK_SIZEOF(RwFrameList, 0x8);

RwFrameList *fn_80483774(RwStream *stream, RwFrameList *frameList); /* stream read */
RwFrameList *fn_80483728(RwFrameList *frameList);                   /* deinitialise */

/* objects attached to frames (inferred) */
void fn_8048D7C0(void *object, RwFrame *frame); /* sets the frame */
void fn_8048D834(void *object);                 /* releases the frame */

/* textures (inferred) */
RwTexture *fn_80483D28(RwStream *stream); /* stream read */
RwBool fn_8048CD8C(RwTexture *texture);   /* destroy (releases a reference) */
RwTexture *fn_8048D130(const RwChar *name, const RwChar *maskName); /* read */
RwTexture *fn_8048CCDC(RwRaster *raster);                           /* create */
RwTexture *fn_8048CE54(RwTexture *texture, const RwChar *name);     /* sets the name */
RwTexture *fn_8048CF00(RwTexture *texture, const RwChar *maskName); /* sets the mask name */
RwTexture *fn_8048CA00(RwTexture *texture, RwRaster *raster);       /* sets the raster */
RwTexture *fn_8048CFFC(RwTexDictionary *dict, const RwChar *name);  /* finds a texture by name */
RwBool fn_8048CC54(RwTexDictionary *(*callBack)(RwTexDictionary *dict, void *data),
                   void *data); /* calls a function for each dictionary */
RwBool fn_8048D0EC(RwChar *name, RwChar *maskName, RwUInt8 mipLevel,
                   RwInt32 format); /* a mip level's names */

/* texture dictionaries (inferred) */
RwTexDictionary *fn_8048CA64(void);                  /* create */
RwBool fn_8048CB38(RwTexDictionary *dict);           /* destroy */
RwTexDictionary *fn_8048D0C4(RwTexDictionary *dict); /* sets the current one */
RwTexDictionary *fn_8048D0D8(void);                  /* the current one */
RwTexDictionary *fn_8048CBE0(RwTexDictionary *dict,
                             RwTexture *(*callBack)(RwTexture *texture, void *data),
                             void *data); /* calls a function for each texture */
RwTexture *fn_8048CFAC(RwTexDictionary *dict, RwTexture *texture); /* adds a texture */
extern RwPluginRegistry lbl_8056E030;                              /* the texture plugin registry */
extern RwPluginRegistry lbl_8056E048; /* the texture dictionary plugin registry */

/* texture mipmapping settings (inferred) */
RwBool fn_8048C9A8(RwBool enable); /* sets whether new textures are mipmapped */
RwBool fn_8048C9C0(void);          /* whether new textures are mipmapped */
RwBool fn_8048C9D4(RwBool enable); /* sets whether their mipmaps are generated */
RwBool fn_8048C9EC(void);          /* whether their mipmaps are generated */

/* strings in streams (inferred) */
RwUInt32 fn_80482F64(const RwChar *string);                        /* the chunk size */
const RwChar *fn_80482FA4(const RwChar *string, RwStream *stream); /* writes a chunk */
RwChar *fn_80483060(RwChar *string, RwStream *stream);             /* reads a chunk */

/* texture streams (inferred) */
RwInt32 fn_80483AFC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB); /* registers a stream plugin */
RwUInt32 fn_80483B38(const RwTexture *texture);                  /* the stream size */
const RwTexture *fn_80483BA0(const RwTexture *texture, RwStream *stream); /* stream write */
RwTexDictionary *fn_80483FBC(RwStream *stream); /* texture dictionary stream read */

RwBool fn_80485C3C(RwDevice *device, RwInt32 request, void *out, void *inOut,
                   RwInt32 numIn); /* a device system request (inferred) */

/* An image (inferred) */
typedef struct RwImage {
    /* 0x00 */ RwInt32 flags;
    /* 0x04 */ RwInt32 width;
    /* 0x08 */ RwInt32 height;
    /* 0x0C */ RwInt32 depth;
    /* 0x10 */ RwInt32 stride;
    /* 0x14 */ RwUInt8 *cpPixels;
    /* 0x18 */ RwRGBA *palette;
} RwImage;
CHECK_SIZEOF(RwImage, 0x1C);

/* images (inferred) */
RwImage *fn_80487D08(RwInt32 width, RwInt32 height, RwInt32 depth); /* create */
RwBool fn_80487DB0(RwImage *image);                                 /* destroy */
RwImage *fn_80487E18(RwImage *image);                               /* allocates the pixels */
RwImage *fn_804899A8(RwImage *image, RwRaster *raster);             /* reads a raster */
RwImage *fn_80487F50(RwImage *image);                               /* frees the pixels */
RwImage *fn_80487FA8(RwImage *image); /* sets each pixel's alpha to its brightest component */
RwImage *fn_80488088(RwImage *image, const RwImage *mask); /* applies a mask's alpha */
const RwChar *
fn_80488B58(const RwChar *name); /* the extension of the image file found for a name */
/* registers an extension's read and write functions (name inferred; stripped from retail) */
RwBool ImageRegisterFormat(const RwChar *extension, RwImage *(*imageRead)(const RwChar *imageName),
                           RwImage *(*imageWrite)(RwImage *image, const RwChar *imageName));
RwImage *fn_80488B94(const RwChar *imageName, const RwChar *maskName); /* reads a masked image */
RwRGBA *fn_80489118(RwRGBA *rgbOut, RwUInt32 pixelValue, RwInt32 rasterFormat); /* a pixel's RGBA */
RwImage *fn_804893B8(RwImage *destImage, const RwImage *sourceImage);           /* copy */
RwImage *fn_804894B8(RwImage *image); /* gamma corrects the palette or pixels */
RwImage *fn_8048A790(RwImage *destImage,
                     const RwImage *sourceImage); /* resamples a 32-bit image into another */
RwImage *fn_8048A96C(const RwImage *sourceImage, RwInt32 width,
                     RwInt32 height); /* a 32-bit copy resampled to a new size */
RwImage *fn_80489A90(RwImage *image, RwInt32 rasterType, RwInt32 *width, RwInt32 *height,
                     RwInt32 *depth, RwInt32 *format); /* the raster format that suits an image */

/* The color statistics of a palette quantizer leaf or box (0x1C bytes): the weight, the weighted
 * sums of red, green, blue and alpha (red and blue scaled for perceptual distance), the weighted
 * squared error, and the palette index (type and field names inferred) */
typedef struct rwPalQuantLeafNode {
    /* 0x00 */ RwReal weight;
    /* 0x04 */ RwRGBAReal sums;
    /* 0x14 */ RwReal error;
    /* 0x18 */ RwUInt8 palIndex;
} rwPalQuantLeafNode;
CHECK_SIZEOF(rwPalQuantLeafNode, 0x1C);

/* A palette quantizer tree node: its statistics and its 16 children (one per combination of a bit
 * of red, green, blue and alpha) (0x5C bytes; type and field names inferred) */
typedef struct rwPalQuantOctNode {
    /* 0x00 */ rwPalQuantLeafNode leaf;
    /* 0x1C */ struct rwPalQuantOctNode *children[16];
} rwPalQuantOctNode;
CHECK_SIZEOF(rwPalQuantOctNode, 0x5C);

/* A box of quantized colors: its lower and upper corners in tree cells (0x20 bytes; type and field
 * names inferred) */
typedef struct rwPalQuantBox {
    /* 0x00 */ RwInt32 lower[4];
    /* 0x10 */ RwInt32 upper[4];
} rwPalQuantBox;
CHECK_SIZEOF(rwPalQuantBox, 0x20);

/* A palette quantizer: the boxes the colors are split into, their errors and statistics, and the
 * tree the colors are added to (0x4008 bytes; field names inferred) */
typedef struct RwPalQuant {
    /* 0x0000 */ rwPalQuantBox boxes[256];
    /* 0x2000 */ RwReal boxErrors[256];
    /* 0x2400 */ rwPalQuantLeafNode boxStats[256];
    /* 0x4000 */ rwPalQuantOctNode *root;
    /* 0x4004 */ RwFreeList *nodeFreeList;
} RwPalQuant;
CHECK_SIZEOF(RwPalQuant, 0x4008);

/* palette quantization (inferred) */
RwBool fn_8048FED8(RwPalQuant *palQuant);                                      /* initialize */
void fn_8048DEB0(RwPalQuant *palQuant, RwImage *image, RwReal weight);         /* adds the colors */
RwInt32 fn_8048F5D0(RwRGBA *palette, RwInt32 maxColors, RwPalQuant *palQuant); /* the palette */
void fn_8048FAB0(RwUInt8 *pixels, RwInt32 stride, RwInt32 depth, RwBool packed,
                 RwPalQuant *palQuant, RwImage *image); /* matches an image to the palette */
void fn_804903F8(RwPalQuant *palQuant);                 /* terminate */

/* generates a raster's mip levels from an image, or from its first level (inferred) */
RwBool fn_8048D2C0(RwRaster *raster, RwImage *image); /* generates the mip levels */

/* rasters (inferred) */
#define rwRASTERTYPETEXTURE 0x4
RwRaster *fn_80489E74(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags); /* create */
RwBool fn_80489BDC(RwRaster *raster);                                               /* destroy */
RwRaster *fn_80489B2C(RwRaster *raster, RwInt16 *xOffset, RwInt16 *yOffset);        /* its offset */
RwRaster *fn_80489B40(RwRaster *raster);                                            /* unlock */
RwRaster *fn_80489B88(RwRaster *raster); /* unlocks the palette */
RwInt32 fn_80489C54(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB,
                    RwPluginObjectCopy copyCB);           /* registers a plugin */
RwUInt8 *fn_80489C94(RwRaster *raster, RwInt32 lockMode); /* locks the palette */
RwInt32 fn_80489CE4(RwRaster *raster);                    /* the number of mip levels */
RwRaster *fn_80489DC4(RwRaster *subRaster, RwRaster *raster, RwRect *rect); /* a sub raster */
RwUInt8 *fn_80489F70(RwRaster *raster, RwUInt8 level, RwInt32 lockMode);    /* lock */
RwRaster *fn_80489A1C(RwRaster *raster, RwImage *image); /* sets the pixels from an image */
const RwChar *fn_804885DC(const RwChar *path);           /* sets the path */

/* files (inferred) */
void *fn_8047A178(const RwChar *name, const RwChar *access);                         /* open */
RwInt32 fn_8047A388(void *fptr);                                                     /* close */
RwUInt32 fn_8047A3E0(void *buffer, RwUInt32 size, RwUInt32 count, void *fptr);       /* read */
RwUInt32 fn_8047A428(const void *buffer, RwUInt32 size, RwUInt32 count, void *fptr); /* write */
RwInt32 fn_8047A470(void *fptr, RwInt32 offset, RwInt32 origin);                     /* seek */
RwChar *fn_8047A518(RwChar *buffer, RwInt32 maxLen, void *fptr); /* reads a line */
RwBool fn_8047A67C(const RwChar *name);                          /* whether it exists */
RwBool fn_8047A650(void *fptr);                                  /* whether at the end */
RwInt32 fn_8047A7C0(void *fptr);                                 /* the position */

/* lists of fixed-size entries (inferred) */
/* A list of fixed-size entries in one growable array (0x10 bytes) */
typedef struct RwSList {
    /* 0x0 */ RwUInt8 *listElements;
    /* 0x4 */ RwInt32 numElementsFilled;
    /* 0x8 */ RwInt32 numElementsAlloced;
    /* 0xC */ RwInt32 entrySize;
} RwSList;
CHECK_SIZEOF(RwSList, 0x10);
RwSList *fn_8047DC90(RwInt32 size, RwUInt32 hint);              /* create */
void *fn_8047DEA0(RwSList *sList, RwInt32 size, RwUInt32 hint); /* new entries */
void *fn_8047DFAC(RwSList *sList);                              /* the entries */
RwBool fn_8047DFC8(RwSList *sList);                             /* destroy */
void *fn_8047DDBC(RwSList *sList, RwUInt32 hint);               /* a new entry */
RwInt32 fn_8047E03C(RwSList *sList);                            /* the number of entries */
void *fn_8047E0CC(RwSList *sList, RwInt32 index);               /* an entry */
RwSList *fn_8047DDB0(RwSList *sList);                           /* empties */
void *fn_8047E044(RwSList *sList);                   /* destroys the list, keeping its entries */
void *fn_8047E0E0(RwSList *sList);                   /* the first entry */
void *fn_8047E0E8(RwSList *sList);                   /* the end of the entries */
RwSList *fn_8047DF9C(RwSList *sList, RwInt32 count); /* drops entries from the end */

/* An immediate mode 3D vertex (0x24 bytes) */
typedef struct RwIm3DVertex {
    /* 0x00 */ RwV3d objVertex;
    /* 0x0C */ RwV3d objNormal;
    /* 0x18 */ RwRGBA color;
    /* 0x1C */ RwReal u;
    /* 0x20 */ RwReal v;
} RwIm3DVertex;
CHECK_SIZEOF(RwIm3DVertex, 0x24);

typedef RwUInt16 RwImVertexIndex;

/* An immediate mode 2D vertex (0x18 bytes) */
struct RwIm2DVertex {
    /* 0x00 */ RwReal x;
    /* 0x04 */ RwReal y;
    /* 0x08 */ RwReal z;
    /* 0x0C */ RwRGBA emissiveColor;
    /* 0x10 */ RwReal u;
    /* 0x14 */ RwReal v;
};
CHECK_SIZEOF(struct RwIm2DVertex, 0x18);

/* GameCube device driver state (inferred) */
struct _GXRenderModeObj;
extern struct _GXRenderModeObj *lbl_805F164C; /* the render mode of the frame buffer */
extern RwInt32 lbl_805F15F0;    /* non-zero while subraster viewports are at double height */
extern RwInt32 lbl_805F1648;    /* a frame buffer line that scissor boxes are split at */
extern RwInt32 lbl_805EDF28;    /* selects which side of that line scissor boxes cover */
extern RwTexture *lbl_805F1664; /* 2D vertices are textured while its raster is set */
extern RwRaster *lbl_805F1660;  /* a white 4x4 raster, set on lbl_805F1664 for a NULL texture */
extern RwInt32 lbl_805F15E0;    /* whether frame buffer copies clear the frame buffer */
extern RwInt32 lbl_805F15E4;    /* the frame buffer's GX pixel format */
extern RwInt32 lbl_805F15E8;    /* the GX pixel format currently set */
extern RwRaster *lbl_805F1668;  /* the raster context, which rasters are rendered into */
/* The device's frame queue and external frame buffers (see the device unit) */
extern volatile RwInt32 lbl_805F161C;
extern volatile RwInt32 lbl_805F1610;
extern volatile RwInt32 lbl_805F1614;
extern void *volatile lbl_805F1600;
extern void *volatile lbl_805F1604;
extern RwInt32 lbl_805F1670; /* the texture plugin's offset (inferred) */
void fn_80497804(void);      /* sets up the texture of textured 2D vertices */

/* GameCube 2D immediate mode (inferred) */
RwBool fn_80490CBC(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1,
                   RwInt32 vert2); /* draws a line */
RwBool fn_80490AB8(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2,
                   RwInt32 vert3); /* draws a triangle */
RwBool fn_80490E30(RwInt32 primType, RwIm2DVertex *vertices,
                   RwInt32 numVertices); /* draws a primitive */
RwBool fn_8049140C(RwInt32 primType, RwIm2DVertex *vertices, RwInt32 numVertices,
                   RwImVertexIndex *indices, RwInt32 numIndices); /* draws an indexed primitive */

/* GameCube device standard functions (inferred roles from the standards table) */
RwBool fn_804946A4(void *out, void *inOut, RwInt32 numIn); /* camera begin update */
RwBool fn_80494C5C(void *out, void *inOut, RwInt32 numIn); /* camera end update */
RwBool fn_804945E8(void *out, void *inOut, RwInt32 numIn); /* camera clear */
RwBool fn_80494C6C(void *out, void *inOut, RwInt32 numIn); /* raster show */
RwBool fn_80491BAC(void *out, void *inOut, RwInt32 numIn); /* RGB to pixel */
RwBool fn_80491EC0(void *out, void *inOut, RwInt32 numIn); /* pixel to RGB */
RwBool fn_804929F0(void *out, void *inOut, RwInt32 numIn); /* image from a raster */
RwBool fn_804935C4(void *out, void *inOut, RwInt32 numIn); /* raster from an image */
RwBool fn_80493888(void *out, void *inOut, RwInt32 numIn); /* raster format for an image */
RwBool fn_80497008(void *out, void *inOut, RwInt32 numIn); /* raster destroy */
RwBool fn_80496C58(void *out, void *inOut, RwInt32 numIn); /* raster create */
RwBool fn_8049714C(void *out, void *inOut, RwInt32 numIn); /* texture set raster */
RwBool fn_80495C54(void *out, void *inOut, RwInt32 numIn); /* raster lock */
RwBool fn_804960B4(void *out, void *inOut, RwInt32 numIn); /* raster unlock */
RwBool fn_80496268(void *out, void *inOut, RwInt32 numIn); /* raster palette lock */
RwBool fn_80496338(void *out, void *inOut, RwInt32 numIn); /* raster palette unlock */
RwBool fn_804994DC(void *out, void *inOut, RwInt32 numIn); /* raster clear */
RwBool fn_804994B8(void *out, void *inOut, RwInt32 numIn); /* raster clear rectangle */
RwBool fn_80498A4C(void *out, void *inOut, RwInt32 numIn); /* raster render */
RwBool fn_80498A9C(void *out, void *inOut, RwInt32 numIn); /* raster render scaled */
RwBool fn_80498A74(void *out, void *inOut, RwInt32 numIn); /* raster render fast */
RwBool fn_80499520(void *out, void *inOut, RwInt32 numIn); /* set raster context */
RwBool fn_80497168(void *out, void *inOut, RwInt32 numIn); /* sub raster */
RwBool fn_8049952C(void *out, void *inOut, RwInt32 numIn); /* native texture size */
RwBool fn_80495688(void *out, void *inOut, RwInt32 numIn); /* raster mip levels */

/* native texture write and read, whose out argument is the stream */
RwBool fn_804995B4(void *stream, void *inOut, RwInt32 numIn);
RwBool fn_8049988C(void *stream, void *inOut, RwInt32 numIn);

/* GameCube device driver functions in later units (inferred) */
RwBool fn_80497904(RwInt32 state, void *value); /* sets a render state */
RwBool fn_80497474(RwInt32 state, void *value); /* gets a render state */
RwInt32 fn_804953F0(void);          /* registers the raster plugin; returns its offset */
RwInt32 fn_80499BF8(void);          /* registers the texture plugin; returns its offset */
RwBool fn_8049A78C(RwUInt16 token); /* whether the GPU has passed a draw sync token */
void fn_80498E08(RwRaster *raster, RwRect *rect, RwRGBA *color,
                 RwInt32 clearMode); /* clears a raster rectangle */
void fn_80497198(void);              /* finishes starting the device */
void fn_8049742C(void);              /* starts stopping the device */

/* immediate mode 3D vertex flags (RwIm3DTransformFlags) */
#define rwIM3D_VERTEXUV 0x1
#define rwIM3D_NOCLIP 0x4
#define rwIM3D_VERTEXXYZ 0x8
#define rwIM3D_VERTEXRGBA 0x10

/* The transformed vertices and primitive an Im3D render pipeline works on */
typedef struct rwIm3DPoolStash {
    /* 0x00 */ RwUInt32 flags; /* rwIM3D_* */
    /* 0x04 */ RwMatrix *ltm;  /* the vertices' frame, or NULL */
    /* 0x08 */ RwUInt32 numVerts;
    /* 0x0C */ RwIm3DVertex *objVerts;
    /* 0x10 */ void *camVerts;    /* not used by the GameCube pipelines */
    /* 0x14 */ void *devVerts;    /* not used by the GameCube pipelines */
    /* 0x18 */ void *meshState;   /* not used by the GameCube pipelines */
    /* 0x1C */ void *renderState; /* not used by the GameCube pipelines */
    /* 0x20 */ RxPipeline *pipeline;
    /* 0x24 */ RwInt32 primType; /* rwPRIMTYPE* */
    /* 0x28 */ RwImVertexIndex *indices;
    /* 0x2C */ RwUInt32 numIndices;
} rwIm3DPoolStash;
CHECK_SIZEOF(rwIm3DPoolStash, 0x30);

/* The vertices of the current immediate mode 3D transform */
typedef struct rwIm3DPool {
    /* 0x00 */ RwUInt16 numElements;
    /* 0x04 */ RwIm3DVertex *elements; /* NULL when no transform is active */
    /* 0x08 */ RwInt32 stride;
    /* 0x0C */ rwIm3DPoolStash stash;
} rwIm3DPool;
CHECK_SIZEOF(rwIm3DPool, 0x3C);

/* An Im3D render pipeline per primitive type */
typedef struct rwIm3DRenderPipelines {
    /* 0x00 */ RxPipeline *triList;
    /* 0x04 */ RxPipeline *triFan;
    /* 0x08 */ RxPipeline *triStrip;
    /* 0x0C */ RxPipeline *lineList;
    /* 0x10 */ RxPipeline *polyLine;
    /* 0x14 */ RxPipeline *pointList;
} rwIm3DRenderPipelines;
CHECK_SIZEOF(rwIm3DRenderPipelines, 0x18);

/* The immediate module's globals in the engine: the current transform and render pipelines,
 * the platform's, and the current transform */
typedef struct rwImmediGlobals {
    /* 0x00 */ RxPipeline *im3DTransformPipeline;
    /* 0x04 */ rwIm3DRenderPipelines im3DRenderPipelines;
    /* 0x1C */ RxPipeline *platformIm3DTransformPipeline;
    /* 0x20 */ rwIm3DRenderPipelines platformIm3DRenderPipelines;
    /* 0x38 */ rwIm3DPool curPool;
} rwImmediGlobals;
CHECK_SIZEOF(rwImmediGlobals, 0x74);

extern RwModuleInfo lbl_805F1684; /* the immediate module */

#define RWIMMEDIGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwImmediGlobals, lbl_805F15B4, lbl_805F1684.globalsOffset)->var)

/* immediate mode 3D (inferred) */
/* transforms vertices for the render functions below; NULL on failure */
void *fn_8049A82C(RwIm3DVertex *vertices, RwUInt32 numVertices, RwMatrix *ltm, RwUInt32 flags);
RwBool fn_8049A90C(void); /* ends the transform */
RwBool fn_8049A95C(RwInt32 primType, RwImVertexIndex *indices, RwInt32 numIndices);
RwBool fn_8049AAF8(RwInt32 primType); /* renders the transformed vertices in order */
/* set the transform pipeline and a primitive type's render pipeline; NULL restores the
 * platform's. They return the pipeline set, or NULL */
RxPipeline *fn_8049AC90(RxPipeline *pipeline);
RxPipeline *fn_8049ACE0(RxPipeline *pipeline, RwInt32 primType);

/* the GameCube Im3D pipelines (inferred): create and destroy the transform pipeline and the
 * render pipelines; the creators return FALSE on failure */
RwBool fn_8049F8AC(RxPipeline **transformPipeline);
void fn_8049F948(RxPipeline **transformPipeline);
void fn_8049F990(rwIm3DRenderPipelines *renderPipelines);
RwBool fn_8049FA1C(rwIm3DRenderPipelines *renderPipelines);

/* the node definitions of the GameCube Im3D pipelines (inferred) */
RxNodeDefinition *fn_8049FB0C(void); /* the transform node */
RxNodeDefinition *fn_804A0760(void); /* the render node */

RwReal fn_804823F0(const RwV2d *v); /* the length of a vector */

/* cameras (inferred) */
void fn_804842C8(RwCamera *camera);      /* perspective frustum */
RwCamera *fn_80484AAC(RwCamera *camera); /* perspective view matrix */
void fn_80484C10(RwCamera *camera);      /* parallel frustum */
RwCamera *fn_80485124(RwCamera *camera); /* parallel view matrix */
void *fn_8048528C(void *object);         /* frame sync */
RwCamera *fn_804852F0(RwCamera *camera); /* default end update */
RwCamera *fn_80485354(RwCamera *camera); /* default begin update */
RwCamera *fn_804854B4(RwCamera *camera); /* end update */
RwCamera *fn_804854DC(RwCamera *camera); /* begin update */
RwCamera *fn_804857BC(RwCamera *camera, RwRGBA *color, RwInt32 clearMode); /* clear */
RwCamera *fn_80485808(RwCamera *camera, void *device, RwUInt32 flags);     /* show raster */
RwBool fn_8048B354(void);           /* synchronizes dirty frames; returns TRUE (inferred) */
void fn_8049B148(RwCamera *camera); /* the pipeline library's camera hook; does nothing */
RwRaster *fn_80489D44(RwRaster *raster, void *device, RwUInt32 flags); /* shows a raster */
extern RwPluginRegistry lbl_8056DF70; /* the camera plugin registry */

/* cameras (inferred) */
RwCamera *fn_80485A58(void);                                      /* create */
RwCamera *fn_80485950(RwCamera *camera, const RwV2d *viewWindow); /* set the view window */
RwCamera *fn_80485504(RwCamera *camera, const RwV2d *offset);     /* set the view offset */
RwCamera *fn_80485550(RwCamera *camera, RwReal nearPlane);        /* set the near plane */
RwCamera *fn_8048560C(RwCamera *camera, RwReal farPlane);         /* set the far plane */
RwCamera *fn_8048584C(RwCamera *camera, RwInt32 projection);      /* set the projection */
RwInt32 fn_80483554(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB); /* registers a stream plugin */
RwCamera *fn_80483590(RwStream *stream);                         /* stream read */
RwBool fn_804859F8(RwCamera *camera);                            /* destroy */

/* transforms a point by a matrix and returns out (inferred) */
RwV3d *fn_80482458(RwV3d *out, const RwV3d *point, const RwMatrix *matrix);
RwV3d *fn_804824A4(RwV3d *pointsOut, const RwV3d *pointsIn, RwInt32 numPoints,
                   const RwMatrix *matrix);
RwV3d *fn_80482504(RwV3d *out, const RwV3d *vector, const RwMatrix *matrix);

/* The current camera's view transform (inferred): the inverse of the camera frame's LTM combined
 * with a transform from the camera's view offset, set when a camera begins its update
 * (fn_804946A4) */
extern RwMatrix lbl_805E31CC;

/* calls a function for each entry in use in a free list (inferred) */
RwFreeList *fn_804800DC(RwFreeList *freeList, void (*callBack)(void *mem, void *data), void *data);

RwFrame *fn_8048752C(RwFrame *frame, RwFrameCallBack callBack, void *data);

/* plugin registration with cameras and textures (inferred: the user data plugin registers with
 * them in RenderWare's attach order, between frames and lights and after materials) */
RwInt32 fn_804859B8(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32 fn_80483554(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB);
RwInt32 fn_8048D280(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwInt32 fn_80483AFC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB);

/* errors */
void *fn_8047DB10(void *instance, RwInt32 offset, RwInt32 size); /* opens a counted module */
void *fn_8047DB24(void *instance, RwInt32 offset, RwInt32 size); /* closes it */
void *fn_8047DB38(void *instance, RwInt32 offset, RwInt32 size); /* opens the error module */
void *fn_8047DB70(void *instance, RwInt32 offset, RwInt32 size); /* closes it */
RwError *fn_8047DB84(const RwError *error);
RwError *fn_8047DBE4(RwError *error); /* takes the pending error */
RwInt32 fn_8047DC28(RwInt32 code, ...);

/* the engine and its plugins */
RwInt32 fn_80485D6C(void);
RwInt32 fn_80485D78(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor);
RwInt32 fn_80485DB8(RwUInt32 pluginID); /* the engine plugin offset of a plugin, or -1 */
RwVideoMode *fn_80485DE4(RwVideoMode *modeinfo, RwInt32 modeIndex); /* a video mode's info */
RwInt32 fn_80485E70(void);                                          /* the current video mode */
RwBool fn_80485EF4(void);                                           /* stop */
RwBool fn_80485FE0(void);                                           /* start */
RwBool fn_80486138(void);                                           /* close */
RwBool fn_80486204(RwEngineOpenParams *initParams);                 /* open */
RwBool fn_804864AC(void);                                           /* terminate */
RwBool fn_804864F8(const RwMemoryFunctions *memFuncs, RwUInt32 initFlags,
                   RwUInt32 resArenaSize); /* initialize */

#ifdef __cplusplus
}
#endif

#endif
