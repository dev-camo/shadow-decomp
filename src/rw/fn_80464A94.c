/* RenderWare GameCube native data: reading and writing the instanced data of geometries and world
 * sectors as a stream chunk, with its pointers stored as offsets into the vertex data. The original
 * file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>

/* The size of a resource entry's native data chunk; the original subtracts 7 from the entry size,
 * whose meaning is not established */
#define NATIVEDATASIZE(repEntry) ((repEntry)->size - 7)

/* Reports an error of the native data; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_WORLDPLUGIN;                                                        \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* Reads an object's instanced data from its native data chunk into a new resource entry: the
 * header, then the vertex data aligned to 32 bytes, whose offsets are turned back into pointers.
 * Returns the object, or NULL on failure. */
void *fn_80464A94(RwStream *stream, void *object, RwResEntry **repEntryRef, RwUInt32 numMeshes) {
    RwUInt32 version;
    RwUInt32 length;
    RwUInt32 headerSize;
    RwUInt32 vertexSize;
    RwInt32 platform;
    RpGameCubeInstanceData *instance;
    RpGameCubeDisplayList *displayLists;
    RwUInt8 *base;
    RwUInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    if (fn_8047D868(stream, &platform, sizeof(platform)) == NULL) {
        return NULL;
    }
    if (platform != rwID_GAMECUBE) {
        return NULL;
    }
    if (fn_8047D868(stream, (RwInt32 *)&headerSize, sizeof(headerSize)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, (RwInt32 *)&vertexSize, sizeof(vertexSize)) == NULL) {
        return NULL;
    }

    *repEntryRef = RwMalloc(sizeof(RwResEntry) + headerSize + vertexSize + 31,
                            rwMEMHINTDUR_EVENT | rwID_WORLDPIPEMODULE);
    instance = (RpGameCubeInstanceData *)(*repEntryRef + 1);
    if (fn_80480D4C(stream, instance, headerSize) != headerSize) {
        return NULL;
    }
    if (fn_80480D4C(stream, (void *)(((RwUInt32)instance + headerSize + 31) & ~31), vertexSize) !=
        vertexSize) {
        return NULL;
    }

    displayLists = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);
    base = (RwUInt8 *)(((RwUInt32)(displayLists + numMeshes) + 31) & ~31);
    for (i = 0; i < instance->numVertexDescriptors; i++) {
        instance->vertexDescriptors[i].data = base + (RwUInt32)instance->vertexDescriptors[i].data;
    }
    for (i = 0; i < numMeshes; i++) {
        displayLists[i].data = base + (RwUInt32)displayLists[i].data;
    }

    (*repEntryRef)->link.next = NULL;
    (*repEntryRef)->link.prev = NULL;
    (*repEntryRef)->owner = object;
    (*repEntryRef)->size = length;
    (*repEntryRef)->ownerRef = repEntryRef;
    (*repEntryRef)->destroyNotify = fn_8046288C;
    instance->syncToken = lbl_805EDF40;
    DCFlushRange(*repEntryRef + 1, (*repEntryRef)->size);
    GXInvalidateVtxCache();
    return object;
}

/* Writes a resource entry's instanced data as a native data chunk, with its pointers turned into
 * offsets into the vertex data while it is written */
RwStream *fn_80464DAC(RwStream *stream, RwResEntry *repEntry, RwUInt32 numMeshes) {
    RwUInt32 base;
    RwInt32 platform = rwID_GAMECUBE;
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(repEntry + 1);
    RpGameCubeDisplayList *displayLists = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);
    RwUInt32 headerSize;
    RwUInt32 vertexSize;
    RwUInt32 i;

    if (fn_8047CEF0(stream, rwID_STRUCT, NATIVEDATASIZE(repEntry) - rwCHUNKHEADERSIZE,
                    RW_STREAM_VERSION, RW_STREAM_BUILD) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, &platform, sizeof(platform)) == NULL) {
        return NULL;
    }
    headerSize = sizeof(RpGameCubeInstanceData) +
                 (instance->numVertexDescriptors - 1) * sizeof(RpGameCubeVertexDescriptor) +
                 numMeshes * sizeof(RpGameCubeDisplayList);
    if (fn_8047D458(stream, (RwInt32 *)&headerSize, sizeof(headerSize)) == NULL) {
        return NULL;
    }
    vertexSize = repEntry->size - headerSize - 31;
    if (fn_8047D458(stream, (RwInt32 *)&vertexSize, sizeof(vertexSize)) == NULL) {
        return NULL;
    }

    base = ((RwUInt32)(displayLists + numMeshes) + 31) & ~31;
    for (i = 0; i < instance->numVertexDescriptors; i++) {
        instance->vertexDescriptors[i].data =
            (void *)((RwUInt32)instance->vertexDescriptors[i].data - base);
    }
    for (i = 0; i < numMeshes; i++) {
        displayLists[i].data = (void *)((RwUInt32)displayLists[i].data - base);
    }
    if (fn_80480ED0(stream, instance, headerSize) == NULL) {
        return NULL;
    }
    for (i = 0; i < instance->numVertexDescriptors; i++) {
        instance->vertexDescriptors[i].data =
            (void *)(base + (RwUInt32)instance->vertexDescriptors[i].data);
    }
    for (i = 0; i < numMeshes; i++) {
        displayLists[i].data = (void *)(base + (RwUInt32)displayLists[i].data);
    }
    if (fn_80480ED0(stream, displayLists->data, vertexSize) == NULL) {
        return NULL;
    }
    DCFlushRange(repEntry + 1, repEntry->size);
    GXInvalidateVtxCache();
    return stream;
}

/* The native data of a geometry: its size, writing and reading */
RwInt32 fn_8046510C(const RpGeometry *geometry) {
    if ((geometry->flags & rpGEOMETRYNATIVE) && geometry->repEntry != NULL) {
        return NATIVEDATASIZE(geometry->repEntry);
    }
    return 0;
}

RwStream *fn_80465138(RwStream *stream, const RpGeometry *geometry) {
    if (geometry->flags & rpGEOMETRYNATIVE) {
        fn_80464DAC(stream, geometry->repEntry, geometry->mesh->numMeshes);
    }
    return stream;
}

RpGeometry *fn_80465180(RwStream *stream, RpGeometry *geometry) {
    return fn_80464A94(stream, geometry, &geometry->repEntry, geometry->mesh->numMeshes);
}

/* The native data of a world sector: its size, writing and reading */
RwInt32 fn_804651AC(const RpWorldSector *sector) {
    if ((fn_8045FEA0((RpWorldSector *)sector)->flags & rpWORLDNATIVE) && sector->repEntry != NULL) {
        return NATIVEDATASIZE(sector->repEntry);
    }
    return 0;
}

RwStream *fn_80465200(RwStream *stream, const RpWorldSector *sector) {
    if (fn_8045FEA0((RpWorldSector *)sector)->flags & rpWORLDNATIVE) {
        fn_80464DAC(stream, sector->repEntry, sector->mesh->numMeshes);
    }
    return stream;
}

RpWorldSector *fn_80465260(RwStream *stream, RpWorldSector *sector) {
    return fn_80464A94(stream, sector, &sector->repEntry, sector->mesh->numMeshes);
}
