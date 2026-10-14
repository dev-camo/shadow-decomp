#ifndef RW_RPWORLD_H
#define RW_RPWORLD_H

#include "rw/rwcore.h"

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
#define rpGEOMETRYTEXTURED2 0x80
#define rpGEOMETRYNATIVE 0x01000000 /* instanced in the platform's format */

typedef struct RwSurfaceProperties {
    RwReal ambient;
    RwReal specular;
    RwReal diffuse;
} RwSurfaceProperties;

/* A material (0x1C bytes) */
typedef struct RpMaterial {
    /* 0x00 */ RwTexture *texture;
    /* 0x04 */ RwRGBA color;
    /* 0x08 */ void *pipeline;
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
typedef void *(*RwObjectHasFrameSyncFunction)(void *object);

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
#define rpWORLDPRELIT 0x8
#define rpWORLDNORMALS 0x10
#define rpWORLDTEXTURED 0x4
#define rpWORLDTEXTURED2 0x80
#define rpWORLDNATIVE 0x1000000
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

/* world sectors (inferred): destroying a sector tree in a world allocated as one block, or with
 * separate allocations */
void fn_8045F6E0(RpSector *sector);
void fn_8045F7D0(RpSector *sector);

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

/* The pipeline module's globals in the engine; only the default atomic pipeline is mapped */
typedef struct RpPipelineGlobals {
    /* 0x00 */ RwUInt8 field_0x0[0x3C];
    /* 0x3C */ RxPipeline *defaultAtomicPipeline;
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

#define rpLIGHT 3 /* RwObject type of a light */

/* RpLight flags */
#define rpLIGHTLIGHTATOMICS 0x1
#define rpLIGHTLIGHTWORLD 0x2

#define rpLIGHTPRIVATENOCHROMA 0x1 /* the colour is white or grey */

RpLight *fn_8045AD34(RwInt32 type);     /* create */
RpLight *fn_8045AB1C(RwStream *stream); /* stream read */
RwBool fn_8045ACD4(RpLight *light);     /* destroy */

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

/* meshes and mesh building (inferred) */
typedef struct RpBuildMesh RpBuildMesh;
RwBool fn_8045C2D4(RpMeshHeader *mesh);         /* destroy */
RpBuildMesh *fn_8045C144(RwInt32 numTriangles); /* create */
RwBool fn_8045C26C(RpBuildMesh *buildMesh);     /* destroy */
/* adds a triangle with its material and the indices of its texture, raster and pipeline */
RpBuildMesh *fn_8045C340(RpBuildMesh *buildMesh, RpMaterial *material, RwUInt16 vert0,
                         RwUInt16 vert1, RwUInt16 vert2, RwUInt16 matIndex, RwUInt16 textureIndex,
                         RwUInt16 rasterIndex, RwUInt16 pipelineIndex);
/* the mesh header of a built mesh, as triangle strips or a triangle list */
RpMeshHeader *fn_8045F1FC(RpBuildMesh *buildMesh, RwBool triStrip);

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

/* The mesh module's globals in the engine; only the mapped members are used */
typedef struct RpMeshGlobals {
    /* 0x00 */ RwInt16 nextSerialNum;
    /* 0x02 */ RwUInt8 field_0x2[0x2B];
    /* 0x2D */ RwUInt8 primType; /* of new mesh headers (inferred) */
} RpMeshGlobals;

extern RwInt32 lbl_805F14F0; /* engine plugin offset of the mesh module's globals */

/* meshes (inferred from their use) */
RpMeshHeader *fn_8045BF30(RwUInt32 size); /* allocates a mesh header */
RwUInt16 fn_8045CA20(void);               /* the next mesh serial number */

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

/* building instanced data from source streams (inferred) */
void *fn_80466194(RpGameCubeStreamSource *streams, RwInt32 numStreams, RwInt32 numVertices);
RpGameCubeBuiltStream *fn_80465ED4(void *builder, RpGameCubeStreamSource *streams,
                                   RwInt32 numStreams, RwInt32 numVertices);
/* each stream's indices into its built values for a mesh */
RxVertexIndex **fn_80465CDC(void *builder, RxVertexIndex **indices, RwInt32 numStreams,
                            RwUInt32 numIndices);
/* the triangle strips of a mesh's indices */
void fn_8046D8C8(RxVertexIndex *indices, RwUInt32 numIndices, RwInt32 *numStrips,
                 RwUInt32 *numStripIndices, RwInt32 flags);
RpGameCubeVertexFormat *fn_80462E1C(void); /* the default vertex format */

/* GameCube: a builder of GX vertex descriptors and formats (0x20 bytes, inferred) */
typedef struct RpGameCubeVertexDescBuilder {
    /* 0x00 */ RwUInt8 field_0x0[0x20];
} RpGameCubeVertexDescBuilder;

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

extern RwUInt16 lbl_805EDF40; /* the next GX draw sync token */

/* GameCube: what the all-in-one pipeline's lighting callback leaves for its render callback */
typedef struct RpGameCubeRenderMesh {
    /* 0x00 */ RwResEntry *repEntry;
    /* 0x04 */ RpMeshHeader *meshHeader; /* of the geometry; its meshes follow it */
    /* 0x08 */ RwInt32 geometryFlags;
    /* 0x0C */ RwRGBAReal ambientColor; /* (inferred) */
    /* 0x1C */ RwBool hasAmbient;       /* (inferred) */
    /* 0x20 */ RwUInt32 lightMask;
    /* 0x24 */ RwInt32 numLights;
} RpGameCubeRenderMesh;

typedef RpAtomic *(*RpGameCubeInstanceCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
typedef RpAtomic *(*RpGameCubeLightingCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
typedef RpAtomic *(*RpGameCubeRenderCallBack)(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);

/* creates an atomic all-in-one pipeline with the plugin's callbacks, which receive the atomic
 * being rendered (inferred) */
void *fn_8046335C(RwUInt32 pluginID, RwUInt32 pluginData,
                  RpGameCubeInstanceCallBack instanceCallBack,
                  RpGameCubeInstanceCallBack reinstanceCallBack,
                  RpGameCubeLightingCallBack lightingCallBack,
                  RpGameCubeRenderCallBack renderCallBack);
RpAtomic *fn_80463428(RpAtomic *atomic,
                      RpGameCubeRenderMesh *renderMesh); /* the default lighting */
/* instances an atomic's vertices into its instanced data (inferred) */
RpAtomic *fn_80463840(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh);
void fn_8049B800(void *pipeline); /* destroys a pipeline */

/* GameCube render helpers (inferred from their use) */
/* sets the vertex descriptors and formats for a vertex format and render mesh */
void fn_80462C30(RpGameCubeVertexFormat *format, RpGameCubeRenderMesh *renderMesh);
RwInt32 fn_80462C28(void);                            /* a GameCube render mode (lbl_805EDDD8) */
void fn_80462C20(RwInt32 mode);                       /* sets it */
void fn_804A2F08(void);                               /* resets the material effects render state */
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
