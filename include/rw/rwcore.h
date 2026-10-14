#ifndef RW_RWCORE_H
#define RW_RWCORE_H

#include <dolphin/types.h>

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
#define E_RW_NOMEM 0x80000013 /* reported with the size that could not be allocated */

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
#define rwRENDERSTATESRCBLEND 10
#define rwRENDERSTATEDESTBLEND 11
#define rwRENDERSTATEVERTEXALPHAENABLE 12

/* The engine globals; only the current camera and world, the frame counters, the render state,
 * string copy and length, memory and free list functions are mapped. */
typedef struct RwGlobals {
    /* 0x000 */ RwCamera *curCamera;
    /* 0x004 */ void *curWorld;
    /* 0x008 */ RwUInt16 renderFrame;
    /* 0x00A */ RwUInt16 lightFrame; /* lights are created one frame behind it */
    /* 0x00C */ RwUInt8 field_0xC[0x14];
    /* 0x020 */ RwBool (*renderStateSet)(RwInt32 state, void *value);
    /* 0x024 */ RwBool (*renderStateGet)(RwInt32 state, void *value);
    /* 0x028 */ RwBool (*stdFunc[0x29])(void *out, void *inOut, RwInt32 numIn); /* RwStandard */
    /* 0x0CC */ RwChar *(*rwstrcpy)(RwChar *dst, const RwChar *src);
    /* 0x0D0 */ RwUInt8 field_0xD0[0x24];
    /* 0x0F4 */ RwUInt32 (*rwstrlen)(const RwChar *string);
    /* 0x0F8 */ RwUInt8 field_0xF8[0x10];
    /* 0x108 */ void *(*rwmalloc)(RwUInt32 size, RwUInt32 hint);
    /* 0x10C */ void (*rwfree)(void *mem);
    /* 0x110 */ void *(*rwrealloc)(void *mem, RwUInt32 newSize, RwUInt32 hint);
    /* 0x114 */ RwUInt8 field_0x114[0x4];
    /* 0x118 */ void *(*rwfreelistalloc)(RwFreeList *freeList, RwUInt32 hint);
    /* 0x11C */ void (*rwfreelistfree)(RwFreeList *freeList, void *mem);
} RwGlobals;

extern RwGlobals *lbl_805F15B4; /* the engine instance */

#define RwMalloc(size, hint) (lbl_805F15B4->rwmalloc((size), (hint)))
#define RwFree(mem) (lbl_805F15B4->rwfree(mem))
#define RwRealloc(mem, newSize, hint) (lbl_805F15B4->rwrealloc((mem), (newSize), (hint)))
#define RwFreeListAlloc(freeList, hint) (lbl_805F15B4->rwfreelistalloc((freeList), (hint)))
#define RwFreeListFree(freeList, mem) (lbl_805F15B4->rwfreelistfree((freeList), (mem)))
#define rwstrlen(string) (lbl_805F15B4->rwstrlen(string))
#define rwstrcpy(dst, src) (lbl_805F15B4->rwstrcpy((dst), (src)))
#define RwRenderStateSet(state, value) (lbl_805F15B4->renderStateSet((state), (value)))
#define RwRenderStateGet(state, value) (lbl_805F15B4->renderStateGet((state), (value)))

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

/* A free list: fixed size entries allocated in blocks (0x24 bytes; two in one unit's .bss are 0x24
 * apart) */
struct RwFreeList {
    /* 0x00 */ RwUInt8 field_0x0[0x24];
};

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

/* A link of a doubly linked list */
typedef struct RwLLLink {
    struct RwLLLink *next;
    struct RwLLLink *prev;
} RwLLLink;

/* A doubly linked list: its link is the head and the tail sentinel */
typedef struct RwLinkList {
    RwLLLink link;
} RwLinkList;

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
    /* 0x020 */ RwUInt8 field_0x20[0x74];
    /* 0x094 */ RwFrustumPlane frustumPlanes[6];
    /* 0x10C */ RwBBox frustumBoundBox;
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
    /* 0x10 */ void *firstRegEntry;
    /* 0x14 */ void *lastRegEntry;
} RwPluginRegistry;

/* plugin registries (inferred) */
/* each returns the registry, or NULL on failure */
/* constructs the plugins' data */
RwPluginRegistry *fn_8048209C(RwPluginRegistry *registry, void *object);
/* reads an object's plugin data chunks */
RwPluginRegistry *fn_804816AC(RwPluginRegistry *registry, RwStream *stream, void *object);
/* calls the rights callback of the plugin with the ID */
RwPluginRegistry *fn_80481870(RwPluginRegistry *registry, RwUInt32 pluginID, void *object,
                              RwUInt32 extraData);
/* destructs and copies the plugins' data */
RwPluginRegistry *fn_80482144(RwPluginRegistry *registry, void *object);
RwPluginRegistry *fn_804821B0(RwPluginRegistry *registry, void *dst, const void *src);
/* registering plugins: the plugin's offset in the object (or stream), or -1 */
RwInt32 fn_80481DC8(RwPluginRegistry *registry, RwInt32 size, RwUInt32 pluginID,
                    RwPluginObjectConstructor constructor, RwPluginObjectDestructor destructor,
                    RwPluginObjectCopy copy);
RwInt32 fn_804815F0(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwPluginDataChunkReadCallBack read, RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize);
RwInt32 fn_80481634(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwBool (*callBack)(void *object, RwInt32 offset, RwInt32 size));
RwInt32 fn_80481670(RwPluginRegistry *registry, RwUInt32 pluginID,
                    RwBool (*callBack)(void *object, RwInt32 offset, RwInt32 size,
                                       RwUInt32 extraData));
RwInt32 fn_80481D98(RwPluginRegistry *registry, RwUInt32 pluginID); /* the plugin's offset */

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

/* The resources module's globals in the engine; only the use order is mapped */
typedef struct RwResourcesGlobals {
    /* 0x00 */ RwUInt8 field_0x0[0x24];
    /* 0x24 */ RwLLLink *usedEntries; /* most recently used first */
} RwResourcesGlobals;

extern RwInt32 lbl_805F1578; /* engine plugin offset of the resources module */

#define RWRESOURCESGLOBAL(var) (RWPLUGINOFFSET(RwResourcesGlobals, lbl_805F15B4, lbl_805F1578)->var)

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

/* A texture: its raster and reference count (0x58 bytes) */
typedef struct RwTexture {
    /* 0x00 */ RwRaster *raster;
    /* 0x04 */ RwUInt8 field_0x4[0x50];
    /* 0x54 */ RwInt32 refCount;
} RwTexture;

typedef struct RwLine {
    RwV3d start;
    RwV3d end;
} RwLine;

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

/* optimises a matrix's flags, with a tolerance or the default one for NULL (inferred) */
RwMatrix *fn_8047E990(RwMatrix *matrix, const void *tolerance);

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

/* matrices (inferred) */
RwMatrix *fn_8047F994(void);             /* create */
RwBool fn_8047F958(RwMatrix *matrix);    /* destroy */
RwMatrix *fn_8047EBA0(RwMatrix *matrix); /* clears the identity and orthonormal flags */
RwMatrix *fn_8047F59C(RwMatrix *matrix, const RwV3d *translation, RwInt32 combine);
RwMatrix *fn_8047EFC8(RwMatrix *matrix, const RwV3d *axis, RwReal angle, RwInt32 combine);
RwMatrix *fn_8047F72C(RwMatrix *matrix, const RwMatrix *transform, RwInt32 combine);

typedef struct RwSphere {
    RwV3d center;
    RwReal radius;
} RwSphere;

/* streams */
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
RwUInt32 fn_80480D4C(RwStream *stream, void *buffer, RwUInt32 length);
RwStream *fn_80480ED0(RwStream *stream, const void *buffer, RwUInt32 length);
RwStream *fn_804810FC(RwStream *stream, RwUInt32 offset);

/* frames */
RwMatrix *RwFrameGetLTM(RwFrame *frame);

/* maths and intersection tests (inferred from their use) */
RwReal fn_80482340(RwReal x); /* sqrt(x) */
RwReal fn_80482394(RwReal x); /* 1 / sqrt(x) */

/* whether a triangle meets a box */
RwBool fn_8047BDD0(const RwBBox *box, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2);

/* whether a triangle meets a sphere, with the triangle's normal and the distance of the sphere's
 * center from its plane */
RwBool fn_8047B720(const RwSphere *sphere, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2,
                   RwV3d *normal, RwReal *distance);

/* whether a sphere is in a camera's view frustum (inferred) */
RwInt32 fn_804856C8(const RwCamera *camera, const RwSphere *sphere);

/* free lists (inferred) */
RwFreeList *fn_8047FBD8(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwUInt32 hint); /* create */
RwFreeList *fn_8047FC04(RwInt32 entrySize, RwInt32 entriesPerBlock, RwInt32 alignment,
                        RwInt32 numBlocksToPreallocate, RwFreeList *freeList, RwUInt32 hint);
RwBool fn_8047FCDC(RwFreeList *freeList);

/* frames (inferred) */
RwInt32 fn_804836EC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB);
RwInt32 fn_804878D4(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB);
RwFrame *fn_8048722C(RwFrame *frame);
RwFrame *fn_80486C5C(RwFrame *root); /* clones a hierarchy */
RwFrame *fn_80486CA4(RwFrame *root); /* forgets the clones made of a hierarchy */
RwBool fn_80486CF0(RwFrame *frame);  /* whether the frame's hierarchy is dirty */
RwBool fn_80487208(RwFrame *frame);  /* destroys a hierarchy */

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

/* cameras (inferred) */
RwCamera *fn_80483590(RwStream *stream); /* stream read */
RwBool fn_804859F8(RwCamera *camera);    /* destroy */

/* transforms a point by a matrix and returns out (inferred) */
RwV3d *fn_80482458(RwV3d *out, const RwV3d *point, const RwMatrix *matrix);

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
RwError *fn_8047DB84(const RwError *error);
RwInt32 fn_8047DC28(RwInt32 code, ...);

/* the engine and its plugins */
RwInt32 fn_80485D6C(void);
RwInt32 fn_80485D78(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor);
RwInt32 fn_80485DB8(RwUInt32 pluginID); /* the engine plugin offset of a plugin, or -1 */

#ifdef __cplusplus
}
#endif

#endif
