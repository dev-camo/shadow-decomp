/* RenderWare skinning plugin (plugin ID 0x116), GameCube: the skin's native data (the byte weights
 * and indices the kernels read) and its native stream format, and the instancing of skinned
 * geometries for the skin pipeline. The original file name is unknown. */
#include "rw/rpskin.h"
#include <dolphin/gx.h>
#include <string.h>

#define rwMEMHINT_SKIN (rwMEMHINTDUR_EVENT | rpID_SKINPLUGIN)
/* for memory freed before the allocating function returns */
#define rwMEMHINT_SKINTEMP (rwMEMHINTDUR_FUNCTION | rpID_SKINPLUGIN)

/* the platform of a native skin in the stream */
#define rwPLATFORMGAMECUBE 6

/* the destroy callback of generic GameCube instanced data (inferred) */
void fn_8046288C(RwResEntry *repEntry);

/* Prepares a geometry's skin for instancing: native geometries have their vertices and normals
 * marked for reinstancing, others get the byte weights the kernels read */
RpGeometry *fn_8044E6BC(RpGeometry *geometry) {
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);

    if (skin != NULL) {
        if (geometry->flags & rpGEOMETRYNATIVE) {
            if (geometry->flags & rpGEOMETRYNORMALS) {
                geometry->lockedSinceLastInst |= rpGEOMETRYLOCKVERTICES | rpGEOMETRYLOCKNORMALS;
            } else {
                geometry->lockedSinceLastInst |= rpGEOMETRYLOCKVERTICES;
            }
            skin->positions = NULL;
            skin->normals = NULL;
        } else if (fn_8044E408(skin, geometry->numVertices) == NULL) {
            return NULL;
        }
    }
    return geometry;
}

/* Releases a geometry's skin data for the kernels */
RpGeometry *fn_8044E758(RpGeometry *geometry) {
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);

    if (skin->weights != NULL) {
        RwFree(skin->weights);
        skin->weights = NULL;
        skin->indices = NULL;
    }
    skin->positions = NULL;
    skin->normals = NULL;
    return geometry;
}

/* Gives a geometry's bind-pose buffers back to its instanced data, which the generic instancing
 * then owns */
static inline void SkinReleaseBindPose(RpGeometry *geometry, RpSkin *skin) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(geometry->repEntry + 1);
    RwUInt32 flags = geometry->flags;

    SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data)->ownerRef = NULL;
    instance->vertexDescriptors[0].data = skin->positions;
    if (flags & rpGEOMETRYNORMALS) {
        instance->vertexDescriptors[1].data = skin->normals;
    }
    geometry->repEntry->destroyNotify = fn_8046288C;
    skin->positions = NULL;
    skin->normals = NULL;
}

/* The size of a native skin in the stream; the bind-pose buffers are given back first */
RwInt32 fn_8044E7D0(RpGeometry *geometry) {
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);
    /* the struct chunk header, the platform and the counts, then the used bones */
    RwInt32 size = skin->numBoneIds + 20;

    if (skin->maxNumWeightsForVertex > 1) {
        size += geometry->numVertices * (skin->maxNumWeightsForVertex * 2);
    }
    size += skin->numBones * sizeof(RwMatrix);
    if (skin->positions != NULL) {
        SkinReleaseBindPose(geometry, skin);
    }
    size += fn_8044BDBC(skin);
    return size;
}

/* Writes a native skin: a struct chunk with the platform, the counts, the used bones, the byte
 * indices and weights, the matrices and the split data */
RwStream *fn_8044E89C(RwStream *stream, RpGeometry *geometry) {
    RwUInt32 maxNumWeights;
    RwInt32 platform = rwPLATFORMGAMECUBE;
    RwInt32 numVertices;
    RwUInt32 size;
    RwUInt32 counts;
    RpSkin *skin;

    if (fn_8047CEF0(stream, rwID_STRUCT, fn_8044E7D0(geometry) - 12, RW_STREAM_VERSION,
                    RW_STREAM_BUILD) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, &platform, sizeof(RwInt32)) == NULL) {
        return NULL;
    }

    numVertices = geometry->numVertices;
    skin = RPSKINGEOMETRYGETDATA(geometry);
    /* the byte lengths below use this count, read with the packed counts */
    maxNumWeights = skin->maxNumWeightsForVertex;
    counts =
        ((maxNumWeights & 0xFF) << 16) | ((skin->numBoneIds & 0xFF) << 8) | (skin->numBones & 0xFF);
    if (fn_8047D458(stream, (RwInt32 *)&counts, sizeof(RwUInt32)) == NULL) {
        return NULL;
    }
    if (fn_80480ED0(stream, skin->boneIds, skin->numBoneIds) == NULL) {
        return NULL;
    }
    if (skin->maxNumWeightsForVertex > 1) {
        size = maxNumWeights * numVertices;
        if (fn_80480ED0(stream, skin->indices, size) == NULL) {
            return NULL;
        }
        if (fn_80480ED0(stream, skin->weights, size) == NULL) {
            return NULL;
        }
    }
    if (fn_80480ED0(stream, skin->skinToBoneMatrices, skin->numBones * sizeof(RwMatrix)) == NULL) {
        return NULL;
    }
    if (fn_8044BB18(stream, skin) != NULL) {
        return stream;
    }
    return NULL;
}

/* Reads a native skin; its byte weights and indices, matrices and used bones share one block */
RwStream *fn_8044EAD0(RwStream *stream, RpGeometry *geometry) {
    RwInt32 platform;
    RwUInt32 counts;
    RwUInt32 length;
    RwUInt32 version;
    RwError error;
    RpSkin *skin;
    RwInt32 numVertices;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version < 0x35000 || version > RW_STREAM_VERSION) {
        error.pluginID = rpID_SKINPLUGIN;
        error.errorCode = fn_8047DC28(E_RW_BADVERSION);
        fn_8047DB84(&error);
        return NULL;
    }
    if (fn_8047D868(stream, &platform, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (platform != rwPLATFORMGAMECUBE) {
        return NULL;
    }

    skin = RwFreeListAlloc(lbl_8056D708.freeList, rwMEMHINT_SKIN);
    memset(skin, 0, sizeof(RpSkin));
    if (fn_8047D868(stream, (RwInt32 *)&counts, sizeof(RwUInt32)) == NULL) {
        return NULL;
    }
    skin->numBones = counts & 0xFF;
    skin->numBoneIds = (counts >> 8) & 0xFF;
    skin->maxNumWeightsForVertex = (counts >> 16) & 0xFF;
    numVertices = geometry->numVertices;

    /* the rest of the chunk, after the platform and the counts */
    length -= 8;
    if (skin->maxNumWeightsForVertex > 1) {
        skin->weights = RwMalloc(length + 5, rwMEMHINT_SKIN);
        skin->indices = skin->weights + skin->maxNumWeightsForVertex * numVertices;
        skin->indices = (RwUInt8 *)(((RwUInt32)skin->indices + 3) & ~3);
        skin->skinToBoneMatrices =
            (RwMatrix *)(skin->indices + skin->maxNumWeightsForVertex * numVertices);
        skin->skinToBoneMatrices = (RwMatrix *)(((RwUInt32)skin->skinToBoneMatrices + 3) & ~3);
        skin->boneIds = (RwUInt8 *)(skin->skinToBoneMatrices + skin->numBones);

        length = skin->numBoneIds;
        if (fn_80480D4C(stream, skin->boneIds, length) != length) {
            return NULL;
        }
        length = skin->maxNumWeightsForVertex * numVertices;
        if (fn_80480D4C(stream, skin->indices, length) != length) {
            return NULL;
        }
        length = skin->maxNumWeightsForVertex * numVertices;
        if (fn_80480D4C(stream, skin->weights, length) != length) {
            return NULL;
        }
        length = skin->numBones * sizeof(RwMatrix);
        if (fn_80480D4C(stream, skin->skinToBoneMatrices, length) != length) {
            return NULL;
        }
    } else {
        skin->weights = RwMalloc(length + 3, rwMEMHINT_SKIN);
        skin->skinToBoneMatrices = (RwMatrix *)skin->weights;
        skin->skinToBoneMatrices = (RwMatrix *)(((RwUInt32)skin->skinToBoneMatrices + 3) & ~3);
        skin->boneIds = (RwUInt8 *)(skin->skinToBoneMatrices + skin->numBones);

        length = skin->numBoneIds;
        if (fn_80480D4C(stream, skin->boneIds, length) != length) {
            return NULL;
        }
        length = skin->numBones * sizeof(RwMatrix);
        if (fn_80480D4C(stream, skin->skinToBoneMatrices, length) != length) {
            return NULL;
        }
    }

    if (fn_8044BBFC(stream, skin) == NULL) {
        return NULL;
    }
    fn_8044CCC0(geometry, skin);
    return stream;
}

/* The size of an atomic's skin chunk: none, but the atomic's bind-pose buffers are given back to
 * its instanced data first */
RwInt32 fn_8044EE3C(const void *object, RwInt32 offset, RwInt32 size) {
    RpAtomic *atomic = (RpAtomic *)object;
    SkinAtomicData *atomicData = RPSKINATOMICGETDATA(atomic);

    if (atomicData->positions != NULL) {
        RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(atomic->repEntry + 1);

        SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data)->ownerRef = NULL;
        instance->vertexDescriptors[0].data = atomicData->positions;
        if (atomic->geometry->flags & rpGEOMETRYNORMALS) {
            instance->vertexDescriptors[1].data = atomicData->normals;
        }
        atomic->repEntry->destroyNotify = fn_8046288C;
        atomicData->positions = NULL;
        atomicData->normals = NULL;
    }
    return 0;
}

/* Builds the native vertex data and per-mesh display lists of a skinned geometry from its source
 * streams: positions, normals, prelit colors and texture coordinates, then the bone indices and
 * weights (bytes for skins with several weights per vertex). Positions and normals depend on the
 * indices and weights, and those on the positions and normals. */
RwBool fn_8044EEAC(RpGeometry *geometry, RpGameCubeBuiltStream **builtStreams,
                   RxVertexIndex ****meshIndexLists) {
    RpGameCubeStreamSource streams[16];
    RpMorphTarget *morphTarget;
    RpSkin *skin;
    void *builder;
    RxVertexIndex **meshIndices;
    RwInt32 numStreams;
    RwInt32 numMeshes;
    RwInt32 numSkinned;
    RwInt32 i;
    RwUInt32 j; /* compared unsigned */

    if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
        fn_80462E1C();
    }

    morphTarget = geometry->morphTarget;
    numStreams = 1;
    streams[0].data = morphTarget->verts;
    streams[0].kind = 8;
    streams[0].dependencies[0] = -1;
    if (geometry->flags & rpGEOMETRYNORMALS) {
        streams[numStreams].data = morphTarget->normals;
        numStreams = 2;
        streams[1].kind = 8;
        streams[1].dependencies[0] = -1;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        streams[numStreams].data = geometry->preLitLum;
        streams[numStreams].kind = 4;
        streams[numStreams].dependencies[0] = -1;
        numStreams++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < geometry->numTexCoordSets; i++) {
            streams[numStreams].data = geometry->texCoords[i];
            streams[numStreams].kind = 7;
            streams[numStreams].dependencies[0] = -1;
            numStreams++;
        }
    }

    skin = fn_8044CCAC(geometry);
    if (skin->maxNumWeightsForVertex > 1) {
        streams[numStreams].data = skin->indices;
        switch (skin->maxNumWeightsForVertex) {
        case 1:
            streams[numStreams].kind = 1;
            break;
        case 2:
            streams[numStreams].kind = 2;
            break;
        case 3:
            streams[numStreams].kind = 3;
            break;
        case 4:
            streams[numStreams].kind = 4;
            break;
        }
    } else {
        streams[numStreams].data = skin->vertexBoneIndices;
        streams[numStreams].kind = 4;
    }
    streams[numStreams].dependencies[0] = -1;
    if (skin->maxNumWeightsForVertex > 1) {
        streams[numStreams + 1].data = skin->weights;
        streams[numStreams + 1].kind = streams[numStreams].kind;
        streams[numStreams + 1].dependencies[0] = -1;
        numStreams += 2;
    } else {
        streams[numStreams + 1].data = skin->vertexBoneWeights;
        streams[numStreams + 1].kind = 9;
        streams[numStreams + 1].dependencies[0] = -1;
        numStreams += 2;
    }

    if (geometry->flags & rpGEOMETRYNORMALS) {
        streams[0].dependencies[0] = 1;
        streams[0].dependencies[1] = numStreams - 2;
        streams[0].dependencies[2] = numStreams - 1;
        streams[0].dependencies[3] = -1;
        streams[1].dependencies[0] = 0;
        streams[1].dependencies[1] = numStreams - 2;
        streams[1].dependencies[2] = numStreams - 1;
        streams[1].dependencies[3] = -1;
        streams[numStreams - 2].dependencies[0] = 0;
        streams[numStreams - 2].dependencies[1] = 1;
        streams[numStreams - 2].dependencies[2] = numStreams - 1;
        streams[numStreams - 2].dependencies[3] = -1;
        streams[numStreams - 1].dependencies[0] = 0;
        streams[numStreams - 1].dependencies[1] = 1;
        streams[numStreams - 1].dependencies[2] = numStreams - 2;
        streams[numStreams - 1].dependencies[3] = -1;
    } else {
        streams[0].dependencies[0] = numStreams - 2;
        streams[0].dependencies[1] = numStreams - 1;
        streams[0].dependencies[2] = -1;
        streams[numStreams - 2].dependencies[0] = 0;
        streams[numStreams - 2].dependencies[1] = numStreams - 1;
        streams[numStreams - 2].dependencies[2] = -1;
        streams[numStreams - 1].dependencies[0] = 0;
        streams[numStreams - 1].dependencies[1] = numStreams - 2;
        streams[numStreams - 1].dependencies[2] = -1;
    }

    builder = fn_80466194(streams, numStreams, geometry->numVertices);
    if (builder == NULL) {
        return FALSE;
    }
    *builtStreams = fn_80465ED4(builder, streams, numStreams, geometry->numVertices);
    if (*builtStreams == NULL) {
        RwFree(builder);
        return FALSE;
    }

    /* each mesh's indices, once per stream that is not skinned */
    numMeshes = geometry->mesh->numMeshes;
    numSkinned = numStreams - 2;
    meshIndices = RwMalloc(numSkinned * (numMeshes * sizeof(RxVertexIndex *)), rwMEMHINT_SKINTEMP);
    if (meshIndices == NULL) {
        RwFree(*builtStreams);
        RwFree(builder);
        return FALSE;
    }
    memset(meshIndices, 0, numSkinned * (numMeshes * sizeof(RxVertexIndex *)));
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        for (j = 0; j < numStreams - 2; j++) {
            meshIndices[i * (numStreams - 2) + j] = mesh->indices;
        }
    }

    *meshIndexLists = RwMalloc(numMeshes * sizeof(RxVertexIndex **), rwMEMHINT_SKIN);
    if (*meshIndexLists == NULL) {
        RwFree(meshIndices);
        RwFree(*builtStreams);
        RwFree(builder);
        return FALSE;
    }
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        (*meshIndexLists)[i] = fn_80465CDC(builder, &meshIndices[i * (numStreams - 2)],
                                           numStreams - 2, mesh->numIndices);
        if ((*meshIndexLists)[i] == NULL) {
            /* (sic) frees the earlier display lists but the first */
            while (--i > 0) {
                RwFree((*meshIndexLists)[i]);
            }
            return FALSE;
        }
    }

    RwFree(meshIndices);
    RwFree(builder);
    return TRUE;
}

/* The vertex descriptors of a skinned geometry */
RpGameCubeVertexDescBuilder lbl_805E1560;

/* The GX index type for a stream with count distinct values */
#define SKININDEXTYPE(count) ((count) >= 255 ? GX_INDEX16 : GX_INDEX8)

/* Describes a skinned geometry's vertices to GX: a matrix index for rigid skins, then indexed
 * positions, normals (or normals, binormals and tangents), prelit colors and texture coordinates
 * in the geometry's vertex format */
RpGameCubeVertexDescBuilder *fn_8044F4D0(RpGeometry *geometry, RpGameCubeBuiltStream *counts) {
    RpSkin *skin = fn_8044CCAC(geometry);
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E1560);
    fn_8046FC48(&lbl_805E1560, 0);
    if (skin->maxNumWeightsForVertex == 1) {
        fn_8046FF34(&lbl_805E1560, GX_VA_PNMTXIDX, GX_DIRECT);
    }
    fn_8046FC50(&lbl_805E1560, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    numAttributes = 0;
    fn_8046FF34(&lbl_805E1560, GX_VA_POS, SKININDEXTYPE(counts[numAttributes].count));
    numAttributes++;

    if (geometry->flags & rpGEOMETRYNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E1560, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E1560, GX_VA_NBT, SKININDEXTYPE(counts[numAttributes].count));
        } else {
            fn_8046FC50(&lbl_805E1560, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E1560, GX_VA_NRM, SKININDEXTYPE(counts[numAttributes].count));
        }
        numAttributes++;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        fn_8046FC50(&lbl_805E1560, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E1560, GX_VA_CLR0, SKININDEXTYPE(counts[numAttributes].count));
        numAttributes++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        RpGameCubeBuiltStream *count = &counts[numAttributes];

        for (i = 0; i < geometry->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E1560, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E1560, GX_VA_TEX0 + i, SKININDEXTYPE(count->count));
            count++;
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E1560, numAttributes);
    return &lbl_805E1560;
}

/* The GX matrix slot (3 rows each) of each index of a rigid skin's mesh: the position of the
 * vertex's bone among the mesh's split bones (at most 10), or among the skin's used bones */
RwUInt16 *fn_8044F740(RpSkin *skin, RxVertexIndex *indices, RwUInt32 *vertexBoneIndices,
                      RwInt32 numIndices, RwInt32 meshIndex) {
    RwUInt16 *slots;
    RwInt32 i;
    RwUInt32 j;

    if (skin->skinSplitData.numMeshes != 0) {
        RwUInt32 firstRun = skin->skinSplitData.meshRLECount[meshIndex].start & 0x7F;
        RwUInt8 bones[10];
        RwUInt8 *bone = bones;
        RwInt32 numRuns = skin->skinSplitData.meshRLECount[meshIndex].length;
        RwInt32 run;

        for (run = 0; run < numRuns; run++) {
            RwUInt8 start = (skin->skinSplitData.meshRLE + run + firstRun)->start;
            RwUInt8 length = (skin->skinSplitData.meshRLE + run + firstRun)->length;
            RwUInt8 k;

            for (k = 0; k < length; k++) {
                *bone++ = start + k;
            }
        }

        slots = RwMalloc(numIndices * sizeof(RwUInt16), rwMEMHINT_SKIN);
        for (i = 0; i < numIndices; i++) {
            for (j = 0; j < 10; j++) {
                if (bones[j] == (RwUInt8)vertexBoneIndices[indices[i]]) {
                    slots[i] = j * 3;
                    break;
                }
            }
        }
    } else {
        slots = RwMalloc(numIndices * sizeof(RwUInt16), rwMEMHINT_SKIN);
        for (i = 0; i < numIndices; i++) {
            for (j = 0; j < skin->numBoneIds; j++) {
                if (skin->boneIds[j] == (RwUInt8)vertexBoneIndices[indices[i]]) {
                    slots[i] = j * 3;
                    break;
                }
            }
        }
    }
    return slots;
}

/* The number of a skinned geometry's vertex streams before the bone indices: positions, normals,
 * prelit colors and texture coordinates */
static inline RwInt32 SkinNumVertexStreams(RpGeometry *geometry) {
    RwInt32 numStreams = 1;
    RwInt32 i;

    if (geometry->flags & rpGEOMETRYNORMALS) {
        numStreams = 2;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        numStreams++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < geometry->numTexCoordSets; i++) {
            numStreams++;
        }
    }
    return numStreams;
}

/* Sets a mesh's attribute indices from its per-stream index lists, with the GX matrix slots of a
 * rigid skin */
static inline void SkinSetMeshIndices(RpGameCubeIndexSource *indexSource, RpGeometry *geometry,
                                      RxVertexIndex **lists, RwUInt16 *slots) {
    RwUInt8 numTexCoordSets = geometry->numTexCoordSets;
    RwUInt32 flags = geometry->flags;
    RwInt32 list;
    RwUInt32 i;

    if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
        fn_80462E1C();
    }
    if (slots != NULL) {
        indexSource->indices[GX_VA_PNMTXIDX] = slots;
    }
    indexSource->indices[GX_VA_POS] = lists[0];
    list = 1;
    if (flags & rpGEOMETRYNORMALS) {
        indexSource->indices[GX_VA_NRM] = lists[list];
        list = 2;
    }
    if (flags & rpGEOMETRYPRELIT) {
        indexSource->indices[GX_VA_CLR0] = lists[list];
        list++;
    }
    if (flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < numTexCoordSets; i++) {
            indexSource->indices[GX_VA_TEX0 + i] = lists[list];
            list++;
        }
    }
}

/* Instances a skinned geometry whose vertices are rebuilt as distinct values per stream: the
 * vertex data, a display list per mesh (triangle strips when the geometry has them) indexing each
 * stream, and for skins with several weights the built byte indices and weights */
RwResEntry *fn_8044F9E4(RpGeometry *geometry, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpGameCubeBuiltStream *builtStreams;
    RxVertexIndex ***meshIndexLists;
    RwInt32 numStrips;
    RwUInt32 numStripIndices;
    RpGameCubeVertexDescBuilder *builder;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RpSkin *skin;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 size;
    RwUInt32 entrySize;
    RwInt32 primitive;
    RwUInt32 i;
    RwInt32 j;

    fn_8044EEAC(geometry, &builtStreams, &meshIndexLists);
    builder = fn_8044F4D0(geometry, builtStreams);
    {
        RwUInt8 numTexCoordSets = geometry->numTexCoordSets;
        RwUInt32 flags = geometry->flags;
        RwInt32 stream;

        if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
            fn_80462E1C();
        }
        vertexSource.numElements[GX_VA_POS] = builtStreams[0].count;
        vertexSource.data[GX_VA_POS] = builtStreams[0].data;
        stream = 1;
        if (flags & rpGEOMETRYNORMALS) {
            vertexSource.numElements[GX_VA_NRM] = builtStreams[stream].count;
            vertexSource.data[GX_VA_NRM] = builtStreams[stream].data;
            stream = 2;
        }
        if (flags & rpGEOMETRYPRELIT) {
            vertexSource.numElements[GX_VA_CLR0] = builtStreams[stream].count;
            vertexSource.data[GX_VA_CLR0] = builtStreams[stream].data;
            stream++;
        }
        if (flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
            for (i = 0; i < numTexCoordSets; i++) {
                vertexSource.numElements[GX_VA_TEX0 + i] = builtStreams[stream].count;
                vertexSource.data[GX_VA_TEX0 + i] = builtStreams[stream].data;
                stream++;
            }
        }
    }

    /* the vertex data header, the display list records, then the display lists */
    vertexDataSize = fn_8046EE8C(builder);
    size = vertexDataSize + geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList) + 31;
    primitive = GX_TRIANGLES;
    if (geometry->flags & rpGEOMETRYTRISTRIP) {
        primitive = GX_TRIANGLESTRIP;
    }
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        if (geometry->flags & rpGEOMETRYTRISTRIP) {
            RwInt32 meshStrips;
            RwUInt32 meshStripIndices;

            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &meshStrips, &meshStripIndices, 1);
            size += fn_8046B094(builder, meshStrips, meshStripIndices);
        } else {
            size += fn_8046B094(builder, 1, mesh->numIndices);
        }
    }
    size += fn_8046EEA0(builder, &vertexSource);

    entrySize = size;
    repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_SKIN);
    repEntry->link.next = NULL;
    repEntry->link.prev = NULL;
    repEntry->owner = owner;
    repEntry->size = size;
    repEntry->ownerRef = ownerRef;
    repEntry->destroyNotify = fn_8046288C;
    *ownerRef = repEntry;

    instance = (RpGameCubeInstanceData *)(repEntry + 1);
    memset(instance, 0, size);
    instance->syncToken = lbl_805F1678;
    instance->serialNum = geometry->mesh->serialNum;
    instance->flags = 0;
    if (geometry->flags & rpGEOMETRYPRELIT) {
        RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);

        /* whether the colors need alpha */
        if (format == NULL) {
            instance->flags &= ~1;
            for (j = 0; j < geometry->numVertices; j++) {
                if (geometry->preLitLum[j].alpha < 255) {
                    instance->flags |= 1;
                    break;
                }
            }
        } else if (format->colorType > GX_RGBX8) {
            instance->flags |= 1;
        } else {
            instance->flags &= ~1;
        }
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList +
                                   geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList) + 31) &
                                  ~31);
    skin = fn_8044CCAC(geometry);
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;
        RwUInt16 *slots = NULL;
        RwUInt32 displayListSize;

        if (geometry->flags & rpGEOMETRYTRISTRIP) {
            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &numStrips, &numStripIndices, 1);
        } else {
            numStripIndices = mesh->numIndices;
            numStrips = 1;
        }
        displayListSize = fn_8046B094(builder, numStrips, numStripIndices);
        fn_8046B0E0(displayList, i, displayListSize, displayListData);

        if (skin->maxNumWeightsForVertex > 1) {
            SkinSetMeshIndices(&indexSource, geometry, meshIndexLists[i], NULL);
        } else {
            slots =
                fn_8044F740(skin, meshIndexLists[i][0],
                            builtStreams[SkinNumVertexStreams(geometry)].data, mesh->numIndices, i);
            SkinSetMeshIndices(&indexSource, geometry, meshIndexLists[i], slots);
        }
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices,
                    geometry->flags & rpGEOMETRYTRISTRIP, fn_8046ACBC(builder), 1, primitive, 0);
        if (slots != NULL) {
            RwFree(slots);
        }
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 0, 0);
    if (skin->maxNumWeightsForVertex > 1) {
        RpGameCubeBuiltStream *indices = &builtStreams[SkinNumVertexStreams(geometry)];

        memcpy(skin->indices, indices[0].data, skin->maxNumWeightsForVertex * indices[0].count);
        memcpy(skin->weights, indices[1].data, skin->maxNumWeightsForVertex * indices[1].count);
    }
    geometry->numVertices = builtStreams[0].count;
    DCFlushRange(instance, entrySize);
    GXInvalidateVtxCache();

    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RwFree(meshIndexLists[i]);
    }
    RwFree(meshIndexLists);
    RwFree(builtStreams);
    return repEntry;
}

/* Describes a rigid skinned geometry's vertices to GX: a matrix index, then positions, normals
 * (or normals, binormals and tangents), prelit colors and texture coordinates indexed by vertex */
RpGameCubeVertexDescBuilder *fn_80450448(RpGeometry *geometry) {
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    RwUInt32 numVertices = geometry->numVertices; /* compared unsigned */
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E1560);
    fn_8046FC48(&lbl_805E1560, 0);
    fn_8046FF34(&lbl_805E1560, GX_VA_PNMTXIDX, GX_DIRECT);
    fn_8046FC50(&lbl_805E1560, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    fn_8046FF34(&lbl_805E1560, GX_VA_POS, SKININDEXTYPE(numVertices));
    numAttributes = 1;

    if (geometry->flags & rpGEOMETRYNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E1560, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E1560, GX_VA_NBT, SKININDEXTYPE(numVertices));
        } else {
            fn_8046FC50(&lbl_805E1560, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E1560, GX_VA_NRM, SKININDEXTYPE(numVertices));
        }
        numAttributes = 2;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        fn_8046FC50(&lbl_805E1560, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E1560, GX_VA_CLR0, SKININDEXTYPE(numVertices));
        numAttributes++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < geometry->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E1560, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E1560, GX_VA_TEX0 + i, SKININDEXTYPE(numVertices));
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E1560, numAttributes);
    return &lbl_805E1560;
}

/* The GX matrix slot (3 rows each) of each index of a rigid skin's mesh, as fn_8044F740 */
RwUInt16 *fn_8045066C(RpSkin *skin, RpMesh *mesh, RwInt32 meshIndex) {
    RwUInt16 *slots;
    RwUInt32 i;
    RwUInt32 j;

    if (skin->skinSplitData.numMeshes != 0) {
        RwUInt32 firstRun = skin->skinSplitData.meshRLECount[meshIndex].start & 0x7F;
        RwUInt8 bones[10];
        RwUInt8 *bone = bones;
        RwUInt32 numRuns = skin->skinSplitData.meshRLECount[meshIndex].length;
        RwUInt32 run;

        for (run = 0; run < numRuns; run++) {
            RwUInt8 start = (skin->skinSplitData.meshRLE + run + firstRun)->start;
            RwUInt8 length = (skin->skinSplitData.meshRLE + run + firstRun)->length;
            RwUInt8 k;

            for (k = 0; k < length; k++) {
                *bone++ = start + k;
            }
        }

        slots = RwMalloc(mesh->numIndices * sizeof(RwUInt16), rwMEMHINT_SKIN);
        for (i = 0; i < mesh->numIndices; i++) {
            for (j = 0; j < 10; j++) {
                if (bones[j] == (RwUInt8)skin->vertexBoneIndices[mesh->indices[i]]) {
                    slots[i] = j * 3;
                    break;
                }
            }
        }
    } else {
        slots = RwMalloc(mesh->numIndices * sizeof(RwUInt16), rwMEMHINT_SKIN);
        for (i = 0; i < mesh->numIndices; i++) {
            for (j = 0; j < skin->numBoneIds; j++) {
                if (skin->boneIds[j] == (RwUInt8)skin->vertexBoneIndices[mesh->indices[i]]) {
                    slots[i] = j * 3;
                    break;
                }
            }
        }
    }
    return slots;
}

/* Instances a rigid skinned geometry: indexed vertex data and a display list per mesh whose
 * vertices carry their bone's GX matrix slot */
RwResEntry *fn_80450938(RpGeometry *geometry, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpSkin *skin = fn_8044CCAC(geometry);
    RpGameCubeVertexDescBuilder *builder = fn_80450448(geometry);
    RpMorphTarget *morphTarget;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 size;
    RwUInt32 entrySize;
    RwInt32 primitive;
    RwUInt32 i;
    RwInt32 j;

    if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
        fn_80462E1C();
    }

    morphTarget = geometry->morphTarget;
    vertexSource.numElements[GX_VA_POS] = geometry->numVertices;
    vertexSource.data[GX_VA_POS] = morphTarget->verts;
    if (geometry->flags & rpGEOMETRYNORMALS) {
        vertexSource.numElements[GX_VA_NRM] = geometry->numVertices;
        vertexSource.data[GX_VA_NRM] = morphTarget->normals;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        vertexSource.numElements[GX_VA_CLR0] = geometry->numVertices;
        vertexSource.data[GX_VA_CLR0] = geometry->preLitLum;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (j = 0; j < geometry->numTexCoordSets; j++) {
            vertexSource.numElements[GX_VA_TEX0 + j] = geometry->numVertices;
            vertexSource.data[GX_VA_TEX0 + j] = geometry->texCoords[j];
        }
    }

    /* the vertex data header, the display list records, then the display lists */
    vertexDataSize = fn_8046EE8C(builder);
    size = vertexDataSize + geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList) + 31;
    primitive = GX_TRIANGLES;
    if (geometry->flags & rpGEOMETRYTRISTRIP) {
        primitive = GX_TRIANGLESTRIP;
    }
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        size += fn_8046B094(builder, 1, ((RpMesh *)(geometry->mesh + 1))[i].numIndices);
    }
    size += fn_8046EEA0(builder, &vertexSource);

    if (geometry->flags & 0x02000000) {
        repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_SKIN);
        repEntry->link.next = NULL;
        repEntry->link.prev = NULL;
        repEntry->owner = owner;
        repEntry->size = size;
        repEntry->ownerRef = ownerRef;
        repEntry->destroyNotify = fn_8046288C;
        *ownerRef = repEntry;
    } else {
        repEntry = fn_80480878(owner, ownerRef, size, fn_8046288C);
    }

    instance = (RpGameCubeInstanceData *)(repEntry + 1);
    entrySize = size;
    memset(instance, 0, size);
    instance->syncToken = lbl_805F1678;
    instance->serialNum = geometry->mesh->serialNum;
    instance->flags = 0;
    if (geometry->flags & rpGEOMETRYPRELIT) {
        RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);

        /* whether the colors need alpha */
        if (format == NULL) {
            instance->flags &= ~1;
            for (j = 0; j < geometry->numVertices; j++) {
                if (geometry->preLitLum[j].alpha < 255) {
                    instance->flags |= 1;
                    break;
                }
            }
        } else if (format->colorType > GX_RGBX8) {
            instance->flags |= 1;
        } else {
            instance->flags &= ~1;
        }
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList +
                                   geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList) + 31) &
                                  ~31);
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;
        RwUInt32 displayListSize = fn_8046B094(builder, 1, mesh->numIndices);
        RwUInt16 *slots;
        RxVertexIndex *indices;

        fn_8046B0E0(displayList, i, displayListSize, displayListData);
        slots = fn_8045066C(skin, mesh, i);
        indices = mesh->indices;
        if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
            fn_80462E1C();
        }
        indexSource.indices[GX_VA_PNMTXIDX] = slots;
        indexSource.indices[GX_VA_POS] = indices;
        if (geometry->flags & rpGEOMETRYNORMALS) {
            indexSource.indices[GX_VA_NRM] = indices;
        }
        if (geometry->flags & rpGEOMETRYPRELIT) {
            indexSource.indices[GX_VA_CLR0] = indices;
        }
        if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
            for (j = 0; j < geometry->numTexCoordSets; j++) {
                indexSource.indices[GX_VA_TEX0 + j] = indices;
            }
        }
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices, 0, fn_8046ACBC(builder),
                    1, primitive, 0);
        RwFree(slots);
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 0, 0);
    DCFlushRange(instance, entrySize);
    GXInvalidateVtxCache();
    return repEntry;
}
