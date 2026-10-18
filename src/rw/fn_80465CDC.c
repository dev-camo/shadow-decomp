/* RenderWare GameCube instanced data streams: finding the duplicate values of vertex streams,
 * building each stream's distinct values and remapping mesh indices into them, and the sizes of
 * vertex attributes in a vertex format. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <string.h>

/* the vertex format plugin ID, which owns the build allocations (inferred) */
#define rwID_VTXFMTPLUGIN 0x50D

/* The size of an element of a stream of the kind. The kinds are element layouts: 1 one byte, 2 two
 * bytes, 3 three bytes, 4 and 6 and 10 one word, 7 two words, 8 three words, 9 four words. */
static inline RwUInt32 StreamElementSize(RwInt32 kind) {
    switch (kind) {
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 6:
        return 4;
    case 7:
        return 8;
    case 8:
        return 12;
    case 9:
        return 16;
    case 10:
        return 4;
    default:
        return 0;
    }
}

/* Whether two elements of a stream of the kind are equal */
static inline RwBool StreamElementsEqual(RwInt32 kind, const void *a, const void *b) {
    const RwUInt8 *bytesA = a;
    const RwUInt8 *bytesB = b;
    const RwUInt32 *wordsA = a;
    const RwUInt32 *wordsB = b;
    RwBool equal;

    switch (kind) {
    case 1:
        equal = *bytesA == *bytesB;
        break;
    case 2:
        equal = *(const RwUInt16 *)a == *(const RwUInt16 *)b;
        break;
    case 3:
        equal = FALSE;
        if (bytesB[0] == bytesA[0] && bytesB[1] == bytesA[1] && bytesB[2] == bytesA[2]) {
            equal = TRUE;
        }
        break;
    case 4:
        equal = *wordsA == *wordsB;
        break;
    case 6:
        equal = *wordsA == *wordsB;
        break;
    case 7:
        equal = FALSE;
        if (wordsB[0] == wordsA[0] && wordsB[1] == wordsA[1]) {
            equal = TRUE;
        }
        break;
    case 8:
        equal = FALSE;
        if (wordsB[0] == wordsA[0] && wordsB[1] == wordsA[1] && wordsB[2] == wordsA[2]) {
            equal = TRUE;
        }
        break;
    case 9:
        equal = FALSE;
        if (wordsB[0] == wordsA[0] && wordsB[1] == wordsA[1] && wordsB[2] == wordsA[2] &&
            wordsB[3] == wordsA[3]) {
            equal = TRUE;
        }
        break;
    case 10:
        equal = *wordsA == *wordsB;
        break;
    default:
        equal = FALSE;
        break;
    }
    return equal;
}

/* Remaps each stream's mesh indices into the stream's distinct values: an index is replaced by
 * the earlier vertex it duplicates, less the duplicates before that vertex */
RxVertexIndex **fn_80465CDC(RpGameCubeStreamRemap *remaps, RxVertexIndex **indices,
                            RwInt32 numStreams, RwUInt32 numIndices) {
    RwUInt32 indicesSize = numIndices * sizeof(RxVertexIndex);
    RwUInt32 headerSize = numStreams * sizeof(RxVertexIndex *);
    RxVertexIndex **remapped =
        RwMalloc(headerSize + numStreams * indicesSize, rwMEMHINTDUR_EVENT | rwID_VTXFMTPLUGIN);
    RwUInt32 offset;
    RwUInt32 i;

    if (remapped == NULL) {
        return NULL;
    }
    memset(remapped, 0, numStreams * sizeof(RxVertexIndex *));
    offset = headerSize;
    for (i = 0; i < numStreams; i++) {
        remapped[i] = (RxVertexIndex *)((RwUInt8 *)remapped + offset);
        offset += indicesSize;
    }

    for (i = 0; i < numStreams; i++) {
        RwInt32 *remap = remaps[i].remap;
        RxVertexIndex *source = indices[i];
        RxVertexIndex *destination = remapped[i];
        RwUInt32 j;

        for (j = 0; j < numIndices; j++) {
            RxVertexIndex index = *source;
            RwUInt32 numDuplicates = 0;
            RxVertexIndex k;

            if (remap[index] != -1) {
                index = remap[index];
            }
            for (k = 0; k < index; k++) {
                if (remap[k] != -1) {
                    numDuplicates++;
                }
            }
            source++;
            *destination = index - numDuplicates;
            destination++;
        }
    }
    return remapped;
}

/* Builds each stream's distinct values, in vertex order */
RpGameCubeBuiltStream *fn_80465ED4(RpGameCubeStreamRemap *remaps, RpGameCubeStreamSource *streams,
                                   RwInt32 numStreams, RwInt32 numVertices) {
    RwUInt32 headerSize = numStreams * sizeof(RpGameCubeBuiltStream);
    RwUInt32 size = headerSize;
    RpGameCubeBuiltStream *built;
    RwUInt32 offset;
    RwUInt32 i;

    for (i = 0; i < numStreams; i++) {
        size += remaps[i].numValues * StreamElementSize(streams[i].kind);
    }
    built = RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_VTXFMTPLUGIN);
    if (built == NULL) {
        return NULL;
    }
    memset(built, 0, headerSize);

    offset = headerSize;
    for (i = 0; i < numStreams; i++) {
        built[i].data = (RwUInt8 *)built + offset;
        offset += remaps[i].numValues * StreamElementSize(streams[i].kind);
    }

    for (i = 0; i < numStreams; i++) {
        RwUInt32 elementSize = StreamElementSize(streams[i].kind);
        RwUInt32 j;
        RwUInt8 *source = streams[i].data;
        RwUInt8 *destination = built[i].data;

        for (j = 0; j < numVertices; j++) {
            if (remaps[i].remap[j] == -1) {
                memcpy(destination, source, elementSize);
                destination += elementSize;
            }
            source += elementSize;
        }
        built[i].count = remaps[i].numValues;
    }
    return built;
}

/* Finds the duplicate values of each stream: a vertex duplicates an earlier one when the stream
 * and each stream it depends on have equal values there. Each stream's remap gives the earlier
 * vertex, or -1 for a distinct value. */
RpGameCubeStreamRemap *fn_80466194(RpGameCubeStreamSource *streams, RwInt32 numStreams,
                                   RwInt32 numVertices) {
    RwUInt32 numEntries = numStreams * numVertices;
    RwUInt32 headerSize = numStreams * sizeof(RpGameCubeStreamRemap);
    RpGameCubeStreamRemap *remaps =
        RwMalloc(headerSize + numEntries * sizeof(RwInt32), rwMEMHINTDUR_EVENT | rwID_VTXFMTPLUGIN);
    RwUInt32 offset;
    RwUInt32 i;

    if (remaps == NULL) {
        return NULL;
    }
    memset(remaps, 0, headerSize);
    offset = headerSize;
    for (i = 0; i < numEntries; i++) {
        ((RwInt32 *)(remaps + numStreams))[i] = -1;
    }
    for (i = 0; i < numStreams; i++) {
        remaps[i].remap = (RwInt32 *)((RwUInt8 *)remaps + offset);
        offset += numVertices * sizeof(RwInt32);
        remaps[i].numValues = numVertices;
    }

    for (i = 0; i < numStreams; i++) {
        RpGameCubeStreamSource *stream = &streams[i];
        RpGameCubeStreamRemap *remap = &remaps[i];
        RwUInt32 elementSize = StreamElementSize(stream->kind);
        RwUInt32 vertex;

        for (vertex = 0; vertex < numVertices; vertex++) {
            RwUInt32 other;

            if (remap->remap[vertex] != -1) {
                continue;
            }
            for (other = vertex; other < numVertices; other++) {
                RwBool equal;
                RwInt32 j;
                RwInt8 dependent;
                RwUInt32 dependentSize;

                if (vertex == other) {
                    continue;
                }
                if (!StreamElementsEqual(stream->kind,
                                         (RwUInt8 *)stream->data + other * elementSize,
                                         (RwUInt8 *)stream->data + vertex * elementSize)) {
                    continue;
                }

                /* the vertex is a duplicate only if its dependent streams are equal too */
                equal = TRUE;
                j = 0;
                dependent = streams[i].dependencies[0];
                dependentSize = StreamElementSize(streams[dependent].kind);
                while (dependent >= 0) {
                    RpGameCubeStreamSource *dependentStream = &streams[dependent];

                    if (!StreamElementsEqual(
                            dependentStream->kind,
                            (RwUInt8 *)dependentStream->data + dependentSize * other,
                            (RwUInt8 *)dependentStream->data + dependentSize * vertex)) {
                        equal = FALSE;
                        break;
                    }
                    j++;
                    dependent = streams[i].dependencies[j];
                    dependentSize = StreamElementSize(streams[dependent].kind);
                }
                if (equal) {
                    remap->remap[other] = vertex;
                    remap->numValues--;
                }
            }
        }
    }
    return remaps;
}

/* The byte size of a vertex position in a vertex descriptor builder's format */
RwUInt32 fn_8046693C(const RpGameCubeVertexDescBuilder *builder) {
    RwUInt32 vatA = builder->vat[0];
    RwUInt32 count = vatA & 1;

    if (count == GX_POS_XYZ) {
        count = 3;
    } else if (count == GX_POS_XY) {
        count = 2;
    }
    switch ((RwInt32)((vatA >> 1) & 7)) {
    case GX_U8:
    case GX_S8:
        return count;
    case GX_U16:
    case GX_S16:
        return count * 2;
    case GX_F32:
        return count * 4;
    default:
        return 0;
    }
}

/* The byte size of a vertex normal in the format */
RwUInt32 fn_804669A0(const RpGameCubeVertexDescBuilder *builder) {
    switch ((RwInt32)((builder->vat[0] >> 10) & 7)) {
    case GX_S8:
        return 3;
    case GX_S16:
        return 6;
    case GX_F32:
        return 12;
    default:
        return 0;
    }
}

/* The byte size of a vertex colour of the colour channel in the format */
RwUInt32 fn_804669EC(const RpGameCubeVertexDescBuilder *builder, RwUInt8 channel) {
    switch ((RwInt32)((builder->vat[0] >> (14 + channel * 4)) & 7)) {
    case GX_RGB565:
    case GX_RGBA4:
        return 2;
    case GX_RGB8:
    case GX_RGBA6:
        return 3;
    case GX_RGBX8:
    case GX_RGBA8:
        return 4;
    default:
        return 0;
    }
}

/* The byte size of a vertex texture coordinate of the set in the format */
RwUInt32 fn_80466A54(const RpGameCubeVertexDescBuilder *builder, RwUInt8 set) {
    RwUInt32 count;
    RwInt32 type;

    switch (set) {
    case 0:
        count = (builder->vat[0] >> 21) & 1;
        type = (builder->vat[0] >> 22) & 7;
        break;
    case 1:
        count = builder->vat[1] & 1;
        type = (builder->vat[1] >> 1) & 7;
        break;
    case 2:
        count = (builder->vat[1] >> 9) & 1;
        type = (builder->vat[1] >> 10) & 7;
        break;
    case 3:
        count = (builder->vat[1] >> 18) & 1;
        type = (builder->vat[1] >> 19) & 7;
        break;
    case 4:
        count = (builder->vat[1] >> 27) & 1;
        type = (builder->vat[1] >> 28) & 7;
        break;
    case 5:
        count = (builder->vat[2] >> 5) & 1;
        type = (builder->vat[2] >> 6) & 7;
        break;
    case 6:
        count = (builder->vat[2] >> 14) & 1;
        type = (builder->vat[2] >> 15) & 7;
        break;
    case 7:
        count = (builder->vat[2] >> 23) & 1;
        type = (builder->vat[2] >> 24) & 7;
        break;
    default:
        return 0;
    }

    if (count == GX_TEX_ST) {
        count = 2;
    } else if (count == GX_TEX_S) {
        count = 1;
    }
    switch (type) {
    case GX_U8:
    case GX_S8:
        return count;
    case GX_U16:
    case GX_S16:
        return count * 2;
    case GX_F32:
        return count * 4;
    default:
        return 0;
    }
}
