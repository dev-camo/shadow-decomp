/* RenderWare skinning plugin (plugin ID 0x116): skins and their split data, the engine, atomic and
 * geometry plugin data, the skin stream format, and the plugin attachment. The original file name
 * is unknown. */
#include "rw/rpskin.h"
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000
#define rwMEMHINT_SKIN (rwMEMHINTDUR_EVENT | rpID_SKINPLUGIN)

#define rpID_MATFXPLUGIN 0x120
#define rpID_TOONPLUGIN 0x12E /* (inferred) */

SkinGlobals lbl_8056D708 = {0};
RwFreeList lbl_805E1538;   /* the skin free list */
RwInt32 lbl_805EDD80 = 20; /* skins per free list block */
RwInt32 lbl_805EDD84 = 1;  /* free list blocks allocated up front */

/* Frees a skin's split data and clears it */
#define SKINSPLITDATADESTROY(skin)                                                                 \
    do {                                                                                           \
        if ((skin)->skinSplitData.matrixRemapIndices != NULL) {                                    \
            RwFree((skin)->skinSplitData.matrixRemapIndices);                                      \
        }                                                                                          \
        (skin)->skinSplitData.boneLimit = 0;                                                       \
        (skin)->skinSplitData.numMeshes = 0;                                                       \
        (skin)->skinSplitData.numRLE = 0;                                                          \
        (skin)->skinSplitData.matrixRemapIndices = NULL;                                           \
        (skin)->skinSplitData.meshRLECount = NULL;                                                 \
        (skin)->skinSplitData.meshRLE = NULL;                                                      \
    } while (0)

/* Destroys the split data of a skin */
RwBool fn_8044BAB4(RpSkin *skin) {
    SKINSPLITDATADESTROY(skin);
    return TRUE;
}

/* Writes the split data of a skin */
RwStream *fn_8044BB18(RwStream *stream, RpSkin *skin) {
    SkinSplitData *splitData = &skin->skinSplitData;

    if (fn_8047D458(stream, (RwInt32 *)&splitData->boneLimit, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)&splitData->numMeshes, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)&splitData->numRLE, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (skin->skinSplitData.numMeshes != 0) {
        if (fn_80480ED0(stream, splitData->matrixRemapIndices,
                        skin->numBones + splitData->numMeshes * 2 + splitData->numRLE * 2) ==
            NULL) {
            return NULL;
        }
    }
    return stream;
}

/* Allocates the split data of a skin (inferred) */
static inline RpSkin *SkinSplitDataCreate(RpSkin *skin, RwUInt32 boneLimit, RwUInt32 numMatrices,
                                          RwUInt32 numMeshes, RwUInt32 numRLE) {
    SkinSplitData *splitData = &skin->skinSplitData;
    RwUInt32 size;

    SKINSPLITDATADESTROY(skin);
    size = numMatrices + numMeshes * sizeof(RwUInt16) + numRLE * sizeof(RwUInt16);

    splitData->matrixRemapIndices = RwMalloc(size, rwMEMHINT_SKIN);
    if (splitData->matrixRemapIndices == NULL) {
        return NULL;
    }
    memset(splitData->matrixRemapIndices, 0, size);
    splitData->boneLimit = boneLimit;
    splitData->numMeshes = numMeshes;
    splitData->numRLE = numRLE;
    splitData->meshRLECount = (RwUInt16 *)(splitData->matrixRemapIndices + numMatrices);
    splitData->meshRLE = splitData->meshRLECount + numMeshes;
    return skin;
}

/* Reads the split data of a skin */
RwStream *fn_8044BBFC(RwStream *stream, RpSkin *skin) {
    SkinSplitData *splitData = &skin->skinSplitData;
    RwInt32 numMeshes;
    RwInt32 numRLE;
    RwInt32 boneLimit;

    if (fn_8047D868(stream, &boneLimit, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &numMeshes, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &numRLE, sizeof(RwInt32)) == NULL) {
        return NULL;
    }

    if (numMeshes > 0) {
        if (SkinSplitDataCreate(skin, boneLimit, skin->numBones, numMeshes, numRLE) == NULL) {
            return NULL;
        }
        if (fn_80480D4C(stream, splitData->matrixRemapIndices,
                        skin->numBones + splitData->numMeshes * 2 + splitData->numRLE * 2) == 0) {
            /* (sic) frees the split data itself rather than its block */
            RwFree(splitData);
            return NULL;
        }
    }
    return stream;
}

/* The size of a skin's split data in the stream */
RwInt32 fn_8044BDBC(RpSkin *skin) {
    RwInt32 size = 3 * sizeof(RwInt32);

    if (skin->skinSplitData.numMeshes != 0) {
        size = skin->numBones + skin->skinSplitData.numMeshes * 2 + skin->skinSplitData.numRLE * 2 +
               3 * sizeof(RwInt32);
    }
    return size;
}

/* Counts the most weights (up to four) that any vertex uses (inferred) */
static inline void SkinCountMaxWeights(RpSkin *skin, RwMatrixWeights *vertexWeights,
                                       RwUInt32 numVertices) {
    RwUInt32 *weight;
    RwUInt32 i;
    RwUInt32 j;

    skin->maxNumWeightsForVertex = 1;
    for (i = 0; i < numVertices; vertexWeights++, i++) {
        j = skin->maxNumWeightsForVertex;
        for (weight = (RwUInt32 *)&vertexWeights->w0 + j; j < 4; weight++, j++) {
            if (*weight == 0) {
                break;
            }
            skin->maxNumWeightsForVertex++;
            if (skin->maxNumWeightsForVertex == 4) {
                return;
            }
        }
    }
}

/* Creates a skin. Without numBoneIds the bones that the vertices use are collected; without
 * maxNumWeights the most weights any vertex uses (up to four) is counted. */
RpSkin *fn_8044BDF0(RwUInt32 numVertices, RwUInt32 numBones, RwUInt32 numBoneIds,
                    RwUInt32 maxNumWeights, RwMatrixWeights *vertexWeights, RwUInt32 *vertexIndices,
                    RwMatrix *inverseMatrices) {
    RpSkin *skin;
    RwUInt8 boneIds[256];
    RwUInt32 size;
    RwBool result;
    RwUInt32 i;
    RwUInt32 j;
    RwUInt32 k;

    skin = RwFreeListAlloc(lbl_8056D708.freeList, rwMEMHINT_SKIN);
    memset(skin, 0, sizeof(RpSkin));

    if (maxNumWeights == 0) {
        SkinCountMaxWeights(skin, vertexWeights, numVertices);
    }

    if (numBoneIds == 0) {
        RwMatrixWeights *weightsOfVertex = vertexWeights;
        RwUInt32 *indexOfVertex = vertexIndices;

        numBoneIds = 0;

        for (i = 0; i < numVertices; weightsOfVertex++, indexOfVertex++, i++) {
            RwUInt32 *weight = (RwUInt32 *)&weightsOfVertex->w0;

            for (j = 0; j < skin->maxNumWeightsForVertex; j++) {
                if (weight[j] != 0) {
                    RwUInt8 bone = *indexOfVertex >> (j * 8);
                    RwBool isNew = TRUE;

                    for (k = 0; k < numBoneIds; k++) {
                        if (bone == boneIds[k]) {
                            isNew = FALSE;
                            break;
                        }
                    }
                    if (isNew) {
                        boneIds[numBoneIds] = bone;
                        numBoneIds++;
                    }
                }
            }
        }
    }

    /* the bone ids, then 16-byte aligned matrices, then the indices and weights */
    size = numBones * sizeof(RwMatrix);
    size += numBoneIds + 15;
    size += numVertices * (sizeof(RwUInt32) + sizeof(RwMatrixWeights));
    skin->unaligned = RwMalloc(size, rwMEMHINT_SKIN);
    if (skin->unaligned == NULL) {
        result = FALSE;
    } else {
        memset(skin->unaligned, 0, size);
        skin->numBones = numBones;
        skin->numBoneIds = numBoneIds;
        skin->boneIds = skin->unaligned;
        skin->skinToBoneMatrices = (RwMatrix *)(((RwUInt32)skin->boneIds + numBoneIds + 15) & ~15);
        skin->vertexBoneIndices = (RwUInt32 *)(skin->skinToBoneMatrices + numBones);
        skin->vertexBoneWeights = (RwMatrixWeights *)(skin->vertexBoneIndices + numVertices);

        if (boneIds != NULL && numBoneIds != 0) {
            memcpy(skin->boneIds, boneIds, numBoneIds);
        }
        if (inverseMatrices != NULL) {
            i = skin->numBones;
            while (i-- != 0) {
                skin->skinToBoneMatrices[i] = inverseMatrices[i];
            }
        }
        if (vertexIndices != NULL) {
            memcpy(skin->vertexBoneIndices, vertexIndices, numVertices * sizeof(RwUInt32));
        }
        if (vertexWeights != NULL) {
            memcpy(skin->vertexBoneWeights, vertexWeights, numVertices * sizeof(RwMatrixWeights));
        }
        result = TRUE;
    }

    if (!result) {
        RwFreeListFree(lbl_8056D708.freeList, skin);
        return NULL;
    }
    return skin;
}

/* Engine plugin constructor: on first use, opens the pipelines for the plugins present and creates
 * the skin free list and the matrix cache */
void *fn_8044C174(void *instance, RwInt32 offset, RwInt32 size) {
    RwUInt32 flags;

    if (lbl_8056D708.numInstances == 0) {
        flags = 0x1;
        if (fn_80485DB8(rpID_MATFXPLUGIN) != -1) {
            flags |= 0x2;
        }
        if (fn_80485DB8(rpID_TOONPLUGIN) != -1) {
            flags |= 0x4;
        }
        fn_80451D60(flags);

        lbl_8056D708.freeList = fn_8047FC04(sizeof(RpSkin), lbl_805EDD80, 4, lbl_805EDD84,
                                            &lbl_805E1538, rwMEMHINTDUR_GLOBAL | rpID_SKINPLUGIN);
        lbl_8056D708.matrixCacheUnaligned =
            RwMalloc(256 * sizeof(RwMatrix) + 15, rwMEMHINTDUR_GLOBAL | rpID_SKINPLUGIN);
        memset(lbl_8056D708.matrixCacheUnaligned, 0, 256 * sizeof(RwMatrix) + 15);
        lbl_8056D708.matrixCache =
            (RwMatrix *)(((RwUInt32)lbl_8056D708.matrixCacheUnaligned + 15) & ~15);
    }
    lbl_8056D708.numInstances++;
    return instance;
}

/* Engine plugin destructor: on last use, closes the pipelines and frees the free list and the
 * matrix cache */
void *fn_8044C280(void *instance, RwInt32 offset, RwInt32 size) {
    if (--lbl_8056D708.numInstances == 0) {
        fn_80451E0C();
        fn_8047FCDC(lbl_8056D708.freeList);
        lbl_8056D708.freeList = NULL;
        RwFree(lbl_8056D708.matrixCacheUnaligned);
        lbl_8056D708.matrixCacheUnaligned = NULL;
    }
    return instance;
}

/* Geometry plugin constructor */
void *fn_8044C30C(void *object, RwInt32 offset, RwInt32 size) {
    RPSKINGEOMETRYGETDATA(object) = NULL;
    return object;
}

/* Geometry plugin destructor */
void *fn_8044C324(void *object, RwInt32 offset, RwInt32 size) {
    RpSkin *skin = RPSKINGEOMETRYGETDATA(object);

    if (skin != NULL) {
        fn_8044E758(object);
        RPSKINGEOMETRYGETDATA(object) = fn_8044CD4C(skin);
    }
    return object;
}

/* Geometry plugin copy: skins are not copied */
void *fn_8044C38C(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {}

/* Atomic plugin constructor */
void *fn_8044C390(void *object, RwInt32 offset, RwInt32 size) {
    memset(RPSKINATOMICGETDATA(object), 0, sizeof(SkinAtomicData));
    return object;
}

/* Atomic plugin destructor */
void *fn_8044C3D8(void *object, RwInt32 offset, RwInt32 size) {
    if (RPSKINATOMICGETDATA(object)->hierarchy != NULL) {
        RPSKINATOMICGETDATA(object)->hierarchy = NULL;
    }
}

/* Atomic plugin copy: shares the hierarchy */
void *fn_8044C3FC(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    RPSKINATOMICGETDATA(dstObject)->hierarchy = RPSKINATOMICGETDATA(srcObject)->hierarchy;
}

/* Puts a loaded skinned atomic in the skin pipeline that suits its other plugins */
RwBool fn_8044C414(void *object, RwInt32 offset, RwInt32 size) {
    RwInt32 type = 1;
    RpAtomic *atomic = object;

    if (fn_80485DB8(rpID_MATFXPLUGIN) != -1 &&
        *RWPLUGINOFFSET(RwUInt8, atomic, fn_80458C90(rpID_MATFXPLUGIN)) != 0) {
        type = 2;
    }
    if (atomic->geometry != NULL && fn_8044CCAC(atomic->geometry) != NULL) {
        if (fn_80485DB8(rpID_MATFXPLUGIN) == -1 && type == 2) {
            type = 1;
        } else if (fn_80485DB8(rpID_TOONPLUGIN) == -1 && type == 3) {
            type = 1;
        }
        fn_80451E6C(atomic, type);
    }
    return TRUE;
}

/* Puts a loaded skinned atomic in the skin pipeline given by its rights */
RwBool fn_8044C4D4(void *object, RwInt32 offset, RwInt32 size, RwUInt32 extraData) {
    RwInt32 type = extraData;
    RpAtomic *atomic = object;

    if (atomic->geometry != NULL && fn_8044CCAC(atomic->geometry) != NULL) {
        if (fn_80485DB8(rpID_MATFXPLUGIN) == -1 && type == 2) {
            type = 1;
        } else if (fn_80485DB8(rpID_TOONPLUGIN) == -1 && type == 3) {
            type = 1;
        }
        fn_80451E6C(atomic, type);
    }
    return TRUE;
}

/* The size of a geometry's skin in the stream */
RwInt32 fn_8044C56C(const void *object, RwInt32 offset, RwInt32 size) {
    const RpGeometry *geometry = object;
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);
    RwInt32 streamSize = 0;

    if (skin != NULL) {
        if (!(geometry->flags & rpGEOMETRYNATIVE)) {
            streamSize = sizeof(RwUInt32);
            streamSize += skin->numBoneIds;
            streamSize += geometry->numVertices * sizeof(RwUInt32);
            streamSize += geometry->numVertices * sizeof(RwMatrixWeights);
            streamSize += skin->numBones * sizeof(RwMatrix);
            streamSize += fn_8044BDBC(skin);
        } else {
            streamSize = fn_8044E7D0((RpGeometry *)geometry);
        }
    }
    return streamSize;
}

/* Writes a geometry's skin: the bone counts, the used bones, the vertex indices and weights, the
 * matrices and the split data */
RwStream *fn_8044C5FC(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpGeometry *geometry = object;
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);
    RwInt32 numVertices;
    RwUInt32 counts;

    if (skin != NULL) {
        if (!(geometry->flags & rpGEOMETRYNATIVE)) {
            counts = ((skin->maxNumWeightsForVertex & 0xFF) << 16) |
                     ((skin->numBoneIds & 0xFF) << 8) | (skin->numBones & 0xFF);
            numVertices = geometry->numVertices;
            if (fn_8047D458(stream, (RwInt32 *)&counts, sizeof(RwUInt32)) == NULL) {
                return NULL;
            }
            if (fn_80480ED0(stream, skin->boneIds, skin->numBoneIds) == NULL) {
                return NULL;
            }
            if (fn_8047D458(stream, (RwInt32 *)skin->vertexBoneIndices,
                            numVertices * sizeof(RwUInt32)) == NULL) {
                return NULL;
            }
            if (fn_8047D350(stream, &skin->vertexBoneWeights->w0,
                            numVertices * sizeof(RwMatrixWeights)) == NULL) {
                return NULL;
            }
            if (fn_8047D350(stream, &skin->skinToBoneMatrices->right.x,
                            skin->numBones * sizeof(RwMatrix)) == NULL) {
                return NULL;
            }
            if (fn_8044BB18(stream, skin) == NULL) {
                return NULL;
            }
        } else if (fn_8044E89C(stream, (RpGeometry *)geometry) == NULL) {
            return NULL;
        }
    }
    return stream;
}

/* Reads a geometry's skin */
RwStream *fn_8044C754(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpGeometry *geometry = object;
    RwUInt32 counts;
    RwUInt32 maxNumWeights;
    RwUInt32 numBoneIds;
    RwInt32 numVertices;
    RpSkin *skin;

    if (!(geometry->flags & rpGEOMETRYNATIVE)) {
        if (fn_8047D868(stream, (RwInt32 *)&counts, sizeof(RwUInt32)) == NULL) {
            return NULL;
        }
        numVertices = geometry->numVertices;
        numBoneIds = (counts >> 8) & 0xFF;
        maxNumWeights = (counts >> 16) & 0xFF;
        skin = fn_8044BDF0(numVertices, counts & 0xFF, numBoneIds, maxNumWeights, NULL, NULL, NULL);
        if (skin == NULL) {
            return NULL;
        }
        if (numBoneIds != fn_80480D4C(stream, skin->boneIds, numBoneIds)) {
            return NULL;
        }
        if (fn_8047D868(stream, (RwInt32 *)skin->vertexBoneIndices,
                        numVertices * sizeof(RwUInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D784(stream, &skin->vertexBoneWeights->w0,
                        numVertices * sizeof(RwMatrixWeights)) == NULL) {
            return NULL;
        }
        skin->maxNumWeightsForVertex = maxNumWeights;
        if (fn_8047D784(stream, &skin->skinToBoneMatrices->right.x,
                        skin->numBones * sizeof(RwMatrix)) == NULL) {
            return NULL;
        }
        if (fn_8044BBFC(stream, skin) == NULL) {
            return NULL;
        }
        fn_8044CCC0(geometry, skin);
    } else if (fn_8044EAD0(stream, geometry) == NULL) {
        return NULL;
    }
    return stream;
}

/* Reads an atomic's skin chunk. Old files stored the skin with the atomic: it is read into the
 * geometry when that has no skin yet, otherwise the chunk is skipped. */
RwStream *fn_8044C8C0(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpSkin *skin;
    RpAtomic *atomic = object;
    RpGeometry *geometry = atomic->geometry;
    RwUInt32 numBones;
    RwUInt32 numVertices;
    RwUInt32 i;
    RwUInt32 j;
    RwUInt32 k;

    if (fn_8044CCAC(geometry) == NULL) {
        RwMatrixWeights *vertexWeights;
        RwUInt32 *vertexIndices;
        RwUInt8 *boneIds;

        if (fn_8047D868(stream, (RwInt32 *)&numBones, sizeof(RwUInt32)) == NULL) {
            return NULL;
        }
        numVertices = geometry->numVertices;
        skin = fn_8044BDF0(numVertices, numBones, numBones, 4, NULL, NULL, NULL);
        if (fn_804810FC(stream, sizeof(RwUInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, (RwInt32 *)skin->vertexBoneIndices,
                        numVertices * sizeof(RwUInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D784(stream, &skin->vertexBoneWeights->w0,
                        numVertices * sizeof(RwMatrixWeights)) == NULL) {
            return NULL;
        }
        for (i = 0; i < skin->numBones; i++) {
            /* each matrix follows a bone's flags, index and parent */
            if (fn_804810FC(stream, 12) == NULL) {
                return NULL;
            }
            if (fn_8047D784(stream, &skin->skinToBoneMatrices[i].right.x, sizeof(RwMatrix)) ==
                NULL) {
                return NULL;
            }
        }

        SkinCountMaxWeights(skin, skin->vertexBoneWeights, numVertices);

        /* collect the bones the vertices use */
        vertexWeights = skin->vertexBoneWeights;
        vertexIndices = skin->vertexBoneIndices;
        boneIds = skin->boneIds;
        skin->numBoneIds = 0;
        for (i = 0; i < numVertices; vertexWeights++, vertexIndices++, i++) {
            RwUInt32 *weights = (RwUInt32 *)&vertexWeights->w0;

            for (j = 0; j < skin->maxNumWeightsForVertex; j++) {
                if (weights[j] != 0) {
                    RwUInt8 bone = *vertexIndices >> (j * 8);
                    RwBool isNew = TRUE;

                    for (k = 0; k < skin->numBoneIds; k++) {
                        if (bone == boneIds[k]) {
                            isNew = FALSE;
                            break;
                        }
                    }
                    if (isNew) {
                        boneIds[skin->numBoneIds] = bone;
                        skin->numBoneIds++;
                    }
                }
            }
        }

        fn_8044CCC0(geometry, skin);
    } else if (fn_804810FC(stream, size) == NULL) {
        return NULL;
    }
    return stream;
}

/* Writes an atomic's skin chunk: skins are written with their geometry */
RwStream *fn_8044CB6C(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {}

/* The size of an atomic's skin chunk */
RwInt32 fn_8044CB70(const void *object, RwInt32 offset, RwInt32 size) {
    return fn_8044EE3C(object, offset, size);
}

/* Attaches the plugin to the engine, atomics and geometries */
RwBool fn_8044CB90(void) {
    lbl_8056D708.engineOffset = fn_80485D78(0, rpID_SKINPLUGIN, fn_8044C174, fn_8044C280);
    lbl_8056D708.atomicOffset =
        fn_80458B6C(sizeof(SkinAtomicData), rpID_SKINPLUGIN, fn_8044C390, fn_8044C3D8, fn_8044C3FC);
    fn_80458BEC(rpID_SKINPLUGIN, fn_8044C8C0, fn_8044CB6C, fn_8044CB70);
    fn_80458C28(rpID_SKINPLUGIN, fn_8044C414);
    fn_80458C5C(rpID_SKINPLUGIN, fn_8044C4D4);
    lbl_8056D708.geometryOffset =
        fn_8045994C(sizeof(RpSkin *), rpID_SKINPLUGIN, fn_8044C30C, fn_8044C324, fn_8044C38C);
    fn_8045998C(rpID_SKINPLUGIN, fn_8044C754, fn_8044C5FC, fn_8044C56C);
    return TRUE;
}

/* Sets the hierarchy that animates a skinned atomic */
RpAtomic *fn_8044CC98(RpAtomic *atomic, void *hierarchy) {
    RPSKINATOMICGETDATA(atomic)->hierarchy = hierarchy;
}

/* The skin of a geometry */
RpSkin *fn_8044CCAC(RpGeometry *geometry) {
    return RPSKINGEOMETRYGETDATA(geometry);
}

/* Sets the skin of a geometry, releasing the previous one's instance data and instancing the new
 * one */
RpGeometry *fn_8044CCC0(RpGeometry *geometry, RpSkin *skin) {
    RpSkin *oldSkin = RPSKINGEOMETRYGETDATA(geometry);

    if (skin != oldSkin) {
        if (oldSkin != NULL) {
            fn_8044E758(geometry);
        }
        RPSKINGEOMETRYGETDATA(geometry) = skin;
        if (skin != NULL && fn_8044E6BC(geometry) == NULL) {
            return NULL;
        }
    }
    return geometry;
}

/* Destroys a skin */
RpSkin *fn_8044CD4C(RpSkin *skin) {
    if (skin->unaligned != NULL) {
        RwFree(skin->unaligned);
    }
    fn_8044BAB4(skin);
    RwFreeListFree(lbl_8056D708.freeList, skin);
    return NULL;
}

/* The bone indices of a skin's vertices */
RwUInt32 *fn_8044CDBC(RpSkin *skin) {
    return skin->vertexBoneIndices;
}
