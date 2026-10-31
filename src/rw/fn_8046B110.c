/* RenderWare GameCube instancing: building the instanced data of geometries and world sectors,
 * either optimized (each stream's distinct values indexed separately) or fast (every stream
 * indexed by vertex), with their GX vertex descriptors. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>
#include <string.h>

#define rwMEMHINT_INSTANCE (rwMEMHINTDUR_EVENT | rwID_WORLDPIPEMODULE)
#define rwMEMHINT_INSTANCETEMP (rwMEMHINTDUR_FUNCTION | rwID_WORLDPIPEMODULE)

/* The GX index type for a stream with count distinct values */
#define INDEXTYPE(count) ((count) >= 255 ? GX_INDEX16 : GX_INDEX8)

/* the vertex descriptors of geometries and world sectors */
RpGameCubeVertexDescBuilder lbl_805E18A0;
RpGameCubeVertexDescBuilder lbl_805E18C0;

/* Builds the distinct values of a geometry's vertex streams and each mesh's indices into them.
 * Returns TRUE on success. */
RwBool fn_8046B110(RpGeometry *geometry, RpGameCubeBuiltStream **builtStreams,
                   RxVertexIndex ****meshIndexLists) {
    RpGameCubeStreamSource streams[16];
    RpMorphTarget *morphTarget;
    RpGameCubeStreamRemap *remaps;
    RxVertexIndex **meshIndices;
    RwInt32 numStreams;
    RwUInt16 numMeshes;
    RwInt32 i;
    RwInt32 j;

    /* When the geometry has no vertex format the original asks for the default one and discards
     * the result, here and at the other sites in this file that call fn_80462E1C without using its
     * result */
    if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
        fn_80462E1C();
    }

    morphTarget = geometry->morphTarget;
    numStreams = 0;
    streams[numStreams].data = morphTarget->verts;
    streams[numStreams].kind = 8;
    streams[numStreams].dependencies[0] = -1;
    numStreams++;
    if (geometry->flags & rpGEOMETRYNORMALS) {
        streams[numStreams].data = morphTarget->normals;
        streams[numStreams].kind = 8;
        streams[numStreams].dependencies[0] = -1;
        numStreams++;
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

    remaps = fn_80466194(streams, numStreams, geometry->numVertices);
    if (remaps == NULL) {
        return FALSE;
    }
    *builtStreams = fn_80465ED4(remaps, streams, numStreams, geometry->numVertices);
    if (*builtStreams == NULL) {
        RwFree(remaps);
        return FALSE;
    }

    /* each mesh's indices, once per stream */
    numMeshes = geometry->mesh->numMeshes;
    meshIndices =
        RwMalloc(numStreams * (numMeshes * sizeof(RxVertexIndex *)), rwMEMHINT_INSTANCETEMP);
    if (meshIndices == NULL) {
        RwFree(*builtStreams);
        RwFree(remaps);
        return FALSE;
    }
    memset(meshIndices, 0, numStreams * (numMeshes * sizeof(RxVertexIndex *)));
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        for (j = 0; j < numStreams; j++) {
            meshIndices[i * numStreams + j] = mesh->indices;
        }
    }

    *meshIndexLists = RwMalloc(numMeshes * sizeof(RxVertexIndex **), rwMEMHINT_INSTANCETEMP);
    if (*meshIndexLists == NULL) {
        RwFree(meshIndices);
        RwFree(*builtStreams);
        RwFree(remaps);
        return FALSE;
    }
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        (*meshIndexLists)[i] =
            fn_80465CDC(remaps, &meshIndices[i * numStreams], numStreams, mesh->numIndices);
        if ((*meshIndexLists)[i] == NULL) {
            /* (sic) frees the earlier index lists but the first */
            while (--i > 0) {
                RwFree((*meshIndexLists)[i]);
            }
            return FALSE;
        }
    }

    RwFree(meshIndices);
    RwFree(remaps);
    return TRUE;
}

/* Describes a geometry's vertices to GX: positions, normals (or normals, binormals and tangents),
 * prelit colours and texture coordinates, each indexed into its distinct values */
RpGameCubeVertexDescBuilder *fn_8046B554(RpGeometry *geometry, RpGameCubeBuiltStream *counts) {
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E18A0);
    fn_8046FC48(&lbl_805E18A0, 0);
    fn_8046FC50(&lbl_805E18A0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    numAttributes = 0;
    fn_8046FF34(&lbl_805E18A0, GX_VA_POS, INDEXTYPE(counts[numAttributes].count));
    numAttributes++;

    if (geometry->flags & rpGEOMETRYNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E18A0, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E18A0, GX_VA_NBT, INDEXTYPE(counts[numAttributes].count));
        } else {
            fn_8046FC50(&lbl_805E18A0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E18A0, GX_VA_NRM, INDEXTYPE(counts[numAttributes].count));
        }
        numAttributes++;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        fn_8046FC50(&lbl_805E18A0, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E18A0, GX_VA_CLR0, INDEXTYPE(counts[numAttributes].count));
        numAttributes++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < geometry->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E18A0, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E18A0, GX_VA_TEX0 + i, INDEXTYPE(counts[numAttributes].count));
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E18A0, numAttributes);
    return &lbl_805E18A0;
}

/* Sets the vertex source of the built streams of an object with the flags and texture coordinate
 * sets */
static inline void BuiltStreamsVertexSource(RpGameCubeVertexSource *vertexSource,
                                            RpGameCubeBuiltStream *builtStreams, RwUInt32 flags,
                                            RwUInt8 numTexCoordSets,
                                            RpGameCubeVertexFormat *format) {
    RwInt32 stream;
    RwUInt32 i;

    if (format == NULL) {
        fn_80462E1C();
    }
    stream = 0;
    vertexSource->numElements[GX_VA_POS] = builtStreams[stream].count;
    vertexSource->data[GX_VA_POS] = builtStreams[stream].data;
    stream++;
    if (flags & rpGEOMETRYNORMALS) {
        vertexSource->numElements[GX_VA_NRM] = builtStreams[stream].count;
        vertexSource->data[GX_VA_NRM] = builtStreams[stream].data;
        stream++;
    }
    if (flags & rpGEOMETRYPRELIT) {
        vertexSource->numElements[GX_VA_CLR0] = builtStreams[stream].count;
        vertexSource->data[GX_VA_CLR0] = builtStreams[stream].data;
        stream++;
    }
    if (flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < numTexCoordSets; i++) {
            vertexSource->numElements[GX_VA_TEX0 + i] = builtStreams[stream].count;
            vertexSource->data[GX_VA_TEX0 + i] = builtStreams[stream].data;
            stream++;
        }
    }
}

/* Sets a mesh's attribute indices from its per-stream index lists */
static inline void MeshIndexSource(RpGameCubeIndexSource *indexSource, RxVertexIndex **lists,
                                   RwUInt32 flags, RwUInt8 numTexCoordSets,
                                   RpGameCubeVertexFormat *format) {
    RwInt32 list;
    RwUInt32 i;

    if (format == NULL) {
        fn_80462E1C();
    }
    list = 0;
    indexSource->indices[GX_VA_POS] = lists[list];
    list++;
    if (flags & rpGEOMETRYNORMALS) {
        indexSource->indices[GX_VA_NRM] = lists[list];
        list++;
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

/* Sets whether the instanced prelit colours have alpha: from the vertex format, or without one
 * from whether any vertex is translucent */
#define INSTANCEPRELITALPHA(instance, format, preLitLum, numVertices)                              \
    do {                                                                                           \
        if ((format) == NULL) {                                                                    \
            RwInt32 j_;                                                                            \
                                                                                                   \
            (instance)->flags &= ~rpGAMECUBEINSTANCEPRELITALPHA;                                   \
            for (j_ = 0; j_ < (numVertices); j_++) {                                               \
                if ((preLitLum)[j_].alpha < 255) {                                                 \
                    (instance)->flags |= rpGAMECUBEINSTANCEPRELITALPHA;                            \
                    break;                                                                         \
                }                                                                                  \
            }                                                                                      \
        } else if ((format)->colorType > GX_RGBX8) {                                               \
            (instance)->flags |= rpGAMECUBEINSTANCEPRELITALPHA;                                    \
        } else {                                                                                   \
            (instance)->flags &= ~rpGAMECUBEINSTANCEPRELITALPHA;                                   \
        }                                                                                          \
    } while (0)

/* Instances a geometry whose vertices are rebuilt as distinct values per stream: the vertex data
 * and a display list per mesh (triangle strips when the geometry has them) indexing each stream */
RwResEntry *fn_8046B79C(RpGeometry *geometry, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpGameCubeBuiltStream *builtStreams;
    RxVertexIndex ***meshIndexLists;
    RwInt32 numStrips;
    RwUInt32 numStripIndices;
    RpGameCubeVertexDescBuilder *builder;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 displayListsSize;
    RwUInt32 size;
    RwInt32 primitive;
    RwUInt32 i;

    fn_8046B110(geometry, &builtStreams, &meshIndexLists);
    builder = fn_8046B554(geometry, builtStreams);
    BuiltStreamsVertexSource(&vertexSource, builtStreams, geometry->flags,
                             geometry->numTexCoordSets, GAMECUBEVERTEXFORMAT(geometry));

    /* the vertex data header, the display list records, then the display lists */
    vertexDataSize = fn_8046EE8C(builder);
    displayListsSize = geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList);
    size = vertexDataSize + displayListsSize;
    /* room to align the display list data to 32 bytes */
    size += 31;
    primitive = (geometry->flags & rpGEOMETRYTRISTRIP) ? GX_TRIANGLESTRIP : GX_TRIANGLES;
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;

        if (geometry->flags & rpGEOMETRYTRISTRIP) {
            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &numStrips, &numStripIndices, 1);
            size += fn_8046B094(builder, numStrips, numStripIndices);
        } else {
            numStripIndices = mesh->numIndices;
            size += fn_8046B094(builder, 1, numStripIndices);
        }
    }
    size += fn_8046EEA0(builder, &vertexSource);

    repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_INSTANCE);
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
        INSTANCEPRELITALPHA(instance, GAMECUBEVERTEXFORMAT(geometry), geometry->preLitLum,
                            geometry->numVertices);
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList + displayListsSize + 31) & ~31);
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;
        RwUInt32 displayListSize;

        if (geometry->flags & rpGEOMETRYTRISTRIP) {
            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &numStrips, &numStripIndices, 1);
        } else {
            numStripIndices = mesh->numIndices;
            numStrips = 1;
        }
        displayListSize = fn_8046B094(builder, numStrips, numStripIndices);
        fn_8046B0E0(displayList, i, displayListSize, displayListData);

        MeshIndexSource(&indexSource, meshIndexLists[i], geometry->flags, geometry->numTexCoordSets,
                        GAMECUBEVERTEXFORMAT(geometry));
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices,
                    geometry->flags & rpGEOMETRYTRISTRIP, fn_8046ACBC(builder), 1, primitive, NULL);
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 0, NULL);
    DCFlushRange(instance, size);
    GXInvalidateVtxCache();

    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RwFree(meshIndexLists[i]);
    }
    RwFree(meshIndexLists);
    RwFree(builtStreams);
    return repEntry;
}

/* Describes a geometry's vertices to GX with every attribute indexed by vertex */
RpGameCubeVertexDescBuilder *fn_8046BEC4(RpGeometry *geometry) {
    RwUInt32 numVertices = geometry->numVertices; /* compared unsigned */
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E18A0);
    fn_8046FC48(&lbl_805E18A0, 0);
    fn_8046FC50(&lbl_805E18A0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    numAttributes = 0;
    fn_8046FF34(&lbl_805E18A0, GX_VA_POS, INDEXTYPE(numVertices));
    numAttributes++;

    if (geometry->flags & rpGEOMETRYNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E18A0, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E18A0, GX_VA_NBT, INDEXTYPE(numVertices));
        } else {
            fn_8046FC50(&lbl_805E18A0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E18A0, GX_VA_NRM, INDEXTYPE(numVertices));
        }
        numAttributes++;
    }
    if (geometry->flags & rpGEOMETRYPRELIT) {
        fn_8046FC50(&lbl_805E18A0, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E18A0, GX_VA_CLR0, INDEXTYPE(numVertices));
        numAttributes++;
    }
    if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        for (i = 0; i < geometry->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E18A0, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E18A0, GX_VA_TEX0 + i, INDEXTYPE(numVertices));
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E18A0, numAttributes);
    return &lbl_805E18A0;
}

/* Instances a geometry with every stream indexed by vertex: the vertex data and a display list
 * per mesh (triangle strips when the geometry has them). A geometry instanced for the platform
 * owns a separate allocation. */
RwResEntry *fn_8046C0D4(RpGeometry *geometry, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpGameCubeVertexDescBuilder *builder;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 displayListsSize;
    RwUInt32 size;
    RwInt32 primitive;
    RwUInt32 i;

    builder = fn_8046BEC4(geometry);
    if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
        fn_80462E1C();
    }
    {
        RpMorphTarget *morphTarget = geometry->morphTarget;
        RwInt32 set;

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
            for (set = 0; set < geometry->numTexCoordSets; set++) {
                vertexSource.numElements[GX_VA_TEX0 + set] = geometry->numVertices;
                vertexSource.data[GX_VA_TEX0 + set] = geometry->texCoords[set];
            }
        }
    }

    vertexDataSize = fn_8046EE8C(builder);
    displayListsSize = geometry->mesh->numMeshes * sizeof(RpGameCubeDisplayList);
    size = vertexDataSize + displayListsSize;
    /* room to align the display list data to 32 bytes */
    size += 31;
    primitive = (geometry->flags & rpGEOMETRYTRISTRIP) ? GX_TRIANGLESTRIP : GX_TRIANGLES;
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        size += fn_8046B094(builder, 1, ((RpMesh *)(geometry->mesh + 1))[i].numIndices);
    }
    size += fn_8046EEA0(builder, &vertexSource);

    if (geometry->flags & rpGEOMETRYNATIVEINSTANCE) {
        repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_INSTANCE);
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
    memset(instance, 0, size);
    instance->syncToken = lbl_805F1678;
    instance->serialNum = geometry->mesh->serialNum;
    instance->flags = 0;
    if (geometry->flags & rpGEOMETRYPRELIT) {
        INSTANCEPRELITALPHA(instance, GAMECUBEVERTEXFORMAT(geometry), geometry->preLitLum,
                            geometry->numVertices);
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList + displayListsSize + 31) & ~31);
    for (i = 0; i < geometry->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(geometry->mesh + 1) + i;
        RwUInt32 displayListSize = fn_8046B094(builder, 1, mesh->numIndices);
        RxVertexIndex *indices;
        RwInt32 set;

        fn_8046B0E0(displayList, i, displayListSize, displayListData);
        indices = mesh->indices;
        if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
            fn_80462E1C();
        }
        indexSource.indices[GX_VA_POS] = indices;
        if (geometry->flags & rpGEOMETRYNORMALS) {
            indexSource.indices[GX_VA_NRM] = indices;
        }
        if (geometry->flags & rpGEOMETRYPRELIT) {
            indexSource.indices[GX_VA_CLR0] = indices;
        }
        if (geometry->flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
            for (set = 0; set < geometry->numTexCoordSets; set++) {
                indexSource.indices[GX_VA_TEX0 + set] = indices;
            }
        }
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices, 0, fn_8046ACBC(builder),
                    1, primitive, NULL);
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 0, NULL);
    DCFlushRange(instance, size);
    GXInvalidateVtxCache();
    return repEntry;
}

/* Builds the distinct values of a world sector's vertex streams and each mesh's indices into
 * them. The sector's normals are compared as words, so their padding is cleared first. Returns
 * TRUE on success. */
RwBool fn_8046C4D0(RpWorld *world, RpWorldSector *sector, RpGameCubeBuiltStream **builtStreams,
                   RxVertexIndex ****meshIndexLists) {
    RpGameCubeStreamSource streams[16];
    RpGameCubeStreamRemap *remaps;
    RxVertexIndex **meshIndices;
    RwInt32 numStreams;
    RwUInt16 numMeshes;
    RwInt32 i;
    RwInt32 j;

    if (GAMECUBEWORLDVERTEXFORMAT(world) == NULL) {
        fn_80462E1C();
    }

    numStreams = 0;
    streams[numStreams].data = sector->vertices;
    streams[numStreams].kind = 8;
    streams[numStreams].dependencies[0] = -1;
    numStreams++;
    if (world->flags & rpWORLDNORMALS) {
        for (i = 0; i < sector->numVertices; i++) {
            sector->normals[i].pad = 0;
        }
        streams[numStreams].data = sector->normals;
        streams[numStreams].kind = 4;
        streams[numStreams].dependencies[0] = -1;
        numStreams++;
    }
    if (world->flags & rpWORLDPRELIT) {
        streams[numStreams].data = sector->preLitLum;
        streams[numStreams].kind = 4;
        streams[numStreams].dependencies[0] = -1;
        numStreams++;
    }
    if (world->flags & (rpWORLDTEXTURED | rpWORLDTEXTURED2)) {
        for (i = 0; i < world->numTexCoordSets; i++) {
            streams[numStreams].data = sector->texCoords[i];
            streams[numStreams].kind = 7;
            streams[numStreams].dependencies[0] = -1;
            numStreams++;
        }
    }

    remaps = fn_80466194(streams, numStreams, sector->numVertices);
    if (remaps == NULL) {
        return FALSE;
    }
    *builtStreams = fn_80465ED4(remaps, streams, numStreams, sector->numVertices);
    if (*builtStreams == NULL) {
        RwFree(remaps);
        return FALSE;
    }

    numMeshes = sector->mesh->numMeshes;
    meshIndices =
        RwMalloc(numStreams * (numMeshes * sizeof(RxVertexIndex *)), rwMEMHINT_INSTANCETEMP);
    if (meshIndices == NULL) {
        RwFree(*builtStreams);
        RwFree(remaps);
        return FALSE;
    }
    memset(meshIndices, 0, numStreams * (numMeshes * sizeof(RxVertexIndex *)));
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(sector->mesh + 1) + i;

        for (j = 0; j < numStreams; j++) {
            meshIndices[i * numStreams + j] = mesh->indices;
        }
    }

    *meshIndexLists = RwMalloc(numMeshes * sizeof(RxVertexIndex **), rwMEMHINT_INSTANCETEMP);
    if (*meshIndexLists == NULL) {
        RwFree(meshIndices);
        RwFree(*builtStreams);
        RwFree(remaps);
        return FALSE;
    }
    for (i = 0; i < numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(sector->mesh + 1) + i;

        (*meshIndexLists)[i] =
            fn_80465CDC(remaps, &meshIndices[i * numStreams], numStreams, mesh->numIndices);
        if ((*meshIndexLists)[i] == NULL) {
            /* (sic) frees the earlier index lists but the first */
            while (--i > 0) {
                RwFree((*meshIndexLists)[i]);
            }
            return FALSE;
        }
    }

    RwFree(meshIndices);
    RwFree(remaps);
    return TRUE;
}

/* Describes a world sector's vertices to GX, each attribute indexed into its distinct values */
RpGameCubeVertexDescBuilder *fn_8046C94C(RpWorld *world, RpGameCubeBuiltStream *counts) {
    RpGameCubeVertexFormat *format = GAMECUBEWORLDVERTEXFORMAT(world);
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E18C0);
    fn_8046FC48(&lbl_805E18C0, 0);
    fn_8046FC50(&lbl_805E18C0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    numAttributes = 0;
    fn_8046FF34(&lbl_805E18C0, GX_VA_POS, INDEXTYPE(counts[numAttributes].count));
    numAttributes++;

    if (world->flags & rpWORLDNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E18C0, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E18C0, GX_VA_NBT, INDEXTYPE(counts[numAttributes].count));
        } else {
            fn_8046FC50(&lbl_805E18C0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E18C0, GX_VA_NRM, INDEXTYPE(counts[numAttributes].count));
        }
        numAttributes++;
    }
    if (world->flags & rpWORLDPRELIT) {
        fn_8046FC50(&lbl_805E18C0, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E18C0, GX_VA_CLR0, INDEXTYPE(counts[numAttributes].count));
        numAttributes++;
    }
    if (world->flags & (rpWORLDTEXTURED | rpWORLDTEXTURED2)) {
        for (i = 0; i < world->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E18C0, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E18C0, GX_VA_TEX0 + i, INDEXTYPE(counts[numAttributes].count));
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E18C0, numAttributes);
    return &lbl_805E18C0;
}

/* Instances a world sector whose vertices are rebuilt as distinct values per stream, with its
 * positions relative to its box's corner */
RwResEntry *fn_8046CB94(RpWorld *world, RpWorldSector *sector, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpGameCubeBuiltStream *builtStreams;
    RxVertexIndex ***meshIndexLists;
    RwInt32 numStrips;
    RwUInt32 numStripIndices;
    RpGameCubeVertexDescBuilder *builder;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 displayListsSize;
    RwUInt32 size;
    RwInt32 primitive;
    RwUInt32 i;

    fn_8046C4D0(world, sector, &builtStreams, &meshIndexLists);
    builder = fn_8046C94C(world, builtStreams);
    BuiltStreamsVertexSource(&vertexSource, builtStreams, world->flags, world->numTexCoordSets,
                             GAMECUBEWORLDVERTEXFORMAT(world));

    vertexDataSize = fn_8046EE8C(builder);
    displayListsSize = sector->mesh->numMeshes * sizeof(RpGameCubeDisplayList);
    size = vertexDataSize + displayListsSize;
    /* room to align the display list data to 32 bytes */
    size += 31;
    primitive = (world->flags & rpWORLDTRISTRIP) ? GX_TRIANGLESTRIP : GX_TRIANGLES;
    for (i = 0; i < sector->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(sector->mesh + 1) + i;

        if (world->flags & rpWORLDTRISTRIP) {
            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &numStrips, &numStripIndices, 1);
            size += fn_8046B094(builder, numStrips, numStripIndices);
        } else {
            size += fn_8046B094(builder, 1, mesh->numIndices);
        }
    }
    size += fn_8046EEA0(builder, &vertexSource);

    repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_INSTANCE);
    repEntry->link.next = NULL;
    repEntry->link.prev = NULL;
    repEntry->owner = sector;
    repEntry->size = size;
    repEntry->ownerRef = &sector->repEntry;
    repEntry->destroyNotify = fn_8046288C;
    sector->repEntry = repEntry;

    instance = (RpGameCubeInstanceData *)(repEntry + 1);
    memset(instance, 0, size);
    instance->syncToken = lbl_805F1678;
    instance->serialNum = sector->mesh->serialNum;
    instance->flags = 0;
    if (world->flags & rpWORLDPRELIT) {
        INSTANCEPRELITALPHA(instance, GAMECUBEWORLDVERTEXFORMAT(world), sector->preLitLum,
                            sector->numVertices);
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList + displayListsSize + 31) & ~31);
    for (i = 0; i < sector->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(sector->mesh + 1) + i;
        RwUInt32 displayListSize;

        if (world->flags & rpWORLDTRISTRIP) {
            fn_8046D8C8(meshIndexLists[i][0], mesh->numIndices, &numStrips, &numStripIndices, 1);
        } else {
            numStripIndices = mesh->numIndices;
            numStrips = 1;
        }
        displayListSize = fn_8046B094(builder, numStrips, numStripIndices);
        fn_8046B0E0(displayList, i, displayListSize, displayListData);

        MeshIndexSource(&indexSource, meshIndexLists[i], world->flags, world->numTexCoordSets,
                        GAMECUBEWORLDVERTEXFORMAT(world));
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices,
                    world->flags & rpWORLDTRISTRIP, fn_8046ACBC(builder), 1, primitive,
                    &sector->boundingBox.inf);
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 1, &sector->boundingBox.inf);
    DCFlushRange(instance, size);
    GXInvalidateVtxCache();

    for (i = 0; i < sector->mesh->numMeshes; i++) {
        RwFree(meshIndexLists[i]);
    }
    RwFree(meshIndexLists);
    RwFree(builtStreams);
    return repEntry;
}

/* Describes a world sector's vertices to GX with every attribute indexed by vertex */
RpGameCubeVertexDescBuilder *fn_8046D2B8(RpWorld *world, RpWorldSector *sector) {
    RwUInt32 numVertices = sector->numVertices; /* compared unsigned */
    RpGameCubeVertexFormat *format = GAMECUBEWORLDVERTEXFORMAT(world);
    RwInt32 numAttributes;
    RwInt32 i;

    if (format == NULL) {
        format = fn_80462E1C();
    }

    fn_8046FC1C(&lbl_805E18C0);
    fn_8046FC48(&lbl_805E18C0, 0);
    fn_8046FC50(&lbl_805E18C0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    numAttributes = 0;
    fn_8046FF34(&lbl_805E18C0, GX_VA_POS, INDEXTYPE(numVertices));
    numAttributes++;

    if (world->flags & rpWORLDNORMALS) {
        if (format->normalNbt != 0) {
            fn_8046FC50(&lbl_805E18C0, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            fn_8046FF34(&lbl_805E18C0, GX_VA_NBT, INDEXTYPE(numVertices));
        } else {
            fn_8046FC50(&lbl_805E18C0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            fn_8046FF34(&lbl_805E18C0, GX_VA_NRM, INDEXTYPE(numVertices));
        }
        numAttributes++;
    }
    if (world->flags & rpWORLDPRELIT) {
        fn_8046FC50(&lbl_805E18C0, GX_VA_CLR0,
                    format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType, 0);
        fn_8046FF34(&lbl_805E18C0, GX_VA_CLR0, INDEXTYPE(numVertices));
        numAttributes++;
    }
    if (world->flags & (rpWORLDTEXTURED | rpWORLDTEXTURED2)) {
        for (i = 0; i < world->numTexCoordSets; i++) {
            fn_8046FC50(&lbl_805E18C0, GX_VA_TEX0 + i, GX_TEX_ST, format->texCoordType[i],
                        format->texCoordFrac[i]);
            fn_8046FF34(&lbl_805E18C0, GX_VA_TEX0 + i, INDEXTYPE(numVertices));
            numAttributes++;
        }
    }

    fn_8047046C(&lbl_805E18C0, numAttributes);
    return &lbl_805E18C0;
}

/* Instances a world sector with every stream indexed by vertex, with its positions relative to its
 * box's corner. A world instanced for the platform gives the sector a separate allocation. */
RwResEntry *fn_8046D4C8(RpWorld *world, RpWorldSector *sector, void *owner, RwResEntry **ownerRef) {
    RpGameCubeIndexSource indexSource;
    RpGameCubeVertexSource vertexSource;
    RpGameCubeVertexDescBuilder *builder;
    RwResEntry *repEntry;
    RpGameCubeInstanceData *instance;
    RwUInt8 *displayList;
    RwUInt8 *displayListData;
    RwUInt32 vertexDataSize;
    RwUInt32 displayListsSize;
    RwUInt32 size;
    RwInt32 primitive;
    RwUInt32 i;

    builder = fn_8046D2B8(world, sector);
    if (GAMECUBEWORLDVERTEXFORMAT(world) == NULL) {
        fn_80462E1C();
    }
    {
        RwInt32 set;

        vertexSource.numElements[GX_VA_POS] = sector->numVertices;
        vertexSource.data[GX_VA_POS] = sector->vertices;
        if (world->flags & rpWORLDNORMALS) {
            vertexSource.numElements[GX_VA_NRM] = sector->numVertices;
            vertexSource.data[GX_VA_NRM] = sector->normals;
        }
        if (world->flags & rpWORLDPRELIT) {
            vertexSource.numElements[GX_VA_CLR0] = sector->numVertices;
            vertexSource.data[GX_VA_CLR0] = sector->preLitLum;
        }
        if (world->flags & (rpWORLDTEXTURED | rpWORLDTEXTURED2)) {
            for (set = 0; set < world->numTexCoordSets; set++) {
                vertexSource.numElements[GX_VA_TEX0 + set] = sector->numVertices;
                vertexSource.data[GX_VA_TEX0 + set] = sector->texCoords[set];
            }
        }
    }

    vertexDataSize = fn_8046EE8C(builder);
    displayListsSize = sector->mesh->numMeshes * sizeof(RpGameCubeDisplayList);
    size = vertexDataSize + displayListsSize;
    /* room to align the display list data to 32 bytes */
    size += 31;
    primitive = (world->flags & rpWORLDTRISTRIP) ? GX_TRIANGLESTRIP : GX_TRIANGLES;
    for (i = 0; i < sector->mesh->numMeshes; i++) {
        size += fn_8046B094(builder, 1, ((RpMesh *)(sector->mesh + 1))[i].numIndices);
    }
    size += fn_8046EEA0(builder, &vertexSource);

    if (world->flags & rpWORLDNATIVEINSTANCE) {
        repEntry = RwMalloc(size + sizeof(RwResEntry), rwMEMHINT_INSTANCE);
        repEntry->link.next = NULL;
        repEntry->link.prev = NULL;
        repEntry->owner = sector;
        repEntry->size = size;
        repEntry->ownerRef = &sector->repEntry;
        repEntry->destroyNotify = fn_8046288C;
        sector->repEntry = repEntry;
    } else {
        repEntry = fn_80480878(owner, ownerRef, size, fn_8046288C);
    }

    instance = (RpGameCubeInstanceData *)(repEntry + 1);
    memset(instance, 0, size);
    instance->syncToken = lbl_805F1678;
    instance->serialNum = sector->mesh->serialNum;
    instance->flags = 0;
    if (world->flags & rpWORLDPRELIT) {
        INSTANCEPRELITALPHA(instance, GAMECUBEWORLDVERTEXFORMAT(world), sector->preLitLum,
                            sector->numVertices);
    }

    displayList = (RwUInt8 *)instance + vertexDataSize;
    displayListData = (RwUInt8 *)(((RwUInt32)displayList + displayListsSize + 31) & ~31);
    for (i = 0; i < sector->mesh->numMeshes; i++) {
        RpMesh *mesh = (RpMesh *)(sector->mesh + 1) + i;
        RwUInt32 displayListSize = fn_8046B094(builder, 1, mesh->numIndices);
        RxVertexIndex *indices;
        RwInt32 set;

        fn_8046B0E0(displayList, i, displayListSize, displayListData);
        indices = mesh->indices;
        if (GAMECUBEWORLDVERTEXFORMAT(world) == NULL) {
            fn_80462E1C();
        }
        indexSource.indices[GX_VA_POS] = indices;
        if (world->flags & rpWORLDNORMALS) {
            indexSource.indices[GX_VA_NRM] = indices;
        }
        if (world->flags & rpWORLDPRELIT) {
            indexSource.indices[GX_VA_CLR0] = indices;
        }
        if (world->flags & (rpWORLDTEXTURED | rpWORLDTEXTURED2)) {
            for (set = 0; set < world->numTexCoordSets; set++) {
                indexSource.indices[GX_VA_TEX0 + set] = indices;
            }
        }
        fn_8046E594(builder, displayList, &indexSource, mesh->numIndices, 0, fn_8046ACBC(builder),
                    1, primitive, &sector->boundingBox.inf);
        displayListData += displayListSize;
        displayList += sizeof(RpGameCubeDisplayList);
    }

    fn_8046F208(builder, instance, &vertexSource, displayListData);
    fn_8046F6CC(builder, instance, &vertexSource, 1, &sector->boundingBox.inf);
    DCFlushRange(instance, size);
    GXInvalidateVtxCache();
    return repEntry;
}
