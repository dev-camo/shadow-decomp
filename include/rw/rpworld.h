#ifndef RW_RPWORLD_H
#define RW_RPWORLD_H

#include "rw/rwcore.h"

/* RenderWare world objects; only the members used by recovered code are mapped. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpGEOMETRY 8 /* RwObject type of a geometry */

typedef struct RpGeometry RpGeometry;

typedef struct RpMorphTarget {
    /* 0x00 */ RpGeometry *parentGeom;
    /* 0x04 */ RwSphere boundingSphere;
    /* 0x14 */ RwV3d *verts;
    /* 0x18 */ RwV3d *normals;
} RpMorphTarget;

struct RpGeometry {
    /* 0x00 */ RwUInt8 type;
    /* 0x01 */ RwUInt8 field_0x1[0x2F];
    /* 0x30 */ RwRGBA *preLitLum;
    /* 0x34 */ RwTexCoords *texCoords;
    /* 0x38 */ RwUInt8 field_0x38[0x24];
    /* 0x5C */ RpMorphTarget *morphTarget;
};

typedef struct RpAtomic RpAtomic;

typedef RpAtomic *(*RpAtomicCallBackRender)(RpAtomic *atomic);

/* An atomic (0x70 bytes) */
struct RpAtomic {
    /* 0x00 */ RwUInt8 field_0x0[0x18];
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

/* plugin registration (inferred): fn_80458B6C registers with the registry used by the object code
 * around fn_804566D0, which the intersection tests call on an atomic (the registry's 0x70-byte
 * object size alone does not identify it); fn_8045994C and fn_8045998C register with geometries
 * (0x60 bytes); fn_8045F2F8 and fn_8045F338 with world sectors (0x88), whose plugin data the
 * intersection tests read */
RwInt32 fn_80458B6C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045994C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045998C(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);
/* atomic stream callbacks that run on every load and for the plugin's rights (inferred) */
RwInt32 fn_80458C28(RwUInt32 pluginID,
                    RwBool (*callBack)(void *object, RwInt32 offset, RwInt32 size));
RwInt32 fn_80458C5C(RwUInt32 pluginID, RwBool (*callBack)(void *object, RwInt32 offset,
                                                          RwInt32 size, RwUInt32 extraData));

/* geometry locking (inferred: bracket every edit of a geometry's vertex streams) */
RpGeometry *fn_8045916C(RpGeometry *geometry, RwInt32 lockMode);
RpGeometry *fn_804591C4(RpGeometry *geometry);

RwInt32 fn_8045F2F8(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy);
RwInt32 fn_8045F338(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize);

#ifdef __cplusplus
}
#endif

#endif
