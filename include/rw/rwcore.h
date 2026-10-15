#ifndef RW_RWCORE_H
#define RW_RWCORE_H

#include <dolphin/types.h>
#include <stdarg.h>

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
typedef char RwChar;

#define RW_STREAM_VERSION 0x37002
#define RW_STREAM_BUILD 0x37

/* chunk IDs */
#define rwID_STRUCT 0x1
#define rwID_MATLIST 0x8
#define rwID_ATOMICSECT 0x9
#define rwID_PLANESECT 0xA
#define rwID_WORLD 0xB

/* errors */
#define E_RW_BADVERSION 0x80000004
#define E_RW_READ 0x8000001A    /* reading a stream failed */
#define E_RW_NOMEM 0x80000013   /* reported with the size that could not be allocated */
#define E_RW_NOERROR 0x80000000 /* no error is pending */
#define E_RW_NULLP 0x80000016   /* a required argument is NULL */

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
#define rwRENDERSTATEZTESTENABLE 6
#define rwRENDERSTATEZWRITEENABLE 8
#define rwRENDERSTATETEXTUREFILTER 9
#define rwRENDERSTATESRCBLEND 10
#define rwRENDERSTATEDESTBLEND 11
#define rwRENDERSTATEVERTEXALPHAENABLE 12
#define rwRENDERSTATECULLMODE 20

/* render state values */
#define rwFILTERNEAREST 1
#define rwBLENDSRCALPHA 5
#define rwBLENDINVSRCALPHA 6
#define rwCULLMODECULLNONE 1

typedef struct RwIm2DVertex RwIm2DVertex;

/* The device the engine renders with (0x38 bytes); only its system function, near screen depth,
 * render state functions and 2D indexed primitive function are mapped */
typedef struct RwDevice {
    /* 0x00 */ RwReal gammaCorrection;
    /* 0x04 */ RwBool (*fpSystem)(RwInt32 request, void *out, void *inOut, RwInt32 numIn);
    /* 0x08 */ RwReal zBufferNear; /* the screen depth of 2D immediate mode vertices */
    /* 0x0C */ RwReal zBufferFar;
    /* 0x10 */ RwBool (*fpRenderStateSet)(RwInt32 state, void *value);
    /* 0x14 */ RwBool (*fpRenderStateGet)(RwInt32 state, void *value);
    /* 0x18 */ RwUInt8 field_0x18[0xC];
    /* 0x24 */ RwBool (*fpIm2DRenderIndexedPrimitive)(RwInt32 primType, RwIm2DVertex *vertices,
                                                      RwInt32 numVertices, RwUInt16 *indices,
                                                      RwInt32 numIndices);
    /* 0x28 */ RwUInt8 field_0x28[0x10];
} RwDevice;

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

/* The memory functions the engine allocates with */
typedef struct RwMemoryFunctions {
    /* 0x0 */ void *(*rwmalloc)(RwUInt32 size, RwUInt32 hint);
    /* 0x4 */ void (*rwfree)(void *mem);
    /* 0x8 */ void *(*rwrealloc)(void *mem, RwUInt32 newSize, RwUInt32 hint);
    /* 0xC */ void *(*rwcalloc)(RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint);
} RwMemoryFunctions;

/* A link of a doubly linked list */
typedef struct RwLLLink {
    struct RwLLLink *next;
    struct RwLLLink *prev;
} RwLLLink;

/* A doubly linked list: its link is the head and the tail sentinel */
typedef struct RwLinkList {
    RwLLLink link;
} RwLinkList;

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
    /* 0x048 */ RwBool (*stdFunc[0x1D])(void *out, void *inOut, RwInt32 numIn); /* RwStandard */
    /* 0x0BC */ RwLinkList dirtyFrameList; /* the roots of hierarchies that need synchronizing */
    /* 0x0C4 */ RwStringFunctions stringFuncs;
    /* 0x108 */ RwMemoryFunctions memoryFuncs;
    /* 0x118 */ void *(*rwfreelistalloc)(RwFreeList *freeList, RwUInt32 hint);
    /* 0x11C */ RwFreeList *(*rwfreelistfree)(RwFreeList *freeList, void *mem);
    /* 0x120 */ RwUInt8 field_0x120[0x4];
    /* 0x124 */ RwInt32 engineStatus;      /* rwENGINESTATUS* */
    /* 0x128 */ RwUInt32 resArenaInitSize; /* the bytes of the resources heap (inferred) */
} RwGlobals;

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
#define rwstrcat(dst, src) (lbl_805F15B4->stringFuncs.vecStrcat((dst), (src)))
#define rwstrcmp(string1, string2) (lbl_805F15B4->stringFuncs.vecStrcmp((string1), (string2)))
#define rwstricmp(string1, string2) (lbl_805F15B4->stringFuncs.vecStricmp((string1), (string2)))
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

typedef struct RwV3d {
    RwReal x;
    RwReal y;
    RwReal z;
} RwV3d;

typedef struct RwBBox {
    RwV3d sup;
    RwV3d inf;
} RwBBox;

RwBBox *fn_8048349C(RwBBox *boundBox, const RwV3d *verts, RwInt32 numVerts); /* calculates */

typedef struct RwRGBA {
    RwUInt8 red;
    RwUInt8 green;
    RwUInt8 blue;
    RwUInt8 alpha;
} RwRGBA;

typedef struct RwTexCoords {
    RwReal u;
    RwReal v;
} RwTexCoords;

typedef struct RwV2d {
    RwReal x;
    RwReal y;
} RwV2d;

typedef struct RwRGBAReal {
    RwReal red;
    RwReal green;
    RwReal blue;
    RwReal alpha;
} RwRGBAReal;

/* converts a color with components 0 to 1 to bytes, rounding */
#define RwRGBAFromRwRGBAReal(o, i)                                                                 \
    do {                                                                                           \
        (o)->red = (RwUInt8)(RwInt32)((i)->red * 255.0f + 0.5f);                                   \
        (o)->green = (RwUInt8)(RwInt32)((i)->green * 255.0f + 0.5f);                               \
        (o)->blue = (RwUInt8)(RwInt32)((i)->blue * 255.0f + 0.5f);                                 \
        (o)->alpha = (RwUInt8)(RwInt32)((i)->alpha * 255.0f + 0.5f);                               \
    } while (0)

/* The header of every engine object: its type and the object it is attached to */
typedef struct RwObject {
    /* 0x0 */ RwUInt8 type;
    /* 0x1 */ RwUInt8 subType;
    /* 0x2 */ RwUInt8 flags;
    /* 0x3 */ RwUInt8 privateFlags;
    /* 0x4 */ void *parent;
} RwObject;

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

/* A camera; its parent object is its frame */

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
} RwRaster;

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

/* A plane: points p with dot(normal, p) == distance */
typedef struct RwPlane {
    /* 0x0 */ RwV3d normal;
    /* 0xC */ RwReal distance;
} RwPlane;

/* A plane of a camera's view frustum, with the corner of a box nearest to it (0x14 bytes) */
typedef struct RwFrustumPlane {
    /* 0x00 */ RwPlane plane;
    /* 0x10 */ RwUInt8 closestX; /* each 0 or 1: which of a box's two corners */
    /* 0x11 */ RwUInt8 closestY;
    /* 0x12 */ RwUInt8 closestZ;
    /* 0x13 */ RwUInt8 pad;
} RwFrustumPlane;

typedef void *(*RwObjectHasFrameSyncFunction)(void *object);
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

/* A rendering pipeline; only the plugin that owns it is mapped */
typedef struct RxPipeline {
    /* 0x00 */ RwUInt8 field_0x0[0x2C];
    /* 0x2C */ RwUInt32 pluginId;   /* 0 for none */
    /* 0x30 */ RwUInt32 pluginData; /* the owner plugin's data */
} RxPipeline;

/* A node of a pipeline; only its private data is mapped */
typedef struct RxPipelineNode {
    /* 0x00 */ RwUInt8 field_0x0[0x14];
    /* 0x14 */ void *privateData;
} RxPipelineNode;

/* The parameters a node body receives; only the object being rendered is mapped */
typedef struct RxPipelineNodeParam {
    /* 0x0 */ void *dataParam;
} RxPipelineNodeParam;

typedef RwBool (*RxNodeBodyFn)(RxPipelineNode *self, const RxPipelineNodeParam *params);
typedef RwBool (*RxPipelineNodeInitFn)(RxPipelineNode *self);

/* The definition of a pipeline node (0x44 bytes); only the name, the node body, the pipeline node
 * initialisation and the private data size are mapped */
typedef struct RxNodeDefinition {
    /* 0x00 */ RwChar *name;
    /* 0x04 */ RxNodeBodyFn nodeBody;
    /* 0x08 */ void *nodeInit;
    /* 0x0C */ void *nodeTerm;
    /* 0x10 */ RxPipelineNodeInitFn pipelineNodeInit;
    /* 0x14 */ void *pipelineNodeTerm;
    /* 0x18 */ void *pipelineNodeConfig;
    /* 0x1C */ void *configMsgHandler;
    /* 0x20 */ RwUInt8 io[0x14];
    /* 0x34 */ RwUInt32 pipelineNodePrivateDataSize;
    /* 0x38 */ RwUInt8 field_0x38[0xC];
} RxNodeDefinition;

/* building pipelines (inferred from their use) */
RxPipeline *fn_8049B76C(void);                 /* creates a pipeline */
RxPipeline *fn_8049CCB8(RxPipeline *pipeline); /* locks a pipeline for editing */
RxPipeline *fn_8049C1E0(RxPipeline *pipeline); /* unlocks it */
/* adds a fragment of the NULL-terminated node definitions to a locked pipeline */
RxPipeline *fn_8049D0D0(RxPipeline *pipeline, RwUInt32 *firstIndex, RxNodeDefinition *nodeDef, ...);
/* finds a pipeline node by name */
RxPipelineNode *fn_8049CFCC(RxPipeline *pipeline, const RwChar *name, RxPipelineNode *start,
                            RwInt32 *nodeIndex);

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

/* A texture: its raster and reference count (0x58 bytes) */
/* The characters of a texture name, with its terminator */
#define rwTEXTUREBASENAMELENGTH 32

typedef struct RwTexture {
    /* 0x00 */ RwRaster *raster;
    /* 0x04 */ RwUInt8 field_0x4[0xC];
    /* 0x10 */ RwChar name[rwTEXTUREBASENAMELENGTH];
    /* 0x30 */ RwChar mask[rwTEXTUREBASENAMELENGTH];
    /* 0x50 */ RwUInt32 filterAddressing; /* the filter mode in the low byte */
    /* 0x54 */ RwInt32 refCount;
} RwTexture;

typedef struct RwLine {
    RwV3d start;
    RwV3d end;
} RwLine;

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

/* The squared errors a matrix may have and still count as normal, orthogonal or the identity */
typedef struct RwMatrixTolerance {
    /* 0x0 */ RwReal Normal;
    /* 0x4 */ RwReal Orthogonal;
    /* 0x8 */ RwReal Identity;
} RwMatrixTolerance;

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

/* A stream chunk's header, decoded (0x14 bytes) */
typedef struct RwChunkHeaderInfo {
    /* 0x00 */ RwUInt32 type;
    /* 0x04 */ RwUInt32 length;
    /* 0x08 */ RwUInt32 version;
    /* 0x0C */ RwUInt32 buildNum;
    /* 0x10 */ RwBool isComplex; /* whether the chunk holds other chunks */
} RwChunkHeaderInfo;

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

/* A memory stream's state */
typedef struct RwStreamMemory {
    /* 0x0 */ RwUInt32 position;
    /* 0x4 */ RwUInt32 nSize; /* the bytes of memBlock */
    /* 0x8 */ RwUInt8 *memBlock;
} RwStreamMemory;

/* A custom stream's functions and their data (0x14 bytes) */
typedef struct RwStreamCustom {
    /* 0x00 */ RwBool (*sfnclose)(void *data);
    /* 0x04 */ RwUInt32 (*sfnread)(void *data, void *buffer, RwUInt32 length);
    /* 0x08 */ RwBool (*sfnwrite)(void *data, const void *buffer, RwUInt32 length);
    /* 0x0C */ RwBool (*sfnskip)(void *data, RwUInt32 offset);
    /* 0x10 */ void *data;
} RwStreamCustom;

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

typedef struct RwVideoMode RwVideoMode;

/* The device the engine opens on (inferred) */
RwDevice *fn_804950A0(void);
/* registers the device's plugins (inferred) */
RwBool fn_80493E64(void);
/* sets the gamma correction (inferred) */
RwBool fn_80489830(RwReal gamma);
/* registers more core plugins; negative on failure, like a plugin offset (inferred) */
RwInt32 fn_8049B140(void);

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

RwFrameList *fn_80483774(RwStream *stream, RwFrameList *frameList); /* stream read */
RwFrameList *fn_80483728(RwFrameList *frameList);                   /* deinitialise */

/* objects attached to frames (inferred) */
void fn_8048D7C0(void *object, RwFrame *frame); /* sets the frame */
void fn_8048D834(void *object);                 /* releases the frame */

/* textures (inferred) */
RwTexture *fn_80483D28(RwStream *stream); /* stream read */
RwBool fn_8048CD8C(RwTexture *texture);   /* destroy (releases a reference) */
RwTexture *fn_8048D130(const RwChar *name, const RwChar *maskName); /* read */

/* texture dictionaries (inferred) */
typedef struct RwTexDictionary RwTexDictionary;
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

/* images (inferred) */
RwImage *fn_80487D08(RwInt32 width, RwInt32 height, RwInt32 depth); /* create */
RwBool fn_80487DB0(RwImage *image);                                 /* destroy */
RwImage *fn_80487E18(RwImage *image);                               /* allocates the pixels */
RwImage *fn_804899A8(RwImage *image, RwRaster *raster);             /* reads a raster */

/* rasters (inferred) */
#define rwRASTERTYPETEXTURE 0x4
RwRaster *fn_80489E74(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags); /* create */
RwBool fn_80489BDC(RwRaster *raster);                                               /* destroy */
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

#define rwIM3D_VERTEXUV 0x1
#define rwIM3D_NOCLIP 0x4

/* immediate mode 3D (inferred) */
void *fn_8049A82C(RwIm3DVertex *vertices, RwInt32 numVertices, RwMatrix *ltm, RwUInt32 flags);
RwBool fn_8049A90C(void); /* end */
RwBool fn_8049A95C(RwInt32 primType, RwImVertexIndex *indices, RwInt32 numIndices);

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
void fn_8049B148(RwCamera *camera); /* empty */
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

/* executes a pipeline on an object (inferred) */
RxPipeline *fn_8049B674(RxPipeline *pipeline, void *data, RwBool heapReset);
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
