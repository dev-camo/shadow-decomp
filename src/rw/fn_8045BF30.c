/* RenderWare meshes: the mesh module's globals, mesh headers, building meshes from triangles, and
 * reading and writing meshes in a stream. The original file name is unknown. */
#include "rw/rpworld.h"
#include "layout_check.h"

/* Reports an error: the arguments are the error code and its parameters */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_WORLDPLUGIN;                                                        \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* how many indices are converted through the stack buffer at a time */
#define rpMESHINDEXBUFFERSIZE 256

/* Whether a geometry's or world's meshes are instanced in the platform's format, without stored
 * indices */
#define rpMESHNATIVE(type, object)                                                                 \
    (((type) == rpGEOMETRY && (((const RpGeometry *)(object))->flags & rpGEOMETRYNATIVE)) ||       \
     ((type) == rpWORLD && (((const RpWorld *)(object))->flags & rpWORLDNATIVE)))

/* Whether a geometry's or world's meshes store their indices */
#define rpMESHINDEXED(type, object)                                                                \
    (((type) == rpGEOMETRY && !(((const RpGeometry *)(object))->flags & rpGEOMETRYNATIVE)) ||      \
     ((type) == rpWORLD && !(((const RpWorld *)(object))->flags & rpWORLDNATIVE)))

/* The stream header of a mesh header (0xC bytes) */
typedef struct BinMeshHeader {
    /* 0x0 */ RwUInt32 flags;
    /* 0x4 */ RwUInt32 numMeshes;
    /* 0x8 */ RwUInt32 totalIndicesInMesh;
} BinMeshHeader;
CHECK_SIZEOF(BinMeshHeader, 0xC);

/* The stream header of a mesh (8 bytes) */
typedef struct BinMesh {
    /* 0x0 */ RwUInt32 numIndices;
    /* 0x4 */ RwInt32 matIndex;
} BinMesh;
CHECK_SIZEOF(BinMesh, 0x8);

/* The mesh unit's statics. The original initialises them as an aggregate, which places them in
 * .sdata even though they are zero (inferred). */
typedef struct MeshStatics {
    /* 0x0 */ RwFreeList *buildMeshFreeList;
} MeshStatics;
CHECK_SIZEOF(MeshStatics, 0x4);

MeshStatics lbl_805EDDC0 = {NULL};

RwModuleInfo lbl_805F14F0;

/* Allocates a mesh header of size bytes, with its meshes */
RpMeshHeader *fn_8045BF30(RwUInt32 size) {
    return RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
}

/* Destroys the build mesh free list (inferred) */
static inline void MeshDestroyFreeList(void) {
    if (lbl_805EDDC0.buildMeshFreeList != NULL) {
        fn_8047FCDC(lbl_805EDDC0.buildMeshFreeList);
        lbl_805EDDC0.buildMeshFreeList = NULL;
    }
}

/* engine plugin destructor: the last instance destroys the build mesh free list */
void *fn_8045BF64(void *instance, RwInt32 offset, RwInt32 size) {
    if (--lbl_805F14F0.numInstances == 0) {
        MeshDestroyFreeList();
    }
    return instance;
}

/* engine plugin constructor: the build mesh free list, the serial numbers and the primitive type
 * tables */
void *fn_8045BFBC(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14F0.globalsOffset = offset;
    if (lbl_805F14F0.numInstances == 0) {
        lbl_805EDDC0.buildMeshFreeList =
            fn_8047FBD8(sizeof(RpBuildMesh), 50, 4, rwMEMHINTDUR_GLOBAL | rwID_MESHMODULE);
        if (lbl_805EDDC0.buildMeshFreeList == NULL) {
            MeshDestroyFreeList();
            return NULL;
        }
    }
    RPMESHGLOBAL(nextSerialNum) = 1;
    lbl_805F14F0.numInstances++;

    RPMESHGLOBAL(meshFlagsToPrimType)[0] = rwPRIMTYPETRILIST;
    RPMESHGLOBAL(meshFlagsToPrimType)[rpMESHHEADERTRISTRIP] = rwPRIMTYPETRISTRIP;
    RPMESHGLOBAL(meshFlagsToPrimType)[rpMESHHEADERTRIFAN] = rwPRIMTYPETRIFAN;
    RPMESHGLOBAL(meshFlagsToPrimType)[rpMESHHEADERLINELIST] = rwPRIMTYPELINELIST;
    RPMESHGLOBAL(meshFlagsToPrimType)[rpMESHHEADERPOLYLINE] = rwPRIMTYPEPOLYLINE;
    RPMESHGLOBAL(meshFlagsToPrimType)[rpMESHHEADERPOINTLIST] = rwPRIMTYPEPOINTLIST;

    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPELINELIST] = rpMESHHEADERLINELIST;
    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPEPOLYLINE] = rpMESHHEADERPOLYLINE;
    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPETRILIST] = 0;
    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPETRISTRIP] = rpMESHHEADERTRISTRIP;
    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPETRIFAN] = rpMESHHEADERTRIFAN;
    RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPEPOINTLIST] = rpMESHHEADERPOINTLIST;
    return instance;
}

/* Creates a build mesh with room for numTriangles triangles */
RpBuildMesh *fn_8045C144(RwUInt32 numTriangles) {
    RpBuildMesh *buildMesh =
        RwFreeListAlloc(lbl_805EDDC0.buildMeshFreeList, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

    if (buildMesh != NULL) {
        buildMesh->numTriangles = 0;
        if (numTriangles != 0) {
            RwUInt32 bytes = numTriangles * sizeof(RpBuildMeshTriangle);

            buildMesh->meshTriangles =
                RwMalloc(bytes, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
            if (buildMesh->meshTriangles == NULL) {
                RwFreeListFree(lbl_805EDDC0.buildMeshFreeList, buildMesh);
                RWERROR((E_RW_NOMEM, bytes));
                return NULL;
            }
            buildMesh->triangleBufferSize = numTriangles;
        } else {
            buildMesh->meshTriangles = NULL;
            buildMesh->triangleBufferSize = 0;
        }
        return buildMesh;
    }
    RWERROR((E_RW_NOMEM, sizeof(RpBuildMesh)));
    return NULL;
}

RwBool fn_8045C26C(RpBuildMesh *buildMesh) {
    if (buildMesh->meshTriangles != NULL) {
        RwFree(buildMesh->meshTriangles);
        buildMesh->meshTriangles = NULL;
    }
    RwFreeListFree(lbl_805EDDC0.buildMeshFreeList, buildMesh);
    return TRUE;
}

/* Destroys a mesh header; the shared empty header, all zero, is not freed */
RwBool fn_8045C2D4(RpMeshHeader *mesh) {
    if (mesh->flags != 0 || mesh->numMeshes != 0 || mesh->serialNum != 0 ||
        mesh->totalIndicesInMesh != 0 || mesh->firstMeshOffset != 0) {
        RwFree(mesh);
    }
    return TRUE;
}

/* Adds a triangle, growing the triangle buffer by one when it is full */
RpBuildMesh *fn_8045C340(RpBuildMesh *buildMesh, RpMaterial *material, RwUInt16 vert0,
                         RwUInt16 vert1, RwUInt16 vert2, RwUInt16 matIndex, RwUInt16 textureIndex,
                         RwUInt16 rasterIndex, RwUInt16 pipelineIndex) {
    if (buildMesh->numTriangles >= buildMesh->triangleBufferSize) {
        RwUInt32 bytes = (buildMesh->numTriangles + 1) * sizeof(RpBuildMeshTriangle);
        RpBuildMeshTriangle *triangles;

        if (buildMesh->numTriangles != 0) {
            triangles = RwRealloc(buildMesh->meshTriangles, bytes,
                                  rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
        } else {
            triangles =
                RwMalloc(bytes, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
        }
        if (triangles == NULL) {
            RWERROR((E_RW_NOMEM, bytes));
            return NULL;
        }
        buildMesh->meshTriangles = triangles;
        buildMesh->triangleBufferSize = buildMesh->numTriangles + 1;
    }
    buildMesh->meshTriangles[buildMesh->numTriangles].material = material;
    buildMesh->meshTriangles[buildMesh->numTriangles].vertIndex[0] = vert0;
    buildMesh->meshTriangles[buildMesh->numTriangles].vertIndex[1] = vert1;
    buildMesh->meshTriangles[buildMesh->numTriangles].vertIndex[2] = vert2;
    buildMesh->meshTriangles[buildMesh->numTriangles].matIndex = matIndex;
    buildMesh->meshTriangles[buildMesh->numTriangles].textureIndex = textureIndex;
    buildMesh->meshTriangles[buildMesh->numTriangles].rasterIndex = rasterIndex;
    buildMesh->meshTriangles[buildMesh->numTriangles].pipelineIndex = pipelineIndex;
    buildMesh->numTriangles++;
    return buildMesh;
}

/* Calls back for each mesh of a header until the callback returns NULL */
RpMeshHeader *fn_8045C4E4(RpMeshHeader *meshHeader, RpMeshCallBack callBack, void *data) {
    RwInt32 numMeshes = meshHeader->numMeshes;
    RpMesh *mesh = (RpMesh *)((RwUInt8 *)(meshHeader + 1) + meshHeader->firstMeshOffset);

    while (numMeshes--) {
        if (callBack(mesh, meshHeader, data) == NULL) {
            return meshHeader;
        }
        mesh++;
    }
    return meshHeader;
}

/* Writes a mesh header and its meshes; the indices only when they are not instanced */
RwStream *fn_8045C56C(const RpMeshHeader *meshHeader, const void *object, RwStream *stream,
                      const RpMaterialList *matList) {
    BinMeshHeader binMeshHeader;
    BinMesh binMesh;
    RwUInt32 buffer[rpMESHINDEXBUFFERSIZE];
    RwUInt32 count;
    RwUInt32 type;
    RwUInt32 numMeshes;
    const RpMesh *mesh;
    RwUInt32 remaining;
    const RxVertexIndex *indices;

    binMeshHeader.flags = meshHeader->flags;
    binMeshHeader.numMeshes = meshHeader->numMeshes;
    binMeshHeader.totalIndicesInMesh = meshHeader->totalIndicesInMesh;
    if (fn_8047D458(stream, (RwInt32 *)&binMeshHeader, sizeof(binMeshHeader)) == NULL) {
        return NULL;
    }

    numMeshes = meshHeader->numMeshes;
    mesh = (const RpMesh *)(meshHeader + 1);
    type = ((const RwObject *)object)->type;
    while (numMeshes--) {
        binMesh.numIndices = mesh->numIndices;
        binMesh.matIndex = fn_8045B8EC(matList, mesh->material);
        if (binMesh.matIndex < 0) {
            binMesh.matIndex = 0;
        }
        if (fn_8047D458(stream, (RwInt32 *)&binMesh, sizeof(binMesh)) == NULL) {
            return NULL;
        }

        if (rpMESHINDEXED(type, object)) {
            remaining = mesh->numIndices;
            indices = mesh->indices;
            while (remaining != 0) {
                RwUInt32 i;

                count = (remaining < rpMESHINDEXBUFFERSIZE) ? remaining : rpMESHINDEXBUFFERSIZE;
                for (i = 0; i < count; i++) {
                    buffer[i] = *indices++;
                }
                if (fn_8047D458(stream, (RwInt32 *)buffer, count * sizeof(RwUInt32)) == NULL) {
                    return NULL;
                }
                remaining -= count;
            }
        }
        mesh++;
    }
    return stream;
}

/* Reads a mesh header and its meshes, which follow it, with their indices after them */
RpMeshHeader *fn_8045C770(RwStream *stream, const void *object, const RpMaterialList *matList) {
    BinMeshHeader binMeshHeader;
    BinMesh binMesh;
    RwUInt32 buffer[rpMESHINDEXBUFFERSIZE];
    RwUInt32 size;
    RpMeshHeader *meshHeader;
    RwUInt32 count;
    RwUInt32 numMeshes;
    RwUInt32 type;
    RpMesh *mesh;
    RwUInt32 remaining;
    RxVertexIndex *indices;

    if (fn_8047D868(stream, (RwInt32 *)&binMeshHeader, sizeof(binMeshHeader)) == NULL) {
        return NULL;
    }

    /* 16 bytes for each mesh, 4 more than an RpMesh */
    size = sizeof(RpMeshHeader) + binMeshHeader.numMeshes * 16;
    if (rpMESHINDEXED(((const RwObject *)object)->type, object)) {
        size += binMeshHeader.totalIndicesInMesh * sizeof(RxVertexIndex);
    }
    meshHeader = RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
    if (meshHeader != NULL) {
        mesh = (RpMesh *)(meshHeader + 1);
        indices = (RxVertexIndex *)(mesh + binMeshHeader.numMeshes);
        type = ((const RwObject *)object)->type;

        meshHeader->flags = binMeshHeader.flags;
        meshHeader->numMeshes = binMeshHeader.numMeshes;
        meshHeader->serialNum = RPMESHGLOBAL(nextSerialNum);
        meshHeader->totalIndicesInMesh = binMeshHeader.totalIndicesInMesh;
        meshHeader->firstMeshOffset = 0;
        RPMESHGLOBAL(nextSerialNum)++;

        numMeshes = meshHeader->numMeshes;
        while (numMeshes--) {
            if (fn_8047D868(stream, (RwInt32 *)&binMesh, sizeof(binMesh)) == NULL) {
                return NULL;
            }
            mesh->numIndices = binMesh.numIndices;
            mesh->material = fn_8045B79C(matList, binMesh.matIndex);
            mesh->indices = indices;

            if (rpMESHINDEXED(type, object)) {
                remaining = mesh->numIndices;
                while (remaining != 0) {
                    RwUInt32 *source = buffer;

                    count = (remaining < rpMESHINDEXBUFFERSIZE) ? remaining : rpMESHINDEXBUFFERSIZE;
                    if (fn_8047D868(stream, (RwInt32 *)source, count * sizeof(RwUInt32)) == NULL) {
                        return NULL;
                    }
                    remaining -= count;
                    while (count--) {
                        *indices++ = *source++;
                    }
                }
            }
            mesh++;
        }
    }
    return meshHeader;
}

/* The stream size of a mesh header and its meshes */
RwInt32 fn_8045C9C8(const RpMeshHeader *meshHeader, const void *object) {
    if (rpMESHNATIVE(((const RwObject *)object)->type, object)) {
        return sizeof(BinMeshHeader) + meshHeader->numMeshes * sizeof(BinMesh);
    }
    return sizeof(BinMeshHeader) + meshHeader->numMeshes * sizeof(BinMesh) +
           meshHeader->totalIndicesInMesh * sizeof(RwUInt32);
}

/* The next mesh serial number */
RwInt16 fn_8045CA20(void) {
    return RPMESHGLOBAL(nextSerialNum)++;
}
