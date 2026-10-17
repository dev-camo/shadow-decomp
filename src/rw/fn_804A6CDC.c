/* RenderWare material effects plugin (plugin ID 0x120), GameCube multi-texturing: building the
 * binormals and tangents of instanced geometry whose multi-texture effects need them. The original
 * file name is unknown. */
#include "rw/rpmatfx.h"
#include <dolphin/gx.h>
#include <dolphin/os/OSCache.h>
#include "layout_check.h"

/* The binormal and tangent of a vertex are built once: each vertex's binormal starts with this
 * marker until a triangle that uses the vertex builds them (names inferred) */
#define rpMTNBTUNBUILTS8 0xFF
#define rpMTNBTUNBUILTS16 0xFFFF
#define rpMTNBTUNBUILTREAL 3.4028235e38f

/* The vertices of a triangle that builds the binormal and tangent of its first vertex, in the
 * vertex format's component types (name inferred) */
typedef struct MTNbtTriangle {
    /* 0x00 */ RwUInt8 *positions[3];
    /* 0x0C */ RwUInt8 *normal; /* the first vertex's normal, binormal and tangent */
    /* 0x10 */ RwUInt8 *texCoords[3];
    /* 0x1C */ RwUInt8 positionSize; /* the bytes of a component */
    /* 0x1D */ RwUInt8 normalSize;
    /* 0x1E */ RwUInt8 texCoordSize;
} MTNbtTriangle;
CHECK_SIZEOF(MTNbtTriangle, 0x20);

void fn_804A8558(MTNbtTriangle *triangle);

/* Points a triangle at the attributes of its vertices (name inferred) */
#define MTNBTSETTRIANGLE(triangle, v0, v1, v2, positions, normals, texCoords)                      \
    do {                                                                                           \
        (triangle)->positions[0] = (positions) + (triangle)->positionSize * 3 * (v0);              \
        (triangle)->positions[1] = (positions) + (triangle)->positionSize * 3 * (v1);              \
        (triangle)->positions[2] = (positions) + (triangle)->positionSize * 3 * (v2);              \
        (triangle)->normal = (normals) + (triangle)->normalSize * 9 * (v0);                        \
        (triangle)->texCoords[0] = (texCoords) + (triangle)->texCoordSize * 2 * (v0);              \
        (triangle)->texCoords[1] = (texCoords) + (triangle)->texCoordSize * 2 * (v1);              \
        (triangle)->texCoords[2] = (texCoords) + (triangle)->texCoordSize * 2 * (v2);              \
    } while (0)

/* Sets up the triangle of a triangle strip's vertex with 8-bit indices: the vertex with its
 * neighbours, stepping over degenerate triangles, in the strip's winding (inferred) */
void fn_804A6CDC(MTNbtTriangle *triangle, RwUInt8 *indices, RwUInt32 stride, RwUInt8 *positions,
                 RwUInt8 *normals, RwUInt8 *texCoords, RwInt32 vertex) {
    RwUInt8 v0 = indices[stride * vertex];
    RwUInt8 v1;
    RwUInt8 v2;
    RwUInt8 swap;

    if (vertex < 2) {
        if (vertex == 0) {
            v1 = indices[stride];
            v2 = indices[stride * 2];
        } else {
            v2 = indices[0];
            v1 = indices[stride * 2];
        }
    } else {
        v1 = indices[stride * (vertex - 2)];
        v2 = indices[stride * (vertex - 1)];
        if (v1 == v2) {
            RwUInt8 next = indices[stride * (vertex + 1)];

            if (v0 == next) {
                vertex++;
                v1 = indices[stride * (vertex + 1)];
                if (v0 == v1) {
                    vertex++;
                    v1 = indices[stride * (vertex + 1)];
                }
                v2 = indices[stride * (vertex + 2)];
            } else {
                v1 = v2;
                v2 = next;
            }
        }
        if (vertex & 1) {
            swap = v2;
            v2 = v1;
            v1 = swap;
        }
    }
    MTNBTSETTRIANGLE(triangle, v0, v1, v2, positions, normals, texCoords);
}

/* fn_804A6CDC with 16-bit indices (inferred) */
void fn_804A6E34(MTNbtTriangle *triangle, RwUInt8 *indices, RwUInt32 stride, RwUInt8 *positions,
                 RwUInt8 *normals, RwUInt8 *texCoords, RwInt32 vertex) {
    RwUInt16 v0 = *(RwUInt16 *)(indices + stride * vertex);
    RwUInt16 v1;
    RwUInt16 v2;
    RwUInt16 swap;

    if (vertex < 2) {
        if (vertex == 0) {
            v1 = *(RwUInt16 *)(indices + stride);
            v2 = *(RwUInt16 *)(indices + stride * 2);
        } else {
            v2 = *(RwUInt16 *)indices;
            v1 = *(RwUInt16 *)(indices + stride * 2);
        }
    } else {
        v1 = *(RwUInt16 *)(indices + stride * (vertex - 2));
        v2 = *(RwUInt16 *)(indices + stride * (vertex - 1));
        if (v1 == v2) {
            RwUInt16 next = *(RwUInt16 *)(indices + stride * (vertex + 1));

            if (v0 == next) {
                vertex++;
                v1 = *(RwUInt16 *)(indices + stride * (vertex + 1));
                if (v0 == v1) {
                    vertex++;
                    v1 = *(RwUInt16 *)(indices + stride * (vertex + 1));
                }
                v2 = *(RwUInt16 *)(indices + stride * (vertex + 2));
            } else {
                v1 = v2;
                v2 = next;
            }
        }
        if (vertex & 1) {
            swap = v2;
            v2 = v1;
            v1 = swap;
        }
    }
    MTNBTSETTRIANGLE(triangle, v0, v1, v2, positions, normals, texCoords);
}

/* Sets up the triangle of a triangle list's vertex with 8-bit indices (name inferred) */
static inline void MTNbtListTriangle8(MTNbtTriangle *triangle, RwUInt8 *indices, RwUInt32 stride,
                                      RwUInt8 *positions, RwUInt8 *normals, RwUInt8 *texCoords,
                                      RwInt32 vertex) {
    RwUInt8 v0 = indices[stride * vertex];
    RwUInt8 v1;
    RwUInt8 v2;

    switch (vertex % 3) {
    case 0:
        v1 = indices[stride * (vertex + 1)];
        v2 = indices[stride * (vertex + 2)];
        break;
    case 1:
        v1 = indices[stride * (vertex + 1)];
        v2 = indices[stride * (vertex - 1)];
        break;
    case 2:
        v1 = indices[stride * (vertex - 2)];
        v2 = indices[stride * (vertex - 1)];
        break;
    default:
        return;
    }
    MTNBTSETTRIANGLE(triangle, v0, v1, v2, positions, normals, texCoords);
}

/* MTNbtListTriangle8 with 16-bit indices (name inferred) */
static inline void MTNbtListTriangle16(MTNbtTriangle *triangle, RwUInt8 *indices, RwUInt32 stride,
                                       RwUInt8 *positions, RwUInt8 *normals, RwUInt8 *texCoords,
                                       RwInt32 vertex) {
    RwUInt16 v0 = *(RwUInt16 *)(indices + stride * vertex);
    RwUInt16 v1;
    RwUInt16 v2;

    switch (vertex % 3) {
    case 0:
        v1 = *(RwUInt16 *)(indices + stride * (vertex + 1));
        v2 = *(RwUInt16 *)(indices + stride * (vertex + 2));
        break;
    case 1:
        v1 = *(RwUInt16 *)(indices + stride * (vertex + 1));
        v2 = *(RwUInt16 *)(indices + stride * (vertex - 1));
        break;
    case 2:
        v1 = *(RwUInt16 *)(indices + stride * (vertex - 2));
        v2 = *(RwUInt16 *)(indices + stride * (vertex - 1));
        break;
    default:
        return;
    }
    MTNBTSETTRIANGLE(triangle, v0, v1, v2, positions, normals, texCoords);
}

/* Restores GQR5 (name inferred) */
static inline void MTSetGQR5(RwUInt32 value) {
    register RwUInt32 gqr;

    asm {
        lwz gqr, value
        mtspr GQR5, gqr
    }
}

/* The display list walk of fn_804A6F8C for a normal component type: each primitive's vertices
 * whose binormal is still marked unbuilt are built, with the triangles set up by setTriangle16 for
 * 16-bit indices or setTriangle8 for 8-bit indices. A display list ends at its size or at a zero
 * command. (name inferred) */
#define MTNBTBUILDPRIMITIVES(component, marker16, marker8, setTriangle16, setTriangle8)            \
    while (offset < displayList->size) {                                                           \
        offset += 3;                                                                               \
        numVertices = *(RwUInt16 *)(data + 1);                                                     \
        data += 3;                                                                                 \
        if (instance->vertexDescriptors[0].type == GX_INDEX16) {                                   \
            for (i = 0; i < numVertices; i++) {                                                    \
                index = *(RwUInt16 *)(data + vertexSize * i);                                      \
                if (((component *)normals)[index * 9 + 3] == (marker16)) {                         \
                    setTriangle16(&triangle, data, vertexSize, positions, normals, texCoords, i);  \
                    fn_804A8558(&triangle);                                                        \
                }                                                                                  \
            }                                                                                      \
        } else {                                                                                   \
            for (i = 0; i < numVertices; i++) {                                                    \
                index = data[vertexSize * i];                                                      \
                if (((component *)normals)[index * 9 + 3] == (marker8)) {                          \
                    setTriangle8(&triangle, data, vertexSize, positions, normals, texCoords, i);   \
                    fn_804A8558(&triangle);                                                        \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        data += vertexSize * numVertices;                                                          \
        offset += vertexSize * numVertices;                                                        \
        if (*data == 0) {                                                                          \
            break;                                                                                 \
        }                                                                                          \
    }

/* Builds the binormals and tangents of the vertices of a mesh's display list that are still
 * marked unbuilt (inferred) */
void fn_804A6F8C(RpGameCubeInstanceData *instance, RpGameCubeDisplayList *displayList,
                 RpGameCubeVertexFormat *format) {
    register RwUInt32 positionGqr;
    register RwUInt32 normalGqr;
    register RwUInt32 texCoordGqr;
    register RwUInt32 gqr;
    RwUInt32 savedGqr5; /* GQR5 is restored after the display list */
    MTNbtTriangle triangle;
    RwUInt8 *positions;
    RwUInt8 *normals;
    RwUInt8 *texCoords;
    RwUInt32 vertexSize;
    RwUInt8 *data;
    RwUInt32 offset;
    RwUInt32 numVertices;
    RwUInt32 i;
    RwUInt32 descriptor;
    RwUInt32 index;

    positions = instance->vertexDescriptors[0].data;
    normals = instance->vertexDescriptors[1].data;
    for (i = 2; instance->vertexDescriptors[i].attribute != GX_VA_TEX0; i++) {
    }
    texCoords = instance->vertexDescriptors[i].data;

    /* the bytes of a vertex's indices in the display list */
    vertexSize = 0;
    for (descriptor = 0; descriptor < instance->numVertexDescriptors; descriptor++) {
        if (instance->vertexDescriptors[descriptor].type == GX_INDEX16) {
            vertexSize += 2;
        } else {
            vertexSize += 1;
        }
    }

    if (format != NULL) {
        RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
        RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};
        RwUInt8 normalScale[5] = {0, 6, 0, 14, 0};

        triangle.positionSize = typeSize[format->posType];
        triangle.normalSize = typeSize[format->normalType];
        triangle.texCoordSize = typeSize[format->texCoordType[0]];
        positionGqr = quantizeType[format->posType] | (format->posFrac << 8);
        positionGqr |= positionGqr << 16;
        normalGqr = quantizeType[format->normalType] | (normalScale[format->normalType] << 8);
        normalGqr |= normalGqr << 16;
        texCoordGqr = quantizeType[format->texCoordType[0]] | (format->texCoordFrac[0] << 8);
        texCoordGqr |= texCoordGqr << 16;
    } else {
        /* the texture coordinate size is left unset, although the triangles are set up with it */
        triangle.positionSize = sizeof(RwReal);
        triangle.normalSize = sizeof(RwReal);
        positionGqr = normalGqr = texCoordGqr = 0;
    }

    asm {
        mfspr gqr, GQR5
        stw gqr, savedGqr5
        mtspr GQR5, positionGqr
        mtspr GQR6, normalGqr
        mtspr GQR7, texCoordGqr
    }

    data = displayList->data;
    offset = 0;
    if (*data == GX_TRIANGLESTRIP) {
        if (format != NULL) {
            if (format->normalType == GX_S8) {
                MTNBTBUILDPRIMITIVES(RwUInt8, rpMTNBTUNBUILTS8, rpMTNBTUNBUILTS8, fn_804A6E34,
                                     fn_804A6CDC);
            } else if (format->normalType == GX_S16) {
                /* with 8-bit indices this compares with the float marker, which a 16-bit
                 * binormal never equals, so no vertex is built */
                MTNBTBUILDPRIMITIVES(RwUInt16, rpMTNBTUNBUILTS16, rpMTNBTUNBUILTREAL, fn_804A6E34,
                                     fn_804A6CDC);
            } else {
                MTNBTBUILDPRIMITIVES(RwReal, rpMTNBTUNBUILTREAL, rpMTNBTUNBUILTREAL, fn_804A6E34,
                                     fn_804A6CDC);
            }
        } else {
            MTNBTBUILDPRIMITIVES(RwReal, rpMTNBTUNBUILTREAL, rpMTNBTUNBUILTREAL, fn_804A6E34,
                                 fn_804A6CDC);
        }
    } else {
        if (format != NULL) {
            if (format->normalType == GX_S8) {
                MTNBTBUILDPRIMITIVES(RwUInt8, rpMTNBTUNBUILTS8, rpMTNBTUNBUILTS8,
                                     MTNbtListTriangle16, MTNbtListTriangle8);
            } else if (format->normalType == GX_S16) {
                /* the float marker again, as for the strips */
                MTNBTBUILDPRIMITIVES(RwUInt16, rpMTNBTUNBUILTS16, rpMTNBTUNBUILTREAL,
                                     MTNbtListTriangle16, MTNbtListTriangle8);
            } else {
                MTNBTBUILDPRIMITIVES(RwReal, rpMTNBTUNBUILTREAL, rpMTNBTUNBUILTREAL,
                                     MTNbtListTriangle16, MTNbtListTriangle8);
            }
        } else {
            MTNBTBUILDPRIMITIVES(RwReal, rpMTNBTUNBUILTREAL, rpMTNBTUNBUILTREAL,
                                 MTNbtListTriangle16, MTNbtListTriangle8);
        }
    }

    MTSetGQR5(savedGqr5);
}

/* Whether a GameCube effect's texture coordinate generator uses binormals or tangents: it
 * generates from them or bump maps (name inferred) */
#define MTGCNTEXGENNEEDSNBT(texGen)                                                                \
    ((texGen)->src == GX_TG_BINRM || (texGen)->src == GX_TG_TANGENT ||                             \
     ((texGen)->func >= GX_TG_BUMP0 && (texGen)->func <= GX_TG_BUMP7))

/* Marks the binormals of numVertices normals of the component type unbuilt, counting with i (name
 * inferred) */
#define MTNBTMARKUNBUILT(component, normals, numVertices, marker, i)                               \
    do {                                                                                           \
        component *binormal = (normals);                                                           \
                                                                                                   \
        binormal += 3;                                                                             \
        for ((i) = 0; (i) < (numVertices); (i)++) {                                                \
            *binormal = (marker);                                                                  \
            binormal += 9;                                                                         \
        }                                                                                          \
    } while (0)

/* Builds the binormals and tangents of the meshes whose materials have a GameCube multi-texture
 * effect that needs them */
void fn_804A8120(RpGameCubeRenderMesh *renderMesh, RpGameCubeVertexFormat *format,
                 RwInt32 numVertices) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RpGameCubeDisplayList *displayList;
    RpMeshHeader *meshHeader = renderMesh->meshHeader;
    RpMesh *mesh;
    RwInt32 i;

    displayList = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);

    /* mark every vertex's binormal unbuilt */
    if (format != NULL) {
        if (format->normalType == GX_S8) {
            MTNBTMARKUNBUILT(RwUInt8, instance->vertexDescriptors[1].data, numVertices,
                             rpMTNBTUNBUILTS8, i);
        } else if (format->normalType == GX_S16) {
            MTNBTMARKUNBUILT(RwUInt16, instance->vertexDescriptors[1].data, numVertices,
                             rpMTNBTUNBUILTS16, i);
        } else {
            MTNBTMARKUNBUILT(RwReal, instance->vertexDescriptors[1].data, numVertices,
                             rpMTNBTUNBUILTREAL, i);
        }
    } else {
        MTNBTMARKUNBUILT(RwReal, instance->vertexDescriptors[1].data, numVertices,
                         rpMTNBTUNBUILTREAL, i);
    }

    mesh = (RpMesh *)(meshHeader + 1);
    for (i = 0; i < meshHeader->numMeshes; i++) {
        RpMultiTexture *multiTexture = fn_804A90C8(mesh->material, rwID_GAMECUBE);

        if (multiTexture != NULL && multiTexture->effect != NULL &&
            multiTexture->effect->platformID == rwID_GAMECUBE) {
            MTEffectGameCube *effect = fn_804A60D0(multiTexture->effect);
            RwInt32 j;

            for (j = 0; j < effect->numTexGens; j++) {
                if (MTGCNTEXGENNEEDSNBT(&effect->texGens[j])) {
                    fn_804A6F8C(instance, displayList, format);
                    break;
                }
            }
        }
        displayList++;
        mesh++;
    }

    DCFlushRange(instance->vertexDescriptors[1].data,
                 numVertices * instance->vertexDescriptors[1].stride);
    GXInvalidateVtxCache();
}

RwBool fn_804A8478(RpGameCubeRenderMesh *renderMesh) {
    RpMeshHeader *meshHeader = renderMesh->meshHeader;
    RpMesh *mesh = (RpMesh *)(meshHeader + 1);
    RwInt32 i;

    for (i = 0; i < meshHeader->numMeshes; i++, mesh++) {
        RpMultiTexture *multiTexture = fn_804A90C8(mesh->material, rwID_GAMECUBE);

        if (multiTexture != NULL && multiTexture->effect != NULL &&
            multiTexture->effect->platformID == rwID_GAMECUBE) {
            MTEffectGameCube *effect = fn_804A60D0(multiTexture->effect);
            RwInt32 j;

            for (j = 0; j < effect->numTexGens; j++) {
                if (MTGCNTEXGENNEEDSNBT(&effect->texGens[j])) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

/* Builds the binormal and tangent of a triangle's first vertex from the triangle's positions and
 * texture coordinates, loading and storing the components through GQR5 (positions), GQR6 (normals)
 * and GQR7 (texture coordinates). The binormal is the direction of the texture's S axis made
 * perpendicular to the normal, normalized and flipped for a mirrored texture; the tangent is the
 * binormal before the flip crossed with the normal (inferred) */
asm void fn_804A8558(register MTNbtTriangle *triangle) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lbz r4, MTNbtTriangle.positionSize(triangle)
    rotlwi r4, r4, 1
    lwz r5, MTNbtTriangle.positions[0](triangle)
    psq_l f0, 0(r5), 0, 5
    add r5, r5, r4
    psq_l f1, 0(r5), 1, 5
    lwz r5, MTNbtTriangle.positions[1](triangle)
    psq_l f2, 0(r5), 0, 5
    add r5, r5, r4
    psq_l f3, 0(r5), 1, 5
    lwz r5, MTNbtTriangle.positions[2](triangle)
    psq_l f4, 0(r5), 0, 5
    add r5, r5, r4
    psq_l f5, 0(r5), 1, 5
    lwz r5, MTNbtTriangle.texCoords[0](triangle)
    psq_l f6, 0(r5), 0, 7
    lwz r5, MTNbtTriangle.texCoords[1](triangle)
    psq_l f7, 0(r5), 0, 7
    lwz r5, MTNbtTriangle.texCoords[2](triangle)
    psq_l f8, 0(r5), 0, 7
    ps_sub f9, f2, f0
    ps_sub f10, f3, f1
    ps_sub f11, f4, f0
    ps_sub f12, f5, f1
    ps_sub f0, f7, f6
    ps_sub f1, f8, f6
    ps_muls1 f2, f9, f1
    ps_muls1 f3, f10, f1
    ps_muls1 f4, f11, f0
    ps_muls1 f5, f12, f0
    ps_sub f6, f4, f2
    ps_sub f7, f5, f3
    ps_merge10 f1, f1, f1
    ps_mul f0, f0, f1
    ps_merge10 f1, f0, f0
    ps_cmpu0 cr2, f1, f0
    lbz r4, MTNbtTriangle.normalSize(triangle)
    rotlwi r5, r4, 1
    lwz r6, MTNbtTriangle.normal(triangle)
    psq_l f8, 0(r6), 0, 6
    add r6, r6, r5
    psq_l f9, 0(r6), 1, 6
    add r6, r6, r4
    ps_mul f10, f8, f6
    ps_mul f11, f9, f7
    ps_sum1 f12, f10, f10, f10
    ps_sum0 f12, f11, f11, f12
    ps_merge00 f12, f12, f12
    ps_neg f12, f12
    ps_madd f0, f8, f12, f6
    ps_madd f1, f9, f12, f7
    ps_mul f2, f0, f0
    ps_mul f3, f1, f1
    ps_sum1 f12, f2, f2, f2
    ps_sum0 f12, f3, f3, f12
    ps_sub f4, f4, f4
    ps_cmpu0 cr1, f12, f4
    ble cr1, zero_length
    ps_rsqrte f12, f12
    ps_muls0 f0, f0, f12
    ps_muls0 f1, f1, f12
zero_length:
    ps_muls0 f2, f0, f9
    ps_muls0 f3, f8, f1
    ps_merge10 f8, f8, f8
    ps_mul f4, f0, f8
    ps_merge10 f5, f2, f3
    ps_merge10 f6, f3, f2
    ps_merge10 f7, f4, f4
    ps_sub f2, f5, f6
    ps_sub f3, f4, f7
    bge cr2, same_winding
    ps_neg f0, f0
    ps_neg f1, f1
same_winding:
    psq_st f0, 0(r6), 0, 6
    add r6, r6, r5
    psq_st f1, 0(r6), 1, 6
    add r6, r6, r4
    psq_st f2, 0(r6), 0, 6
    add r6, r6, r5
    psq_st f3, 0(r6), 1, 6
    blr
#endif // clang-format on
}
