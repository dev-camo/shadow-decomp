#ifndef RW_RPWORLD_H
#define RW_RPWORLD_H

#include "rw/rwcore.h"
#include <stddef.h>

/* RenderWare world objects; only the members used by recovered code are mapped. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpGEOMETRY 8 /* RwObject type of a geometry */

typedef struct RpGeometry RpGeometry;

/* RpGeometry flags */
#define rpGEOMETRYTRISTRIP 0x1
#define rpGEOMETRYPOSITIONS 0x2
#define rpGEOMETRYTEXTURED 0x4
#define rpGEOMETRYPRELIT 0x8
#define rpGEOMETRYNORMALS 0x10
#define rpGEOMETRYLIGHT 0x20
#define rpGEOMETRYMODULATEMATERIALCOLOR 0x40
#define rpGEOMETRYTEXTURED2 0x80
#define rpGEOMETRYNATIVE 0x01000000 /* instanced in the platform's format */
#define rpGEOMETRYNATIVEINSTANCE 0x02000000

typedef struct RwSurfaceProperties {
    RwReal ambient;
    RwReal specular;
    RwReal diffuse;
} RwSurfaceProperties;

/* A material (0x1C bytes) */
typedef struct RpMaterial {
    /* 0x00 */ RwTexture *texture;
    /* 0x04 */ RwRGBA color;
    /* 0x08 */ RxPipeline *pipeline;
    /* 0x0C */ RwSurfaceProperties surfaceProps;
    /* 0x18 */ RwInt16 refCount;
    /* 0x1A */ RwInt16 pad;
} RpMaterial;

typedef RwUInt16 RxVertexIndex;

/* A mesh: the triangles of a geometry that share a material */
typedef struct RpMesh {
    /* 0x0 */ RxVertexIndex *indices;
    /* 0x4 */ RwUInt32 numIndices;
    /* 0x8 */ RpMaterial *material;
} RpMesh;

/* The header of a geometry's meshes, which follow it */
typedef struct RpMeshHeader {
    /* 0x0 */ RwUInt32 flags;
    /* 0x4 */ RwUInt16 numMeshes;
    /* 0x6 */ RwUInt16 serialNum;
    /* 0x8 */ RwUInt32 totalIndicesInMesh;
    /* 0xC */ RwUInt32 firstMeshOffset;
} RpMeshHeader;

/* RpMeshHeader flags: the primitive type of the meshes */
#define rpMESHHEADERTRISTRIP 0x1
#define rpMESHHEADERTRIFAN 0x2
#define rpMESHHEADERLINELIST 0x4
#define rpMESHHEADERPOLYLINE 0x8
#define rpMESHHEADERPOINTLIST 0x10
#define rpMESHHEADERPRIMMASK 0xFF

typedef struct RpMorphTarget {
    /* 0x00 */ RpGeometry *parentGeom;
    /* 0x04 */ RwSphere boundingSphere;
    /* 0x14 */ RwV3d *verts;
    /* 0x18 */ RwV3d *normals;
} RpMorphTarget;

/* A triangle of a geometry or world sector (8 bytes). */
typedef struct RpTriangle {
    /* 0x0 */ RwUInt16 vertIndex[3];
    /* 0x6 */ RwUInt16 matIndex; /* 0xFFFF for none */
} RpTriangle;

/* A list of materials (0xC bytes) */
typedef struct RpMaterialList {
    /* 0x0 */ RpMaterial **materials;
    /* 0x4 */ RwInt32 numMaterials;
    /* 0x8 */ RwInt32 space;
} RpMaterialList;

/* A geometry (0x60 bytes) */
struct RpGeometry {
    /* 0x00 */ RwObject object;
    /* 0x08 */ RwUInt32 flags;
    /* 0x0C */ RwUInt16 lockedSinceLastInst; /* rpGEOMETRYLOCK* */
    /* 0x0E */ RwInt16 refCount;
    /* 0x10 */ RwInt32 numTriangles;
    /* 0x14 */ RwInt32 numVertices;
    /* 0x18 */ RwInt32 numMorphTargets;
    /* 0x1C */ RwInt32 numTexCoordSets;
    /* 0x20 */ RpMaterialList matList;
    /* 0x2C */ RpTriangle *triangles;
    /* 0x30 */ RwRGBA *preLitLum;
    /* 0x34 */ RwTexCoords *texCoords[8]; /* numTexCoordSets sets */
    /* 0x54 */ RpMeshHeader *mesh;
    /* 0x58 */ RwResEntry *repEntry;
    /* 0x5C */ RpMorphTarget *morphTarget;
};

typedef struct RpAtomic RpAtomic;

typedef RpAtomic *(*RpAtomicCallBackRender)(RpAtomic *atomic);

/* the atomic render callback installed when none is given (inferred) */
RpAtomic *fn_80455EB4(RpAtomic *atomic);

#define RpAtomicSetRenderCallBack(atomic, callback)                                                \
    do {                                                                                           \
        (atomic)->renderCallBack = (callback);                                                     \
        if ((atomic)->renderCallBack == NULL) {                                                    \
            (atomic)->renderCallBack = fn_80455EB4;                                                \
        }                                                                                          \
    } while (0)

typedef struct RpClump RpClump;

/* The morph target interpolation of an atomic (0x14 bytes) */
typedef struct RpInterpolator {
    /* 0x00 */ RwInt32 flags; /* rpINTERPOLATOR* */
    /* 0x04 */ RwInt16 startMorphTarget;
    /* 0x06 */ RwInt16 endMorphTarget;
    /* 0x08 */ RwReal time;
    /* 0x0C */ RwReal recipTime;
    /* 0x10 */ RwReal position;
} RpInterpolator;

/* RpInterpolator flags */
#define rpINTERPOLATORDIRTYINSTANCE 0x1
#define rpINTERPOLATORDIRTYSPHERE 0x2

/* RpAtomic flags (the object flags) */
#define rpATOMICCOLLISIONTEST 0x1
#define rpATOMICRENDER 0x4

#define rpATOMIC 1 /* RwObject type of an atomic */
#define rpCLUMP 2  /* RwObject type of a clump */

/* the private flag of a frame-attached object whose world bounding sphere is out of date */
#define rpATOMICSPHEREDIRTY 0x1

/* the sync callback of an object attached to a frame */

/* An atomic (0x70 bytes) */
struct RpAtomic {
    /* 0x00 */ RwObject object; /* the parent is the atomic's frame */
    /* 0x08 */ RwLLLink lFrame; /* in the frame's object list */
    /* 0x10 */ RwObjectHasFrameSyncFunction sync;
    /* 0x14 */ RwResEntry *repEntry;
    /* 0x18 */ RpGeometry *geometry;
    /* 0x1C */ RwSphere boundingSphere;
    /* 0x2C */ RwSphere worldBoundingSphere;
    /* 0x3C */ RpClump *clump;
    /* 0x40 */ RwLLLink inClumpLink;
    /* 0x48 */ RpAtomicCallBackRender renderCallBack;
    /* 0x4C */ RpInterpolator interpolator;
    /* 0x60 */ RwUInt16 renderFrame;
    /* 0x62 */ RwUInt16 pad;
    /* 0x64 */ RwLinkList llWorldSectorsInAtomic;
    /* 0x6C */ RxPipeline *pipeline;
};

typedef RpClump *(*RpClumpCallBack)(RpClump *clump, void *data);

/* A clump: atomics, lights and cameras under one frame (0x2C bytes) */
struct RpClump {
    /* 0x00 */ RwObject object; /* the parent is the clump's frame */
    /* 0x08 */ RwLinkList atomicList;
    /* 0x10 */ RwLinkList lightList;
    /* 0x18 */ RwLinkList cameraList;
    /* 0x20 */ RwLLLink inWorldLink;
    /* 0x28 */ RpClumpCallBack callback;
};

/* The clump plugin data of a light or camera (0xC bytes) */
typedef struct RpClumpObjectExtension {
    /* 0x0 */ RpClump *clump;
    /* 0x4 */ RwLLLink inClumpLink;
} RpClumpObjectExtension;

/* A node of a world's sector tree: a plane sector (type: the split axis as a byte offset into an
 * RwV3d) or, with a negative type, a world sector. */
typedef struct RpSector {
    /* 0x0 */ RwInt32 type;
} RpSector;

typedef struct RpPlaneSector {
    /* 0x00 */ RwInt32 type;
    /* 0x04 */ RwReal value;
    /* 0x08 */ RpSector *leftSubTree;
    /* 0x0C */ RpSector *rightSubTree;
    /* 0x10 */ RwReal leftValue;
    /* 0x14 */ RwReal rightValue;
} RpPlaneSector;

/* A compressed vertex normal (4 bytes) */
typedef struct RpVertexNormal {
    /* 0x0 */ RwInt8 x;
    /* 0x1 */ RwInt8 y;
    /* 0x2 */ RwInt8 z;
    /* 0x3 */ RwUInt8 pad;
} RpVertexNormal;

/* A leaf of a world's sector tree: the world's geometry inside its box (0x88 bytes) */
typedef struct RpWorldSector {
    /* 0x00 */ RwInt32 type; /* rpWORLDSECTORTYPE */
    /* 0x04 */ RpTriangle *triangles;
    /* 0x08 */ RwV3d *vertices;
    /* 0x0C */ RpVertexNormal *normals;
    /* 0x10 */ RwTexCoords *texCoords[8]; /* the world's numTexCoordSets sets */
    /* 0x30 */ RwRGBA *preLitLum;
    /* 0x34 */ RwResEntry *repEntry;
    /* 0x38 */ RwLinkList collAtomicsInWorldSector;
    /* 0x40 */ RwLinkList lightsInWorldSector;
    /* 0x48 */ RwBBox field_0x48;
    /* 0x60 */ RwBBox boundingBox; /* as in the stream */
    /* 0x78 */ RpMeshHeader *mesh;
    /* 0x7C */ RxPipeline *pipeline;
    /* 0x80 */ RwUInt16 matListWindowBase;
    /* 0x82 */ RwUInt16 numVertices;
    /* 0x84 */ RwUInt16 numTriangles;
    /* 0x86 */ RwUInt16 pad;
} RpWorldSector;

#define rpWORLDSECTORTYPE -1 /* the RpSector type of a world sector */

#define rpWORLD 7 /* RwObject type of a world */

/* RpWorld flags (the stream's format) */
#define rpWORLDTRISTRIP 0x1
#define rpWORLDPRELIT 0x8
#define rpWORLDNORMALS 0x10
#define rpWORLDTEXTURED 0x4
#define rpWORLDTEXTURED2 0x80
#define rpWORLDNATIVE 0x1000000
#define rpWORLDNATIVEINSTANCE 0x2000000
#define rpWORLDLIGHT 0x20
#define rpWORLDSECTORSOVERLAP 0x40000000 /* plane sectors keep separate left and right values */
#define rpWORLDTEXCOORDSETS(flags) (((flags) & 0xFF0000) >> 16)

/* RpWorld private flags */
#define rpWORLDSINGLEMALLOC 0x1 /* the world and its sectors are one allocation */

/* renders a world sector (inferred) */
typedef RpWorldSector *(*RpWorldSectorCallBackRender)(RpWorldSector *sector);

/* A world: a tree of plane sectors over world sectors, and the clumps and lights in it (0x70
 * bytes) */
typedef struct RpWorld {
    /* 0x00 */ RwObject object;
    /* 0x08 */ RwUInt32 flags; /* rpWORLD* */
    /* 0x0C */ RwInt32 renderOrder;
    /* 0x10 */ RpMaterialList matList;
    /* 0x1C */ RpSector *rootSector;
    /* 0x20 */ RwInt32 numTexCoordSets;
    /* 0x24 */ RwInt32 numClumpsInWorld;
    /* 0x28 */ RwLLLink *currentClumpLink;
    /* 0x2C */ RwLinkList clumpList;
    /* 0x34 */ RwLinkList lightList;
    /* 0x3C */ RwLinkList directionalLightList;
    /* 0x44 */ RwV3d worldOrigin;
    /* 0x50 */ RwBBox boundingBox;
    /* 0x68 */ RpWorldSectorCallBackRender renderCallBack; /* for each sector (fn_80460120) */
    /* 0x6C */ RxPipeline *pipeline;
} RpWorld;

extern RwPluginRegistry lbl_8056D880; /* world sector plugins */
extern RwPluginRegistry lbl_8056D898; /* world plugins */

/* worlds (inferred) */
RpWorld *fn_804556F4(RwStream *stream); /* stream read */
RwBool fn_8045FFA0(RpWorld *world);     /* destroy */
/* finishes a world read from a stream; the world, or NULL on failure */
RpWorld *fn_8045FDB0(RpWorld *world);
void fn_8045FBA4(RpWorld *world);
void fn_8045FCFC(RpWorld *world, RwUInt32 size); /* takes the world's allocation size */
/* sets the world sector render callback, or the default one for NULL */
RpWorld *fn_80460120(RpWorld *world, RpWorldSectorCallBackRender callBack);

/* A tie between an atomic and a world sector it is in (0x18 bytes) */
typedef struct RpTie {
    /* 0x00 */ RwLLLink lAtomicInWorldSector; /* in the sector's atomic list */
    /* 0x08 */ RpAtomic *apAtom;
    /* 0x0C */ RwLLLink lWorldSectorInAtomic; /* in the atomic's sector list */
    /* 0x14 */ RpWorldSector *worldSector;
} RpTie;

/* The tie of a link in an atomic's sector list */
#define TIEFROMATOMICLINK(link)                                                                    \
    ((RpTie *)((RwUInt8 *)(link) - offsetof(RpTie, lWorldSectorInAtomic)))

/* A tie between a light and a world sector it lights (0x18 bytes) */
typedef struct RpLightTie {
    /* 0x00 */ RwLLLink lightInWorldSector; /* in the sector's light list */
    /* 0x08 */ struct RpLight *light;
    /* 0x0C */ RwLLLink WorldSectorInLight; /* in the light's sector list */
    /* 0x14 */ RpWorldSector *sect;
} RpLightTie;

/* world sectors (inferred): destroying a sector tree in a world allocated as one block, or with
 * separate allocations */
void fn_8045F6E0(RpSector *sector);
void fn_8045F7D0(RpSector *sector);

/* world sectors (inferred) */
RpWorldSector *fn_8045F390(RpWorldSector *sector); /* renders a sector and its atomics */

/* A search of a world for one of its sectors (inferred) */
typedef struct RpWorldSectorSearch {
    /* 0x0 */ RpWorldSector *sector;
    /* 0x4 */ RwBool found;
} RpWorldSectorSearch;

RpWorldSector *fn_8045F374(RpWorldSector *sector, void *data); /* the search callback */

typedef RpWorldSector *(*RpWorldSectorCallBack)(RpWorldSector *sector, void *data);

/* calls back for each world sector until the callback returns NULL */
RpWorld *fn_804606F0(RpWorld *world, RpWorldSectorCallBack callBack, void *data);
/* calls back for each world sector in a camera's view frustum (inferred) */
RwCamera *fn_804624E8(RwCamera *camera, RpWorldSectorCallBack callBack, void *data);

/* engine plugin constructors and destructors of the world modules (inferred from their
 * registration in fn_8046080C) */
void *fn_8045B048(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045B0D8(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045BFBC(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045BF64(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80458CF8(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80458D10(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80456A50(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80456994(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045AE98(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045AE20(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045F2A0(void *instance, RwInt32 offset, RwInt32 size);
void *fn_8045F2B4(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80455CF8(void *instance, RwInt32 offset, RwInt32 size);
void *fn_80455CE4(void *instance, RwInt32 offset, RwInt32 size);

/* plugin data stream callbacks: the clump's, the world's and the material's rights (inferred) */
RwStream *fn_80455D4C(RwStream *stream, RwInt32 length, void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_80455DC8(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size);
RwInt32 fn_80455E44(const void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_80454AA4(RwStream *stream, RwInt32 length, void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_80454B20(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size);
RwInt32 fn_80454B9C(const void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_80454BC4(RwStream *stream, RwInt32 length, void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_80454C40(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size);
RwInt32 fn_80454CBC(const void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_8045AF28(RwStream *stream, RwInt32 length, void *object, RwInt32 offset, RwInt32 size);
RwStream *fn_8045AFA4(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size);
RwInt32 fn_8045B020(const void *object, RwInt32 offset, RwInt32 size);

/* native geometry and world sector data in a stream (inferred) */
RwInt32 fn_8046510C(const RpGeometry *geometry);
RwStream *fn_80465138(RwStream *stream, const RpGeometry *geometry);
RpGeometry *fn_80465180(RwStream *stream, RpGeometry *geometry);
RwInt32 fn_804651AC(const RpWorldSector *sector);
RwStream *fn_80465200(RwStream *stream, const RpWorldSector *sector);
RpWorldSector *fn_80465260(RwStream *stream, RpWorldSector *sector);

/* the other world plugin attachments, each TRUE on success (inferred) */
RwBool fn_80456B48(void);
RwBool fn_80461E20(void);
RwBool fn_8046318C(void);

/* the pipeline module's open and close (inferred) */
RwBool fn_80462798(void);
void fn_80462770(void);

/* the platform pipelines (inferred): the open steps, each TRUE on success, and the close steps */
RwBool fn_804631CC(void);
RwBool fn_80463298(void);
RwBool fn_804631D8(void);
void fn_80463244(void);
void fn_80463308(void);
void fn_804631D4(void);

/* ties (inferred): unlink a tie from both its lists and free it */
RwBool fn_80460D80(RpLightTie *tie);
RwBool fn_80460DF0(RpTie *tie);

/* material lists (inferred) */
RpMaterialList *fn_8045B924(RwStream *stream, RpMaterialList *matList); /* stream read */

/* plugin registration (inferred): fn_80458B6C registers with the registry used by the object code
 * around fn_804566D0, which the intersection tests call on an atomic (the registry's 0x70-byte
 * object size alone does not identify it); fn_8045994C and fn_8045998C register with geometries
 * (0x60 bytes); fn_8045F2F8 and fn_8045F338 with world sectors (0x88), whose plugin data the
 * intersection tests read */
RwInt32 fn_80458B6C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_80458BEC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);
RwInt32 fn_80458C90(RwUInt32 pluginID); /* the atomic plugin offset of a plugin */
RwInt32 fn_8045994C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045998C(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);
/* atomic stream callbacks that run on every load and for the plugin's rights (inferred) */
RwInt32 fn_80458C28(RwUInt32 pluginID,
                    RwBool (*callBack)(void *object, RwInt32 offset, RwInt32 size));
RwInt32 fn_80458C5C(RwUInt32 pluginID, RwBool (*callBack)(void *object, RwInt32 offset,
                                                          RwInt32 size, RwUInt32 extraData));

/* atomics (inferred from their use) */
RpAtomic *fn_80456DD8(void);                                                   /* create */
RwBool fn_80456FD8(RpAtomic *atomic);                                          /* destroy */
RpAtomic *fn_80456F0C(RpAtomic *atomic, RpGeometry *geometry, RwUInt32 flags); /* set geometry */
RpAtomic *fn_80458CBC(RpAtomic *atomic, RwFrame *frame);                       /* set frame */
RwSphere *fn_804566D0(RpAtomic *atomic); /* the world bounding sphere, updated */
RpWorld *fn_804622F0(RpAtomic *atomic);  /* the world the atomic is in, or NULL */

/* RpAtomicSetGeometry flags */
#define rpATOMICSAMEBOUNDINGSPHERE 0x1

/* The pipeline module's globals in the engine; only the pipelines are mapped */
typedef struct RpPipelineGlobals {
    /* 0x00 */ RwUInt8 field_0x0[0x3C];
    /* 0x3C */ RxPipeline *defaultAtomicPipeline; /* the current defaults */
    /* 0x40 */ RxPipeline *defaultWorldSectorPipeline;
    /* 0x44 */ RxPipeline *defaultMaterialPipeline;
    /* 0x48 */ RxPipeline *genericAtomicPipeline;
    /* 0x4C */ RxPipeline *genericWorldSectorPipeline;
    /* 0x50 */ RxPipeline *genericMaterialPipeline;
    /* 0x54 */ RxPipeline *platformAtomicPipeline;
    /* 0x58 */ RxPipeline *platformWorldSectorPipeline;
    /* 0x5C */ RxPipeline *platformMaterialPipeline;
} RpPipelineGlobals;

extern RwInt32 lbl_805F1690; /* engine plugin offset of the pipeline module's globals */

/* clumps (inferred) */
RpClump *fn_804573A8(void);                             /* create */
RwBool fn_80457468(RpClump *clump);                     /* destroy */
RpClump *fn_80457090(RpClump *clump);                   /* clone */
RpClump *fn_80457608(RpClump *clump, RpAtomic *atomic); /* add an atomic */
RpClump *fn_80457698(RwStream *stream);                 /* stream read */

/* lights (inferred) */
/* A light (0x40 bytes): its object subtype is the light type and its private flag says whether its
 * colour is white */
typedef struct RpLight {
    /* 0x00 */ RwObject object; /* the parent is the light's frame */
    /* 0x08 */ RwLLLink lFrame;
    /* 0x10 */ RwObjectHasFrameSyncFunction sync;
    /* 0x14 */ RwReal radius;
    /* 0x18 */ RwRGBAReal color;
    /* 0x28 */ RwReal minusCosAngle;
    /* 0x2C */ RwLinkList WorldSectorsInLight;
    /* 0x34 */ RwLLLink inWorld;
    /* 0x3C */ RwUInt16 lightFrame;
    /* 0x3E */ RwUInt16 pad;
} RpLight;

typedef RpAtomic *(*RpAtomicCallBack)(RpAtomic *atomic, void *data);
typedef RpLight *(*RpLightCallBack)(RpLight *light, void *data);
typedef RwCamera *(*RwCameraCallBack)(RwCamera *camera, void *data);

/* calling back for a clump's atomics, cameras and lights until the callback returns NULL */
RpClump *fn_80456C54(RpClump *clump, RpAtomicCallBack callBack, void *data);
RpClump *fn_80456CD0(RpClump *clump, RwCameraCallBack callBack, void *data);
RpClump *fn_80456D54(RpClump *clump, RpLightCallBack callBack, void *data);
RwInt32 fn_80458BAC(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy); /* clumps */

/* world objects (inferred) */
RpWorld *fn_80462130(RpWorld *world, RwCamera *camera); /* adds a camera */
RpWorld *fn_80462184(RpWorld *world, RwCamera *camera); /* removes a camera */
RpWorld *fn_804621C0(RpWorld *world, RpAtomic *atomic); /* adds an atomic */
RpWorld *fn_8046221C(RpWorld *world, RpAtomic *atomic); /* removes an atomic */
RpWorld *fn_804622FC(RpWorld *world, RpClump *clump);   /* adds a clump */
RpWorld *fn_804623EC(RpWorld *world, RpClump *clump);   /* removes a clump */
RpWorld *fn_80462578(RpWorld *world, RpLight *light);   /* adds a light */
RpWorld *fn_80462618(RpWorld *world, RpLight *light);   /* removes a light */
RwBool fn_80462884(void);                               /* (inferred) */
RpWorld *fn_8045FEA0(RpWorldSector *sector);            /* the world a sector belongs to */
/* collects the world sectors in a camera's view frustum */
RpWorld *fn_80461658(RpWorld *world, RwCamera *camera);
/* clumps */

#define rpLIGHT 3 /* RwObject type of a light */

/* RpLightType: a light's object subtype */
typedef enum RpLightType {
    rpNALIGHTTYPE = 0,
    rpLIGHTDIRECTIONAL = 1,
    rpLIGHTAMBIENT = 2,
    rpLIGHTPOINT = 0x80,
    rpLIGHTSPOT = 0x81,
    rpLIGHTSPOTSOFT = 0x82
} RpLightType;

#define RpLightGetType(light) ((RpLightType)(light)->object.subType)

/* RpLight flags */
#define rpLIGHTLIGHTATOMICS 0x1
#define rpLIGHTLIGHTWORLD 0x2

#define rpLIGHTPRIVATENOCHROMA 0x1 /* the colour is white or grey */

RpLight *fn_8045AD34(RwInt32 type);       /* create */
RwReal fn_8045A818(const RpLight *light); /* the cone angle of a spot light, in radians */
RpLight *fn_8045AB1C(RwStream *stream);   /* stream read */
RwBool fn_8045ACD4(RpLight *light);       /* destroy */

/* The geometries of a clump stream (8 bytes) */
typedef struct RpGeometryList {
    /* 0x0 */ RpGeometry **geometries;
    /* 0x4 */ RwInt32 numGeoms;
} RpGeometryList;

RpGeometryList *fn_80455F18(RwStream *stream, RpGeometryList *geomList); /* stream read */
RpGeometry *fn_804599C8(RwStream *stream);                               /* geometry stream read */
RpGeometry *fn_8045984C(RpGeometry *geometry);                           /* add a reference */

/* RpGeometry lock flags */
#define rpGEOMETRYLOCKPOLYGONS 0x1
#define rpGEOMETRYLOCKALL 0xFFF

/* the texture coordinate sets of an RpGeometry format */
#define rpGEOMETRYTEXCOORDSETS(flags) (((flags) & 0xFF0000) >> 16)

/* material lists (inferred) */
RpMaterialList *fn_8045B788(RpMaterialList *matList);                  /* initialise */
RpMaterialList *fn_8045B6F0(RpMaterialList *matList);                  /* deinitialise */
RpMaterial *fn_8045B79C(const RpMaterialList *matList, RwInt32 index); /* the material at index */
RwInt32 fn_8045B7AC(RpMaterialList *matList, RpMaterial *material);    /* append: its index or -1 */
RwInt32 fn_8045B8EC(const RpMaterialList *matList, const RpMaterial *material); /* index or -1 */

/* A triangle of a mesh being built (0x14 bytes) */
typedef struct RpBuildMeshTriangle {
    /* 0x00 */ RxVertexIndex vertIndex[3];
    /* 0x08 */ RpMaterial *material;
    /* 0x0C */ RwUInt16 matIndex;
    /* 0x0E */ RwUInt16 textureIndex;
    /* 0x10 */ RwUInt16 rasterIndex;
    /* 0x12 */ RwUInt16 pipelineIndex;
} RpBuildMeshTriangle;

/* A mesh being built from triangles (0xC bytes) */
typedef struct RpBuildMesh {
    /* 0x0 */ RwUInt32 triangleBufferSize; /* in triangles */
    /* 0x4 */ RwUInt32 numTriangles;
    /* 0x8 */ RpBuildMeshTriangle *meshTriangles;
} RpBuildMesh;

/* meshes and mesh building (inferred) */
RwBool fn_8045C2D4(RpMeshHeader *mesh);          /* destroy */
RpBuildMesh *fn_8045C144(RwUInt32 numTriangles); /* create */
RwBool fn_8045C26C(RpBuildMesh *buildMesh);      /* destroy */
/* adds a triangle with its material and the indices of its texture, raster and pipeline */
RpBuildMesh *fn_8045C340(RpBuildMesh *buildMesh, RpMaterial *material, RwUInt16 vert0,
                         RwUInt16 vert1, RwUInt16 vert2, RwUInt16 matIndex, RwUInt16 textureIndex,
                         RwUInt16 rasterIndex, RwUInt16 pipelineIndex);
/* the mesh header of a built mesh, as triangle strips (flags rpGEOMETRYTRISTRIP) or a triangle
 * list; it destroys the build mesh */
RpMeshHeader *fn_8045F1FC(RpBuildMesh *buildMesh, RwUInt32 flags);

/* builds a world sector's mesh (inferred) */
RpWorld *fn_8045F458(RpWorld *world, RpBuildMesh *buildMesh, RpWorldSector *sector,
                     RpMaterial **materials);

typedef RpMaterial *(*RpMaterialCallBack)(RpMaterial *material, void *data);

/* geometries (inferred from their use) */
RpGeometry *fn_80459464(RwInt32 numVertices, RwInt32 numTriangles, RwUInt32 format); /* create */
RwInt32 fn_80458DB0(RpGeometry *geometry, RwInt32 numMorphTargets); /* the first new one, or -1 */
RpGeometry *fn_804590E8(RpGeometry *geometry, RpMaterialCallBack callBack, void *data);
/* geometries (inferred from their use) */
RpGeometry *fn_80458D24(RwReal radius);   /* an empty geometry with a bounding sphere of radius */
RwBool fn_8045985C(RpGeometry *geometry); /* destroy */
/* sets the material of a triangle, adding it to the geometry's material list */
RpGeometry *fn_8045905C(RpGeometry *geometry, RpTriangle *triangle, RpMaterial *material);

/* materials (inferred from their use) */
RpMaterial *fn_8045B13C(void);                                     /* create */
RwBool fn_8045B1F4(RpMaterial *material);                          /* destroy */
RpMaterial *fn_8045B270(RpMaterial *material, RwTexture *texture); /* set texture */
RpMaterial *fn_8045B348(RwStream *stream);                         /* stream read */

/* plugin registration with lights (inferred from RenderWare's attach order) and materials (the
 * registry is the one the material constructor fn_8045B13C uses) */
RwInt32 fn_8045AAA0(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045AAE0(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);
RwInt32 fn_8045B2CC(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045B30C(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);

/* The mesh module's globals in the engine (0x30 bytes) */
typedef struct RpMeshGlobals {
    /* 0x00 */ RwInt16 nextSerialNum;
    /* 0x04 */ RwFreeList *triStripListEntryFreeList; /* while triangle strips are built */
    /* the primitive type of each combination of rpMESHHEADER* primitive flags, and back */
    /* 0x08 */ RwUInt8 meshFlagsToPrimType[0x1F];
    /* 0x27 */ RwUInt8 primTypeToMeshFlags[7];
} RpMeshGlobals;

/* The mesh module (inferred) */
typedef struct RpMeshModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} RpMeshModule;

extern RpMeshModule lbl_805F14F0;

#define RPMESHGLOBAL(var)                                                                          \
    (RWPLUGINOFFSET(RpMeshGlobals, lbl_805F15B4, lbl_805F14F0.globalsOffset)->var)

typedef RpMesh *(*RpMeshCallBack)(RpMesh *mesh, RpMeshHeader *meshHeader, void *data);

/* meshes (inferred from their use) */
RpMeshHeader *fn_8045BF30(RwUInt32 size); /* allocates a mesh header */
RwInt16 fn_8045CA20(void);                /* the next mesh serial number */
RpMeshHeader *fn_8045C4E4(RpMeshHeader *meshHeader, RpMeshCallBack callBack, void *data);
RwStream *fn_8045C56C(const RpMeshHeader *meshHeader, const void *object, RwStream *stream,
                      const RpMaterialList *matList); /* stream write */
RpMeshHeader *fn_8045C770(RwStream *stream, const void *object,
                          const RpMaterialList *matList);                /* stream read */
RwInt32 fn_8045C9C8(const RpMeshHeader *meshHeader, const void *object); /* stream size */

/* geometry locking (inferred: bracket every edit of a geometry's vertex streams) */
RpGeometry *fn_8045916C(RpGeometry *geometry, RwInt32 lockMode);
RpGeometry *fn_804591C4(RpGeometry *geometry);

RwInt32 fn_8045F2F8(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045F338(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);

/* GameCube: the vertex format of a geometry (0x18 bytes, reference counted), kept in the geometry
 * plugin data at lbl_805F1520. The types are GX component types and the position and texture
 * coordinate fractions are GX fractional bits (inferred). */
typedef struct RpGameCubeVertexFormat {
    /* 0x00 */ RwUInt8 posType;
    /* 0x01 */ RwUInt8 normalType;
    /* 0x02 */ RwUInt8 texCoordType[8];
    /* 0x0A */ RwUInt8 colorType;
    /* 0x0B */ RwUInt8 field_0xB;
    /* 0x0C */ RwUInt8 posFrac;
    /* 0x0D */ RwUInt8 normalNbt; /* nonzero: normals, binormals and tangents (inferred) */
    /* 0x0E */ RwUInt8 texCoordFrac[8];
    /* 0x16 */ RwUInt16 refCount;
} RpGameCubeVertexFormat;

extern RwInt32 lbl_805F1520; /* geometry plugin offset of the vertex format */

#define GAMECUBEVERTEXFORMAT(geometry)                                                             \
    (*RWPLUGINOFFSET(RpGameCubeVertexFormat *, geometry, lbl_805F1520))

/* vertex formats (inferred from their use) */
RpGameCubeVertexFormat *fn_8046301C(void);                                    /* create */
void fn_80462FAC(RpGameCubeVertexFormat *format);                             /* initializes */
void fn_80462F7C(RpGameCubeVertexFormat *format, RwUInt8 type, RwUInt8 frac); /* positions */
void fn_80462F88(RpGameCubeVertexFormat *format, RwUInt8 type, RwUInt8 nbt);  /* normals */
void fn_80462FA4(RpGameCubeVertexFormat *format, RwUInt8 type);               /* colors */
/* texture coordinates of the 1-based set index */
void fn_80462F94(RpGameCubeVertexFormat *format, RwInt32 index, RwUInt8 type, RwUInt8 frac);
void fn_80463104(RpGeometry *geometry, RpGameCubeVertexFormat *format); /* attach */
void fn_804630C0(RpGameCubeVertexFormat *format);                       /* release */

/* GameCube: a vertex descriptor of instanced data: the buffer of one GX vertex attribute, the
 * attribute, its stride, and how it is sent (the GXSetVtxDesc mode); the component format comes
 * from the geometry's RpGameCubeVertexFormat */
typedef struct RpGameCubeVertexDescriptor {
    /* 0x0 */ void *data;
    /* 0x4 */ RwUInt8 attribute; /* GX_VA_* */
    /* 0x5 */ RwUInt8 stride;
    /* 0x6 */ RwUInt8 type; /* GXAttrType: the descriptor mode, e.g. GX_INDEX8 or GX_INDEX16 */
    /* 0x7 */ RwUInt8 field_0x7;
} RpGameCubeVertexDescriptor;

typedef struct RpGameCubeInstanceData RpGameCubeInstanceData;

/* GameCube: a built stream: its distinct values and their number (inferred) */
typedef struct RpGameCubeBuiltStream {
    /* 0x0 */ void *data;
    /* 0x4 */ RwUInt32 count;
} RpGameCubeBuiltStream;

/* GameCube: a source vertex stream for building instanced data: its data, its kind, and the
 * other streams whose indices go with it (-1 ends the list) (inferred) */
typedef struct RpGameCubeStreamSource {
    /* 0x00 */ void *data;
    /* 0x04 */ RwInt32 kind;
    /* 0x08 */ RwInt8 dependencies[4];
    /* 0x0C */ RwUInt8 field_0xC[0xC];
} RpGameCubeStreamSource;

/* GameCube: the duplicate values of a source stream: for each vertex, the earlier vertex with an
 * equal value, or -1 for a distinct value, and the number of distinct values (inferred) */
typedef struct RpGameCubeStreamRemap {
    /* 0x0 */ RwInt32 *remap;
    /* 0x4 */ RwUInt32 numValues;
} RpGameCubeStreamRemap;

/* building instanced data from source streams (inferred) */
RpGameCubeStreamRemap *fn_80466194(RpGameCubeStreamSource *streams, RwInt32 numStreams,
                                   RwInt32 numVertices);
RpGameCubeBuiltStream *fn_80465ED4(RpGameCubeStreamRemap *remaps, RpGameCubeStreamSource *streams,
                                   RwInt32 numStreams, RwInt32 numVertices);
/* each stream's indices into its built values for a mesh */
RxVertexIndex **fn_80465CDC(RpGameCubeStreamRemap *remaps, RxVertexIndex **indices,
                            RwInt32 numStreams, RwUInt32 numIndices);
/* the triangle strips of a mesh's indices */
void fn_8046D8C8(RxVertexIndex *indices, RwUInt32 numIndices, RwInt32 *numStrips,
                 RwUInt32 *numStripIndices, RwInt32 flags);
RpGameCubeVertexFormat *fn_80462E1C(void); /* the default vertex format */

/* GameCube: a builder of GX vertex descriptors and formats (0x20 bytes, inferred); only the
 * vertex attribute format, in the layout of the GX VAT A, B and C registers, is mapped */
typedef struct RpGameCubeVertexDescBuilder {
    /* 0x00 */ RwUInt8 field_0x0[0x4];
    /* 0x04 */ RwUInt32 vat[3];
    /* 0x10 */ RwUInt8 field_0x10[0x10];
} RpGameCubeVertexDescBuilder;

/* the byte sizes of a vertex's attributes in a builder's format */
RwUInt32 fn_8046693C(const RpGameCubeVertexDescBuilder *builder);                  /* position */
RwUInt32 fn_804669A0(const RpGameCubeVertexDescBuilder *builder);                  /* normal */
RwUInt32 fn_804669EC(const RpGameCubeVertexDescBuilder *builder, RwUInt8 channel); /* colour */
RwUInt32 fn_80466A54(const RpGameCubeVertexDescBuilder *builder,
                     RwUInt8 set); /* texture coordinates */

void fn_8046FC1C(RpGameCubeVertexDescBuilder *builder);                       /* initializes */
void fn_8046FC48(RpGameCubeVertexDescBuilder *builder, RwInt32 vertexFormat); /* starts */
void fn_8046FC50(RpGameCubeVertexDescBuilder *builder, RwInt32 attribute, RwInt32 count,
                 RwInt32 type, RwUInt8 frac); /* sets an attribute's format */
void fn_8046FF34(RpGameCubeVertexDescBuilder *builder, RwInt32 attribute,
                 RwInt32 mode); /* sets an attribute's descriptor mode */
void fn_8047046C(RpGameCubeVertexDescBuilder *builder, RwUInt8 numAttributes); /* finishes */

/* GameCube: the source of a geometry's vertex attributes, indexed by GX attribute (inferred) */
typedef struct RpGameCubeVertexSource {
    /* 0x00 */ RwUInt32 numElements[26];
    /* 0x68 */ void *data[26];
} RpGameCubeVertexSource;

/* GameCube: the indices of a mesh's vertex attributes, indexed by GX attribute (inferred) */
typedef struct RpGameCubeIndexSource {
    /* 0x00 */ void *indices[26];
} RpGameCubeIndexSource;

/* building GameCube instanced data and display lists (inferred) */
RwUInt32 fn_8046EE8C(RpGameCubeVertexDescBuilder *builder); /* the vertex data header size */
RwUInt32 fn_8046B094(RpGameCubeVertexDescBuilder *builder, RwInt32 numPrimitives,
                     RwUInt32 numIndices); /* a display list's size */
RwUInt32 fn_8046EEA0(RpGameCubeVertexDescBuilder *builder, RpGameCubeVertexSource *source);
void fn_8046B0E0(void *displayListRecord, RwInt32 meshIndex, RwUInt32 size, void *data);
RwInt32 fn_8046ACBC(RpGameCubeVertexDescBuilder *builder);
/* triStrip: the geometry's tristrip flag, selecting strip handling; flag: forwarded to the winding
 * and degenerate triangle handling (meaning not established) */
void fn_8046E594(RpGameCubeVertexDescBuilder *builder, void *displayListRecord,
                 RpGameCubeIndexSource *indices, RwUInt32 numIndices, RwInt32 triStrip,
                 RwInt32 vertexFormat, RwInt32 flag, RwInt32 primitive, RwInt32 flags);
void fn_8046F208(RpGameCubeVertexDescBuilder *builder, RpGameCubeInstanceData *instance,
                 RpGameCubeVertexSource *source, void *vertexData);
void fn_8046F6CC(RpGameCubeVertexDescBuilder *builder, RpGameCubeInstanceData *instance,
                 RpGameCubeVertexSource *source, RwInt32 firstVertex, RwInt32 flags);
extern RwUInt16 lbl_805F1678; /* a GameCube instance stamp (inferred) */

/* GameCube: a mesh's display list (inferred) */
typedef struct RpGameCubeDisplayList {
    /* 0x0 */ void *data;
    /* 0x4 */ RwUInt32 size;
} RpGameCubeDisplayList;

/* GameCube: the instanced data of a geometry that follows its resource entry: its vertex
 * descriptors, then a display list record per mesh after the variable descriptor table; only the
 * mapped members are used (inferred) */
struct RpGameCubeInstanceData {
    /* 0x00 */ RwUInt16 syncToken; /* GX draw sync token of the last draw */
    /* 0x02 */ RwUInt16 serialNum; /* of the geometry's mesh header */
    /* 0x04 */ RwUInt32 flags;
    /* 0x08 */ RwInt32 numVertexDescriptors;
    /* 0x0C */ RpGameCubeVertexDescriptor vertexDescriptors[1]; /* numVertexDescriptors entries */
};

/* The display list records of the meshes, which follow the instance's vertex descriptors. Written
 * as the second descriptor's address plus the remaining descriptors, which is how the original
 * computes it. */
#define RPGAMECUBEINSTANCEDISPLAYLISTS(instance)                                                   \
    ((RpGameCubeDisplayList *)&(instance)->vertexDescriptors[1] +                                  \
     ((instance)->numVertexDescriptors - 1))

/* the next GX draw sync token. Volatile: every function that issues a token rereads it after
 * comparing it, which only a volatile object explains */
extern volatile RwUInt16 lbl_805EDF40;

/* GX draw sync tokens wrap below this */
#define rwGAMECUBEMAXSYNCTOKEN 0xE000

/* the GX draw sync */
RwBool fn_8049A78C(RwUInt16 token); /* whether the GPU has passed the token */

/* Waits until the GPU is done with the draw that used a sync token, first issuing the token if it
 * is the current one */
#define RPGAMECUBEWAITDRAWSYNC(syncToken)                                                          \
    do {                                                                                           \
        if ((syncToken) == lbl_805EDF40) {                                                         \
            GXSetDrawSync(lbl_805EDF40);                                                           \
            lbl_805EDF40 = (lbl_805EDF40 + 1) % rwGAMECUBEMAXSYNCTOKEN;                            \
        }                                                                                          \
        while (!fn_8049A78C(syncToken)) {                                                          \
        }                                                                                          \
    } while (0)

/* the material color callback a render state needs (inferred) */
typedef void (*RpGameCubeMaterialColorCallBack)(RwRGBAReal *ambient, RwRGBA *color,
                                                RwReal ambientCoef);
RpGameCubeMaterialColorCallBack fn_804655D4(RwInt32 geometryFlags, RwUInt32 lightMask,
                                            RwBool hasAmbient, RwBool prelit);

struct RxGameCubeAllInOneData;

/* GameCube: what an all-in-one node passes to its callbacks: the instanced data, and what the
 * lighting callback leaves for the render callback */
typedef struct RpGameCubeRenderMesh {
    /* 0x00 */ RwResEntry *repEntry;
    /* 0x04 */ RpMeshHeader *meshHeader; /* of the geometry; its meshes follow it */
    /* 0x08 */ RwInt32 geometryFlags;
    /* 0x0C */ RwRGBAReal ambientColor; /* (inferred) */
    /* 0x1C */ RwBool hasAmbient;       /* (inferred) */
    /* 0x20 */ RwUInt32 lightMask;
    /* 0x24 */ RwInt32 numLights;
    /* 0x28 */ struct RxGameCubeAllInOneData *nodeData; /* the node's callbacks */
} RpGameCubeRenderMesh;

typedef RpAtomic *(*RpGameCubeInstanceCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
typedef RpAtomic *(*RpGameCubeLightingCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
typedef RpAtomic *(*RpGameCubeRenderCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);

/* GameCube: a callback of an all-in-one pipeline node, for the atomic or world sector being
 * rendered (inferred) */
typedef void *(*RxGameCubeAllInOneCallBack)(void *object, RpGameCubeRenderMesh *renderMesh);

/* GameCube: the private data of an all-in-one pipeline node (inferred) */
typedef struct RxGameCubeAllInOneData {
    /* 0x0 */ RxGameCubeAllInOneCallBack instanceCallBack;
    /* 0x4 */ RxGameCubeAllInOneCallBack reinstanceCallBack;
    /* 0x8 */ RxGameCubeAllInOneCallBack lightingCallBack;
    /* 0xC */ RxGameCubeAllInOneCallBack renderCallBack;
} RxGameCubeAllInOneData;

/* Calls one of an all-in-one node's callbacks if it is set: FALSE when it fails */
#define RXGAMECUBEALLINONECALL(callBack, object, renderMesh)                                       \
    ((callBack) == NULL || (callBack)((object), (renderMesh)) == (object))

/* setting an all-in-one node's callbacks */
void fn_80462BF0(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack); /* instance */
void fn_80462BFC(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack); /* reinstance */
void fn_80462C08(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack); /* lighting */
void fn_80462C14(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack); /* render */

/* the default render callback of atomics and world sectors */
void *fn_80462914(void *object, RpGameCubeRenderMesh *renderMesh);
/* waits until the GPU is done with a resource entry's instanced data */
void fn_8046288C(RwResEntry *repEntry);

typedef RpWorldSector *(*RpGameCubeSectorCallBack)(RpWorldSector *sector,
                                                   RpGameCubeRenderMesh *renderMesh);

/* creates a world sector all-in-one pipeline (inferred) */
RxPipeline *fn_80463D28(RwUInt32 pluginID, RwUInt32 pluginData,
                        RpGameCubeSectorCallBack instanceCallBack,
                        RpGameCubeSectorCallBack reinstanceCallBack,
                        RpGameCubeSectorCallBack lightingCallBack,
                        RpGameCubeSectorCallBack renderCallBack);
RpWorldSector *fn_80463DF4(RpWorldSector *sector, RpGameCubeRenderMesh *renderMesh); /* lighting */
RpWorldSector *fn_80463F10(RpWorldSector *sector, RpGameCubeRenderMesh *renderMesh); /* instance */
RxNodeDefinition *fn_804641C0(void); /* the world sector all-in-one node definition */
/* create the instanced data of a world sector for its owner (inferred) */
RwResEntry *fn_8046CB94(RpWorld *world, RpWorldSector *sector, void *owner, RwResEntry **ownerRef);
RwResEntry *fn_8046D4C8(RpWorld *world, RpWorldSector *sector, void *owner, RwResEntry **ownerRef);
RpAtomic *fn_804639AC(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh); /* atomic instance */
RwBool fn_804647C8(void); /* the light plugin attachment (inferred) */

/* creates an atomic all-in-one pipeline with the plugin's callbacks, which receive the atomic
 * being rendered (inferred) */
RxPipeline *fn_8046335C(RwUInt32 pluginID, RwUInt32 pluginData,
                        RpGameCubeInstanceCallBack instanceCallBack,
                        RpGameCubeInstanceCallBack reinstanceCallBack,
                        RpGameCubeLightingCallBack lightingCallBack,
                        RpGameCubeRenderCallBack renderCallBack);
RpAtomic *fn_80463428(RpAtomic *atomic,
                      RpGameCubeRenderMesh *renderMesh); /* the default lighting */
/* instances an atomic's vertices into its instanced data (inferred) */
RpAtomic *fn_80463840(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
RxNodeDefinition *fn_80463D1C(void); /* the atomic all-in-one node definition */

/* GameCube lights (inferred from their use) */
/* loads the world's directional lights with the light flags into the mesh's hardware lights and
 * adds its ambient lights to the mesh's ambient colour */
void fn_804641CC(RwUInt8 lightFlags, RpGameCubeRenderMesh *renderMesh);
void fn_804643E8(RpLight *light, RpGameCubeRenderMesh *renderMesh); /* adds a light */

/* GameCube instanced data (inferred from their use) */
/* creates the instanced data of a geometry for its owner, the atomic or the geometry itself */
RwResEntry *fn_8046B79C(RpGeometry *geometry, void *owner, RwResEntry **ownerRef);
RwResEntry *fn_8046C0D4(RpGeometry *geometry, void *owner, RwResEntry **ownerRef);
/* writes an atomic's interpolated morph targets into its instanced data */
void fn_80464814(RpGeometry *geometry, RpGameCubeInstanceData *instance,
                 RpInterpolator *interpolator);
/* write a stream into instanced data in a GX component type and return the size written;
 * positions are made relative to origin - fmod(origin, 1 / scale), the origin truncated toward
 * zero to a multiple of the fixed point step */
RwUInt32 fn_80466B58(void *data, const RwV3d *positions, RwUInt8 type, RwInt32 numVertices,
                     RwUInt8 stride, const RwV3d *origin, RwReal scale);
RwUInt32 fn_80467BC8(void *data, const RwV3d *normals, RwUInt8 type, RwInt32 numVertices,
                     RwUInt8 stride);
RwUInt32 fn_804699F0(void *data, const RwRGBA *colors, RwUInt8 type, RwInt32 numVertices,
                     RwUInt8 stride);
RwUInt32 fn_8046A270(void *data, const RwTexCoords *texCoords, RwUInt8 type, RwInt32 numVertices,
                     RwUInt8 stride, RwReal scale);
void fn_8049B800(void *pipeline); /* destroys a pipeline */

/* GameCube render helpers (inferred from their use) */
/* sets the vertex descriptors and formats for a vertex format and render mesh */
void fn_80462C30(RpGameCubeVertexFormat *format, RpGameCubeRenderMesh *renderMesh);
extern RwInt32 lbl_805EDDD8;    /* the instancing optimisation mode (inferred) */
RwInt32 fn_80462C28(void);      /* a GameCube render mode (lbl_805EDDD8) */
void fn_80462C20(RwInt32 mode); /* sets it */
void fn_804A2F08(void);         /* resets the material effects render state */
RwBool fn_804A8478(RpGameCubeRenderMesh *renderMesh); /* whether a mesh uses material effects */
/* builds the material effects data of a skinned geometry with binormals and tangents */
void fn_804A8120(RpGameCubeRenderMesh *renderMesh, RpGameCubeVertexFormat *format,
                 RwInt32 numVertices);
/* renders a mesh with material effects */
void fn_804A457C(RpMesh *mesh, RpGameCubeDisplayList *displayList, RpAtomic *atomic, RwMatrix *ltm,
                 RpGameCubeRenderMesh *renderMesh);
RwBool fn_804A2064(RpAtomic *atomic);        /* whether an atomic has material effects */
extern RpGameCubeVertexFormat *lbl_805F16B8; /* the vertex format of material effects geometries */

void fn_804950AC(RwMatrix *matrix, RwBool normals);  /* loads the position (and normal) matrix */
void fn_80499C38(RwTexture *texture, RwInt32 stage); /* sets a texture */
void fn_80497778(RwBool opaque);                     /* sets alpha compare for opaque textures */

/* The GameCube raster plugin data; only the flags are mapped */
typedef struct RwGameCubeRasterExt {
    /* 0x00 */ RwUInt8 field_0x0[0x14];
    /* 0x14 */ RwUInt32 flags;
} RwGameCubeRasterExt;

extern RwInt32 lbl_805F1658; /* raster plugin offset */

#ifdef __cplusplus
}
#endif

#endif
