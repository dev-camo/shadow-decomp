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

struct RpGeometry {
    /* 0x00 */ RwUInt8 type;
    /* 0x01 */ RwUInt8 field_0x1[0x7];
    /* 0x08 */ RwUInt32 flags;
    /* 0x0C */ RwUInt8 field_0xC[0x8];
    /* 0x14 */ RwInt32 numVertices;
    /* 0x18 */ RwUInt8 field_0x18[0x8];
    /* 0x20 */ RpMaterial **materials; /* the material list's materials */
    /* 0x24 */ RwUInt8 field_0x24[0xC];
    /* 0x30 */ RwRGBA *preLitLum;
    /* 0x34 */ RwTexCoords *texCoords;
    /* 0x38 */ RwUInt8 field_0x38[0x1C];
    /* 0x54 */ RpMeshHeader *mesh;
    /* 0x58 */ RwUInt8 field_0x58[0x4];
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

/* An atomic (0x70 bytes) */
struct RpAtomic {
    /* 0x00 */ RwObject object; /* the parent is the atomic's frame */
    /* 0x08 */ RwUInt8 field_0x8[0x10];
    /* 0x18 */ RpGeometry *geometry;
    /* 0x1C */ RwUInt8 field_0x1C[0x2C];
    /* 0x48 */ RpAtomicCallBackRender renderCallBack;
    /* 0x4C */ RwUInt8 field_0x4C[0x20];
    /* 0x6C */ void *pipeline;
};

/* A triangle of a world sector (8 bytes). */
typedef struct RpTriangle {
    /* 0x0 */ RwUInt16 vertIndex[3];
    /* 0x6 */ RwInt16 matIndex;
} RpTriangle;

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

typedef struct RpWorldSector {
    /* 0x00 */ RwInt32 type;
    /* 0x04 */ RpTriangle *triangles;
    /* 0x08 */ RwV3d *vertices;
    /* 0x0C */ RwUInt8 field_0xC[0x54];
    /* 0x60 */ RwBBox boundingBox;
} RpWorldSector;

typedef struct RpWorld {
    /* 0x00 */ RwUInt8 field_0x0[0x1C];
    /* 0x1C */ RpSector *rootSector;
} RpWorld;

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

/* geometries (inferred from their use) */
RpGeometry *fn_80458D24(RwReal radius);   /* an empty geometry with a bounding sphere of radius */
RwBool fn_8045985C(RpGeometry *geometry); /* destroy */
/* sets the material of a triangle, adding it to the geometry's material list */
RpGeometry *fn_8045905C(RpGeometry *geometry, RpTriangle *triangle, RpMaterial *material);

/* materials (inferred from their use) */
RpMaterial *fn_8045B13C(void);                                     /* create */
RwBool fn_8045B1F4(RpMaterial *material);                          /* destroy */
RpMaterial *fn_8045B270(RpMaterial *material, RwTexture *texture); /* set texture */

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

/* GameCube: a mesh's display list (inferred) */
typedef struct RpGameCubeDisplayList {
    /* 0x0 */ void *data;
    /* 0x4 */ RwUInt32 size;
} RpGameCubeDisplayList;

/* GameCube: the instanced data of a geometry that follows its resource entry: its vertex
 * descriptors, then a display list record per mesh after the variable descriptor table; only the
 * mapped members are used (inferred) */
typedef struct RpGameCubeInstanceData {
    /* 0x00 */ RwUInt16 syncToken; /* GX draw sync token of the last draw */
    /* 0x02 */ RwUInt8 field_0x2[0x2];
    /* 0x04 */ RwUInt32 flags;
    /* 0x08 */ RwInt32 numVertexDescriptors;
    /* 0x0C */ RpGameCubeVertexDescriptor vertexDescriptors[1]; /* numVertexDescriptors entries */
} RpGameCubeInstanceData;

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
void fn_8049B800(void *pipeline);                        /* destroys a pipeline */

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

extern RwInt32 lbl_805F1658; /* raster plugin offset */

#ifdef __cplusplus
}
#endif

#endif
