#ifndef RW_RPSKIN_H
#define RW_RPSKIN_H

#include "rw/rphanim.h"
#include "rw/rpworld.h"

/* RenderWare skinning (plugin ID 0x116): the bone weights and indices of a geometry's vertices, the
 * matrices that take the skin to each bone, and the split of the skin's meshes into bone-limited
 * batches. The type and field names are inferred from the stream format and the code that uses
 * them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_SKINPLUGIN 0x116

/* The weights of up to four bones on a vertex */
typedef struct RwMatrixWeights {
    RwReal w0;
    RwReal w1;
    RwReal w2;
    RwReal w3;
} RwMatrixWeights;

/* A two-byte record of the split data: a mesh's first run (its low seven bits index meshRLE) and
 * number of runs, or a run's first bone and number of bones (inferred) */
typedef struct SkinRLE {
    RwUInt8 start;
    RwUInt8 length;
} SkinRLE;

/* How the skin's meshes are split so that each batch uses at most boneLimit bones: the bone
 * remap table, then each mesh's runs, then the runs; the records follow numBones bytes, so they are
 * byte pairs */
typedef struct SkinSplitData {
    /* 0x00 */ RwUInt32 boneLimit;
    /* 0x04 */ RwUInt32 numMeshes;
    /* 0x08 */ RwUInt32 numRLE;
    /* 0x0C */ RwUInt8 *matrixRemapIndices; /* numBones entries; the block's allocation */
    /* 0x10 */ SkinRLE *meshRLECount;       /* numMeshes entries */
    /* 0x14 */ SkinRLE *meshRLE;            /* numRLE entries */
} SkinSplitData;

/* A skin (0x4C bytes) */
typedef struct RpSkin {
    /* 0x00 */ RwUInt32 numBones;
    /* 0x04 */ RwUInt32 numBoneIds; /* the bones that some vertex uses */
    /* 0x08 */ RwUInt8 *boneIds;
    /* 16-byte aligned when the core allocates the skin; 4-byte aligned in the native stream
     * reader's single block */
    /* 0x0C */ RwMatrix *skinToBoneMatrices;
    /* 0x10 */ RwUInt32 maxNumWeightsForVertex;
    /* 0x14 */ RwUInt32 *vertexBoneIndices; /* four byte-sized bone indices per vertex */
    /* 0x18 */ RwMatrixWeights *vertexBoneWeights;
    /* 0x1C */ void *positions;       /* GameCube: the instanced bind-pose positions (inferred) */
    /* 0x20 */ void *normals;         /* and normals (inferred) */
    /* 0x24 */ RwUInt8 *weights;      /* GameCube: the vertex weights as bytes, heaviest first */
    /* 0x28 */ RwUInt8 *indices;      /* GameCube: the bone indices in the same order */
    /* 0x2C */ RwUInt32 pipelineType; /* which skin pipeline renders it (inferred) */
    /* 0x30 */ SkinSplitData skinSplitData;
    /* 0x48 */ void *unaligned; /* the allocation of the bone ids, matrices, indices and weights */
} RpSkin;

/* The skin data of an atomic (0xC bytes) */
typedef struct SkinAtomicData {
    /* 0x0 */ RpHAnimHierarchy *hierarchy;
    /* 0x4 */ void *positions; /* GameCube: the instanced bind-pose positions (inferred) */
    /* 0x8 */ void *normals;
} SkinAtomicData;

/* The plugin's globals (0x40 bytes) */
typedef struct SkinGlobals {
    /* 0x00 */ RwInt32 engineOffset;
    /* 0x04 */ RwInt32 atomicOffset;
    /* 0x08 */ RwInt32 geometryOffset;
    /* 0x0C */ RwMatrix *matrixCache; /* 16-byte aligned, inside matrixCacheUnaligned */
    /* 0x10 */ void *matrixCacheUnaligned;
    /* 0x14 */ RwUInt32 field_0x14;
    /* 0x18 */ RwFreeList *freeList; /* RpSkin entries */
    /* 0x1C */ RwUInt32 field_0x1C;
    /* 0x20 */ RwInt32 numInstances; /* engine plugin constructions */
    /* 0x24 */ void *pipelines[3];   /* by skin pipeline type (inferred) */
    /* 0x30 */ RwUInt8 field_0x30[0x10];
} SkinGlobals;

extern SkinGlobals lbl_8056D708;

/* The header of an instanced vertex buffer: the GX draw sync token of its last use, then the
 * buffer's resource entry just before the 32-byte aligned vertices (inferred) */
typedef struct SkinVertexBuffer {
    /* 0x0 */ RwUInt16 syncToken;
} SkinVertexBuffer;

#define SKINVERTEXBUFFERENTRY(vertices) (((RwResEntry **)(vertices))[-1])
#define SKINVERTEXBUFFER(vertices) ((SkinVertexBuffer *)(SKINVERTEXBUFFERENTRY(vertices) + 1))

#define RPSKINATOMICGETDATA(atomic)                                                                \
    RWPLUGINOFFSET(SkinAtomicData, atomic, lbl_8056D708.atomicOffset)
#define RPSKINGEOMETRYGETDATA(geometry)                                                            \
    (*RWPLUGINOFFSET(RpSkin *, geometry, lbl_8056D708.geometryOffset))

/* skins */
RwBool fn_8044BAB4(RpSkin *skin); /* destroys the split data */
RwStream *fn_8044BB18(RwStream *stream, RpSkin *skin);
RwStream *fn_8044BBFC(RwStream *stream, RpSkin *skin);
RwInt32 fn_8044BDBC(RpSkin *skin);
RpSkin *fn_8044BDF0(RwUInt32 numVertices, RwUInt32 numBones, RwUInt32 numBoneIds,
                    RwUInt32 maxNumWeights, RwMatrixWeights *vertexWeights, RwUInt32 *vertexIndices,
                    RwMatrix *inverseMatrices);
RwBool fn_8044CB90(void); /* attaches the plugin */
RpAtomic *fn_8044CC98(RpAtomic *atomic, RpHAnimHierarchy *hierarchy);
RpSkin *fn_8044CCAC(RpGeometry *geometry);
RpGeometry *fn_8044CCC0(RpGeometry *geometry, RpSkin *skin);
RpSkin *fn_8044CD4C(RpSkin *skin);
RwUInt32 *fn_8044CDBC(RpSkin *skin);

/* GameCube (inferred from their use) */
/* sorts the weights heaviest first and makes the byte weights and indices for the kernels */
RpSkin *fn_8044E408(RpSkin *skin, RwUInt32 numVertices);
RpAtomic *fn_8044D924(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh); /* instance */
RpAtomic *fn_8044DA3C(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh); /* reinstance */
RpAtomic *fn_8044DE74(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh); /* render */
/* loads a bone matrix into the GX position (and normal) matrix at index */
void fn_8044DC4C(RwMatrix *matrix, RwUInt32 index, RwBool normals);
/* loads the bone matrices of a split mesh */
void fn_8044DD1C(RpSkin *skin, RwInt32 meshIndex, RwBool normals);
RpGeometry *fn_8044E6BC(RpGeometry *geometry);                 /* instances a skinned geometry */
RpGeometry *fn_8044E758(RpGeometry *geometry);                 /* releases a skin's instance data */
RwInt32 fn_8044E7D0(RpGeometry *geometry);                     /* native stream size */
RwStream *fn_8044E89C(RwStream *stream, RpGeometry *geometry); /* native stream write */
RwStream *fn_8044EAD0(RwStream *stream, RpGeometry *geometry); /* native stream read */
RwInt32 fn_8044EE3C(const void *object, RwInt32 offset, RwInt32 size); /* atomic stream size */
RwBool fn_80451D60(RwUInt32 flags);                                    /* opens the pipelines */
RwBool fn_80451E0C(void);                                              /* closes the pipelines */
RpAtomic *fn_80451E6C(RpAtomic *atomic, RwInt32 type);                 /* sets the skin pipeline */

#ifdef __cplusplus
}
#endif

#endif
