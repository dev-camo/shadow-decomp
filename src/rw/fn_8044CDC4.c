/* RenderWare skinning plugin (plugin ID 0x116), GameCube: the skin pipeline's instance, reinstance
 * and render callbacks. Skinned positions and normals are instanced into vertex buffers that are
 * double buffered against the GPU, the bone matrices are updated from the hierarchy, and the skin
 * is computed by the hand-written skinning kernels at 0x80450D6C. The original file name is
 * unknown. */
#include "rw/rpskin.h"
#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <dolphin/os.h>

#define rpHANIMHIERARCHYLOCALSPACEMATRICES 0x4000

/* GameCube skin pipeline types (inferred) */
#define SKINPIPELINETOON 3

/* the instancing of the skin's geometry (inferred) */
RwResEntry *fn_8044F9E4(RpGeometry *geometry, void *owner, RwResEntry **ownerRef);
RwResEntry *fn_80450938(RpGeometry *geometry, void *owner, RwResEntry **ownerRef);

/* matrices and vectors */
RwMatrix *fn_8047EBB8(RwMatrix *out, const RwMatrix *a, const RwMatrix *b); /* out = a * b */
RwMatrix *fn_8047F0B0(RwMatrix *out, const RwMatrix *in);                   /* inverts */

/* Destroy callback of a skinned geometry's instance data, run before its resource entry is freed:
 * frees the separate entry the skinned vertices are computed into, restores the borrowed bind-pose
 * buffers to the vertex descriptors and clears the skin's or atomic's pointers to them */
void fn_8044CDC4(RwResEntry *repEntry) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(repEntry + 1);
    void *owner;
    RwUInt32 flags;

    RPGAMECUBEWAITDRAWSYNC(instance->syncToken);
    if (instance->vertexDescriptors[0].data != NULL) {
        fn_8048071C(SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data));
    }

    owner = repEntry->owner;
    /* for an atomic owner this reads its frame link (offset 8), not geometry flags */
    flags = ((RpGeometry *)owner)->flags;
    if (((RwObject *)owner)->type == rpATOMIC) {
        SkinAtomicData *atomicData = RPSKINATOMICGETDATA(owner);

        instance->vertexDescriptors[0].data = atomicData->positions;
        if (flags & rpGEOMETRYNORMALS) {
            instance->vertexDescriptors[1].data = atomicData->normals;
        }
        atomicData->positions = NULL;
        atomicData->normals = NULL;
    } else {
        RpSkin *skin = RPSKINGEOMETRYGETDATA(owner);

        instance->vertexDescriptors[0].data = skin->positions;
        if (flags & rpGEOMETRYNORMALS) {
            instance->vertexDescriptors[1].data = skin->normals;
        }
        skin->positions = NULL;
        skin->normals = NULL;
    }
}

/* Destroys an instanced vertex buffer once the GPU is done with it */
void fn_8044CEE0(RwResEntry *repEntry) {
    SkinVertexBuffer *buffer = (SkinVertexBuffer *)(repEntry + 1);

    RPGAMECUBEWAITDRAWSYNC(buffer->syncToken);
}

/* Prepares the vertex buffers that the skin is computed into: the bind-pose buffers move to the
 * owner the first time, a buffer the GPU is still reading is replaced, and a new buffer is
 * allocated as a resource entry when there is none */
RwBool fn_8044CF68(RpSkin *skin, RpAtomic *atomic, RpGameCubeInstanceData *instance,
                   RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry = atomic->geometry;
    void *owner;
    RwResEntry *buffer;
    RwResEntry *ownerEntry;
    RwUInt32 size;

    RwResourcesUseResEntry(renderMesh->repEntry);
    renderMesh->repEntry->destroyNotify = fn_8044CDC4;

    owner = renderMesh->repEntry->owner;
    if (((RwObject *)owner)->type == rpATOMIC) {
        SkinAtomicData *atomicData = RPSKINATOMICGETDATA(owner);

        if (atomicData->positions == NULL) {
            atomicData->positions = instance->vertexDescriptors[0].data;
            instance->vertexDescriptors[0].data = NULL;
            if (geometry->flags & rpGEOMETRYNORMALS) {
                atomicData->normals = instance->vertexDescriptors[1].data;
                instance->vertexDescriptors[1].data = NULL;
            }
        }
    } else if (skin->positions == NULL) {
        skin->positions = instance->vertexDescriptors[0].data;
        instance->vertexDescriptors[0].data = NULL;
        if (geometry->flags & rpGEOMETRYNORMALS) {
            skin->normals = instance->vertexDescriptors[1].data;
            instance->vertexDescriptors[1].data = NULL;
        }
    }

    /* a buffer the GPU may still read is left to its entry */
    if (instance->vertexDescriptors[0].data != NULL &&
        !fn_8049A78C(SKINVERTEXBUFFER(instance->vertexDescriptors[0].data)->syncToken)) {
        SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data)->ownerRef = NULL;
        instance->vertexDescriptors[0].data = NULL;
        if (geometry->flags & rpGEOMETRYNORMALS) {
            instance->vertexDescriptors[1].data = NULL;
        }
    }

    if (instance->vertexDescriptors[0].data == NULL) {
        RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);

        /* the buffer header, then 32-byte aligned positions and normals */
        if (format != NULL) {
            /* the bytes of an xyz vector of each GX component type (u8, s8, u16, s16, f32) */
            RwInt32 componentSizes[5] = {3, 3, 6, 6, 12};

            size = ((geometry->numVertices * componentSizes[format->posType] + 31) & ~31) + 6;
            if (geometry->flags & rpGEOMETRYNORMALS) {
                /* binormals and tangents, when present, follow each normal */
                RwInt32 normalSize =
                    componentSizes[format->normalType] * (format->normalNbt != 0 ? 3 : 1);

                size += (geometry->numVertices * normalSize + 31) & ~31;
            }
        } else {
            RwUInt32 streamSize = (geometry->numVertices * sizeof(RwV3d) + 31) & ~31;

            size = streamSize + 6;
            if (geometry->flags & rpGEOMETRYNORMALS) {
                size += streamSize;
            }
        }

        buffer = fn_80480878(atomic, (RwResEntry **)&instance->vertexDescriptors[0].data, size,
                             fn_8044CEE0);
        if (geometry->numMorphTargets == 1) {
            ownerEntry = geometry->repEntry;
        } else {
            ownerEntry = atomic->repEntry;
        }
        if (buffer == NULL || ownerEntry == NULL) {
            if (buffer != NULL) {
                fn_8048071C(buffer);
            }
            return FALSE;
        }

        instance->vertexDescriptors[0].data =
            (void *)(((RwUInt32)instance->vertexDescriptors[0].data + sizeof(RwResEntry) + 6 + 31) &
                     ~31);
        if (geometry->flags & rpGEOMETRYNORMALS) {
            instance->vertexDescriptors[1].data =
                (RwUInt8 *)instance->vertexDescriptors[0].data +
                ((geometry->numVertices * instance->vertexDescriptors[0].stride + 31) & ~31);
        }
        SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data) = buffer;
    } else {
        RwResourcesUseResEntry(SKINVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data));
    }

    SKINVERTEXBUFFER(instance->vertexDescriptors[0].data)->syncToken = lbl_805EDF40;
    return TRUE;
}

/* Updates the skin's bone matrices from the hierarchy: the bone's frame or the hierarchy's matrix,
 * after the skin-to-bone matrix, and for skins drawn with the atomic's frame, before the inverse of
 * that frame */
void fn_8044D344(RwMatrix *matrices, RpSkin *skin, RwMatrix *ltm, RpHAnimHierarchy *hierarchy) {
    RwMatrix *toSkin;
    RwUInt32 i;

    if (hierarchy == NULL) {
        return;
    }

    if (hierarchy->flags & rpHANIMHIERARCHYNOMATRICES) {
        RwMatrix inverseLtm;
        RwMatrix *skinToBone;
        RwUInt8 *boneIds;
        RwUInt32 numBoneIds;

        if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
            inverseLtm.flags = 0;
            fn_8047F0B0(&inverseLtm, ltm);
            toSkin = &inverseLtm;
        } else {
            toSkin = &lbl_805E31CC;
        }

        boneIds = skin->boneIds;
        skinToBone = skin->skinToBoneMatrices;
        numBoneIds = skin->numBoneIds;
        for (i = 0; i < numBoneIds; i++) {
            RwMatrix boneMatrix;

            boneMatrix.flags = 0;
            fn_8047EBB8(&boneMatrix, &skinToBone[boneIds[i]],
                        RwFrameGetLTM(hierarchy->pNodeInfo[boneIds[i]].pFrame));
            fn_8047EBB8(&matrices[boneIds[i]], &boneMatrix, toSkin);
        }
    } else if (hierarchy->flags & rpHANIMHIERARCHYLOCALSPACEMATRICES) {
        if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
            RwMatrix *skinToBone = skin->skinToBoneMatrices;
            RwUInt8 *boneIds = skin->boneIds;
            RwUInt32 numBoneIds = skin->numBoneIds;

            for (i = 0; i < numBoneIds; i++) {
                RwUInt32 bone = boneIds[i];

                fn_8047EBB8(&matrices[bone], &skinToBone[bone], &hierarchy->pMatrixArray[bone]);
            }
        } else {
            RwMatrix frame;

            frame.flags = 0;
            fn_8047EBB8(&frame, ltm, &lbl_805E31CC);
            fn_80450D6C(matrices, skin->skinToBoneMatrices, hierarchy->pMatrixArray, &frame,
                        skin->boneIds, skin->numBoneIds);
        }
    } else {
        RwMatrix inverseLtm;

        if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
            inverseLtm.flags = 0;
            fn_8047F0B0(&inverseLtm, ltm);
            toSkin = &inverseLtm;
        } else {
            toSkin = &lbl_805E31CC;
        }
        fn_80450D6C(matrices, skin->skinToBoneMatrices, hierarchy->pMatrixArray, toSkin,
                    skin->boneIds, skin->numBoneIds);
    }
}

/* Skins the positions of a geometry into dstPositions */
void fn_8044D554(RpSkin *skin, RwMatrix *matrices, void *srcPositions, void *dstPositions,
                 RpGameCubeVertexFormat *format, RwInt32 numVertices) {
    SkinKernelParams params;
    /* GQR5 loads the byte weights as unsigned bytes scaled by 2^-7 */
    register RwUInt32 gqr5 = 0x07040000;
    register RwUInt32 gqr6 = 0;
    RwUInt32 savedGqr5; /* GQR5 is restored after the kernels */
    RwInt32 i;

    params.dstPositions = dstPositions;
    params.srcPositions = srcPositions;
    params.numVertices = numVertices;
    if (format != NULL) {
        RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
        RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};

        gqr6 = (format->posFrac << 8) | quantizeType[format->posType];
        gqr6 |= gqr6 << 16;
        params.positionSize = typeSize[format->posType];
    } else {
        params.positionSize = 4;
    }

    asm {
        mfspr r0, GQR5
        stw r0, savedGqr5
        mtspr GQR5, gqr5
        mtspr GQR6, gqr6
    }

    switch (skin->maxNumWeightsForVertex) {
    case 1:
        for (i = 0; i < numVertices; i++) {
            fn_80482458((RwV3d *)dstPositions + i, (RwV3d *)skin->positions + i,
                        &matrices[skin->vertexBoneIndices[i] & 0xFF]);
        }
        break;
    case 2:
        fn_80450F68(matrices, skin, &params);
        break;
    case 3:
        fn_80451254(matrices, skin, &params);
        break;
    case 4:
        fn_80451618(matrices, skin, &params);
        break;
    }

    asm {
        lwz r0, savedGqr5
        mtspr GQR5, r0
    }

    DCFlushRange(dstPositions, (numVertices * (params.positionSize * 3) + 31) & ~31);
    GXInvalidateVtxCache();
}

/* Skins the positions and normals of a geometry */
void fn_8044D6E8(RpSkin *skin, RwMatrix *matrices, void *srcPositions, void *srcNormals,
                 void *dstPositions, void *dstNormals, RpGameCubeVertexFormat *format,
                 RwInt32 numVertices) {
    SkinKernelParams params;
    /* GQR5 loads the byte weights as unsigned bytes scaled by 2^-7 */
    register RwUInt32 gqr5 = 0x07040000;
    register RwUInt32 gqr6 = 0;
    register RwUInt32 gqr7 = 0;
    RwUInt32 savedGqr5; /* GQR5 is restored after the kernels */
    RwInt32 i;

    params.dstPositions = dstPositions;
    params.dstNormals = dstNormals;
    params.srcPositions = srcPositions;
    params.srcNormals = srcNormals;
    params.numVertices = numVertices;
    if (format != NULL) {
        RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
        RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};
        RwUInt8 normalScale[5] = {0, 6, 0, 14, 0};

        params.positionSize = typeSize[format->posType];
        params.normalSize = typeSize[format->normalType];
        gqr6 = (format->posFrac << 8) | quantizeType[format->posType];
        gqr6 |= gqr6 << 16;
        gqr7 = (normalScale[format->normalType] << 8) | quantizeType[format->normalType];
        gqr7 |= gqr7 << 16;
        if (format->normalNbt != 0) {
            params.nbtSize = params.normalSize * 6;
        } else {
            params.nbtSize = 0;
        }
    } else {
        params.positionSize = 4;
        params.normalSize = 4;
        params.nbtSize = 0;
    }

    asm {
        mfspr r0, GQR5
        stw r0, savedGqr5
        mtspr GQR5, gqr5
        mtspr GQR6, gqr6
        mtspr GQR7, gqr7
    }

    switch (skin->maxNumWeightsForVertex) {
    case 1:
        for (i = 0; i < numVertices; i++) {
            RwMatrix *matrix = &matrices[skin->vertexBoneIndices[i] & 0xFF];

            fn_80482458((RwV3d *)dstPositions + i, (RwV3d *)srcPositions + i, matrix);
            fn_80482504((RwV3d *)dstNormals + i, (RwV3d *)srcNormals + i, matrix);
        }
        break;
    case 2:
        fn_80451080(matrices, skin, &params);
        break;
    case 3:
        fn_804513C4(matrices, skin, &params);
        break;
    case 4:
        fn_804517C8(matrices, skin, &params);
        break;
    }

    asm {
        lwz r0, savedGqr5
        mtspr GQR5, r0
    }

    DCFlushRange(dstPositions, ((numVertices * (params.positionSize * 3) + 31) & ~31) +
                                   numVertices * (params.nbtSize + params.normalSize * 3));
    GXInvalidateVtxCache();
}

/* Instances a skinned geometry: rigid skins and toon skins drawn with the frame use the skin
 * instancing, others the generic instancing; geometries with native data use the native skin
 * instancing in render mode 1 */
RpAtomic *fn_8044D924(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry = atomic->geometry;
    RpSkin *skin = fn_8044CCAC(geometry);
    void *owner;
    RwResEntry **ownerRef;

    if (geometry->numMorphTargets != 1) {
        owner = atomic;
        ownerRef = &atomic->repEntry;
    } else {
        owner = geometry;
        ownerRef = &geometry->repEntry;
    }

    if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
        if (geometry->flags & 0x02000000) {
            if (lbl_805EDDD8 == 1) {
                renderMesh->repEntry = fn_8044F9E4(geometry, owner, ownerRef);
            } else {
                renderMesh->repEntry = fn_8046C0D4(geometry, owner, ownerRef);
            }
        } else {
            renderMesh->repEntry = fn_8046C0D4(geometry, owner, ownerRef);
        }
    } else if (geometry->flags & 0x02000000) {
        if (lbl_805EDDD8 == 1) {
            renderMesh->repEntry = fn_8044F9E4(geometry, owner, ownerRef);
        } else {
            renderMesh->repEntry = fn_80450938(geometry, owner, ownerRef);
        }
    } else {
        renderMesh->repEntry = fn_80450938(geometry, owner, ownerRef);
    }
    geometry->lockedSinceLastInst = 0;
    return atomic;
}

/* Reinstances a skinned geometry: updates the bone matrices and skins the vertices into a fresh
 * vertex buffer */
RpAtomic *fn_8044DA3C(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry = atomic->geometry;
    RpSkin *skin = RPSKINGEOMETRYGETDATA(geometry);

    if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
        RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
        void *owner = renderMesh->repEntry->owner;
        void **bindPositions;
        void **bindNormals;

        if (((RwObject *)owner)->type == rpATOMIC) {
            SkinAtomicData *atomicData = RPSKINATOMICGETDATA(owner);

            bindPositions = &atomicData->positions;
            bindNormals = &atomicData->normals;
        } else {
            bindPositions = &skin->positions;
            bindNormals = &skin->normals;
        }

        if (*bindPositions == NULL) {
            if (!(geometry->flags & rpGEOMETRYNATIVE)) {
                fn_80463840(atomic, renderMesh);
            }
            *bindPositions = instance->vertexDescriptors[0].data;
            instance->vertexDescriptors[0].data = NULL;
            if (geometry->flags & rpGEOMETRYNORMALS) {
                *bindNormals = instance->vertexDescriptors[1].data;
                instance->vertexDescriptors[1].data = NULL;
            }
        } else if (!(geometry->flags & rpGEOMETRYNATIVE)) {
            /* refresh the bind pose from the geometry */
            void *positions = instance->vertexDescriptors[0].data;

            instance->vertexDescriptors[0].data = *bindPositions;
            if (geometry->flags & rpGEOMETRYNORMALS) {
                void *normals = instance->vertexDescriptors[1].data;

                instance->vertexDescriptors[1].data = *bindNormals;
                fn_80463840(atomic, renderMesh);
                instance->vertexDescriptors[1].data = normals;
            } else {
                fn_80463840(atomic, renderMesh);
            }
            instance->vertexDescriptors[0].data = positions;
        }

        if (!fn_8044CF68(skin, atomic, instance, renderMesh)) {
            return NULL;
        }
        fn_8044D344(lbl_8056D708.matrixCache, skin, RwFrameGetLTM(atomic->object.parent),
                    RPSKINATOMICGETDATA(atomic)->hierarchy);
        if (geometry->flags & rpGEOMETRYNORMALS) {
            fn_8044D6E8(skin, lbl_8056D708.matrixCache, *bindPositions, *bindNormals,
                        instance->vertexDescriptors[0].data, instance->vertexDescriptors[1].data,
                        GAMECUBEVERTEXFORMAT(geometry), geometry->numVertices);
        } else {
            fn_8044D554(skin, lbl_8056D708.matrixCache, *bindPositions,
                        instance->vertexDescriptors[0].data, GAMECUBEVERTEXFORMAT(geometry),
                        geometry->numVertices);
        }
    } else {
        fn_8044D344(lbl_8056D708.matrixCache, skin, RwFrameGetLTM(atomic->object.parent),
                    RPSKINATOMICGETDATA(atomic)->hierarchy);
    }
    return atomic;
}

/* Loads a matrix as a GX position (and normal) matrix: RenderWare matrices are column vectors with
 * the x and z axes the other way round */
static inline void SkinLoadMatrix(RwMatrix *matrix, RwUInt32 index, RwBool normals) {
    Mtx mtx;

    mtx[0][0] = -matrix->right.x;
    mtx[0][1] = -matrix->up.x;
    mtx[0][2] = -matrix->at.x;
    mtx[0][3] = -matrix->pos.x;
    mtx[1][0] = matrix->right.y;
    mtx[1][1] = matrix->up.y;
    mtx[1][2] = matrix->at.y;
    mtx[1][3] = matrix->pos.y;
    mtx[2][0] = -matrix->right.z;
    mtx[2][1] = -matrix->up.z;
    mtx[2][2] = -matrix->at.z;
    mtx[2][3] = -matrix->pos.z;
    GXLoadPosMtxImm(mtx, index);
    if (normals) {
        GXLoadNrmMtxImm(mtx, index);
    }
}

/* Loads a bone matrix into a GX matrix slot */
void fn_8044DC4C(RwMatrix *matrix, RwUInt32 index, RwBool normals) {
    SkinLoadMatrix(matrix, index, normals);
}

/* Loads the bone matrices that a split mesh uses into consecutive GX matrix slots */
void fn_8044DD1C(RpSkin *skin, RwInt32 meshIndex, RwBool normals) {
    SkinRLE *meshRuns = &skin->skinSplitData.meshRLECount[meshIndex];
    RwUInt32 firstRun = meshRuns->start & 0x7F;
    RwUInt8 numRuns = meshRuns->length;
    RwUInt32 index = 0;
    RwUInt8 run;

    for (run = 0; run < numRuns; run++) {
        SkinRLE *bones = &skin->skinSplitData.meshRLE[firstRun + run];
        RwUInt32 bone = bones->start;
        RwUInt32 numBones = bones->length;
        RwUInt32 i;

        for (i = 0; i < numBones; i++, bone++, index += 3) {
            SkinLoadMatrix(&lbl_8056D708.matrixCache[bone], index, normals);
        }
    }
}

/* Renders a skinned atomic: rigid skins load their bone matrices up front, or per mesh when split;
 * each mesh sets its texture and material color and calls its display list */
RpAtomic *fn_8044DE74(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RpGameCubeDisplayList *displayList;
    RwMatrix *ltm;
    RpSkin *skin;
    RpGameCubeMaterialColorCallBack setMaterialColor;
    RwUInt32 numMeshes;
    RpMesh *mesh;
    RwUInt32 i;
    RwUInt32 index;

    displayList = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);
    instance->syncToken = lbl_805EDF40;
    ltm = RwFrameGetLTM(atomic->object.parent);
    fn_80462C30(GAMECUBEVERTEXFORMAT(atomic->geometry), renderMesh);

    skin = fn_8044CCAC(atomic->geometry);
    if (skin->maxNumWeightsForVertex > 1 || skin->pipelineType == SKINPIPELINETOON) {
        fn_804950AC(ltm, (renderMesh->geometryFlags >> 4) & 1);
    } else {
        GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    }
    if (GAMECUBEVERTEXFORMAT(atomic->geometry) == NULL) {
        fn_80462E1C();
    }

    setMaterialColor = fn_804655D4(renderMesh->geometryFlags, renderMesh->lightMask,
                                   renderMesh->hasAmbient, instance->flags & 1);
    numMeshes = renderMesh->meshHeader->numMeshes;
    mesh = (RpMesh *)(renderMesh->meshHeader + 1);

    /* rigid skins load all their bone matrices up front */
    if (skin->maxNumWeightsForVertex == 1 && skin->skinSplitData.numMeshes == 0) {
        for (i = 0, index = 0; i < skin->numBoneIds; index += 3, i++) {
            SkinLoadMatrix(&lbl_8056D708.matrixCache[skin->boneIds[i]], index,
                           (renderMesh->geometryFlags >> 4) & 1);
        }
    }

    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        while (numMeshes-- != 0) {
            RwTexture *texture = mesh->material->texture;

            fn_80499C38(texture, 0);
            if (texture != NULL && texture->raster != NULL) {
                fn_80497778(
                    (RWPLUGINOFFSET(RwGameCubeRasterExt, texture->raster->parent, lbl_805F1658)
                         ->flags &
                     1) ^
                    1);
            } else {
                fn_80497778(TRUE);
            }
            if (setMaterialColor != NULL) {
                setMaterialColor(&renderMesh->ambientColor, &mesh->material->color,
                                 mesh->material->surfaceProps.ambient);
            }
            if (skin->maxNumWeightsForVertex == 1 && skin->skinSplitData.numMeshes != 0) {
                fn_8044DD1C(skin, renderMesh->meshHeader->numMeshes - (numMeshes + 1),
                            (renderMesh->geometryFlags >> 4) & 1);
            }
            GXCallDisplayList(displayList->data, displayList->size);
            displayList++;
            mesh++;
        }
    } else {
        fn_80497778(TRUE);
        while (numMeshes-- != 0) {
            if (setMaterialColor != NULL) {
                setMaterialColor(&renderMesh->ambientColor, &mesh->material->color,
                                 mesh->material->surfaceProps.ambient);
            }
            if (skin->maxNumWeightsForVertex == 1 && skin->skinSplitData.numMeshes != 0) {
                fn_8044DD1C(skin, renderMesh->meshHeader->numMeshes - (numMeshes + 1),
                            (renderMesh->geometryFlags >> 4) & 1);
            }
            GXCallDisplayList(displayList->data, displayList->size);
            displayList++;
            mesh++;
        }
    }
    return atomic;
}

/* Prepares a skin's weights for the kernels: each vertex's weights are sorted heaviest first with
 * their bone indices, and for skins with more than one weight per vertex they are stored as bytes
 * (1.0 is 128) that add up to at least 128, with the bone indices as bytes in the same order */
RpSkin *fn_8044E408(RpSkin *skin, RwUInt32 numVertices) {
    RwUInt32 i;
    RwUInt32 j;
    RwUInt32 k;

    skin->positions = NULL;
    skin->normals = NULL;
    skin->weights = NULL;
    skin->indices = NULL;

    for (i = 0; i < numVertices; i++) {
        for (j = 0; j < 3; j++) {
            RwUInt32 jShift = j * 8;
            RwUInt32 jMask = ~(0xFF << jShift);

            /* compare the j-th weight with each later one */
            for (k = 1; k < 4 - j; k++) {
                RwReal *weights = &skin->vertexBoneWeights[i].w0 + j;

                if (weights[k] > weights[0]) {
                    RwReal weight = weights[0];
                    RwUInt32 kShift = (j + k) * 8;
                    RwUInt32 kMask = ~(0xFF << kShift);
                    RwUInt32 indices;
                    RwUInt32 jIndex;
                    RwUInt32 kIndex;

                    weights[0] = weights[k];
                    weights[k] = weight;

                    indices = skin->vertexBoneIndices[i];
                    skin->vertexBoneIndices[i] = indices & jMask;
                    kIndex = (indices >> kShift) & 0xFF;
                    jIndex = (indices >> jShift) & 0xFF;
                    skin->vertexBoneIndices[i] |= kIndex << jShift;
                    skin->vertexBoneIndices[i] &= kMask;
                    skin->vertexBoneIndices[i] |= jIndex << kShift;
                }
            }
        }
    }

    if (skin->maxNumWeightsForVertex > 1) {
        RwUInt8 *weight;
        RwUInt8 *index;

        skin->weights = RwMalloc(numVertices * (skin->maxNumWeightsForVertex * 2) + 3,
                                 rwMEMHINTDUR_EVENT | rpID_SKINPLUGIN);
        weight = skin->weights;
        for (i = 0; i < numVertices; i++) {
            RwUInt32 total = 0;

            for (j = 0; j < skin->maxNumWeightsForVertex; j++) {
                *weight = 128.0f * (&skin->vertexBoneWeights[i].w0)[j];
                total += *weight++;
            }
            /* make up the rounding on the heaviest weights */
            if ((RwUInt8)total < 128) {
                for (j = 0; j < skin->maxNumWeightsForVertex; j++) {
                    /* the vertex's j-th weight */
                    (*(weight - (skin->maxNumWeightsForVertex - j)))++;
                    total++;
                    if ((RwUInt8)total == 128) {
                        break;
                    }
                }
            }
        }

        skin->indices = skin->weights + skin->maxNumWeightsForVertex * numVertices;
        skin->indices = (RwUInt8 *)(((RwUInt32)skin->indices + 3) & ~3);
        index = skin->indices;
        for (i = 0; i < numVertices; i++) {
            for (j = 0; j < skin->maxNumWeightsForVertex; j++) {
                *index++ = skin->vertexBoneIndices[i] >> (j * 8);
            }
        }
    }
    return skin;
}
