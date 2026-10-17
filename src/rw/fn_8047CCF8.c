/* RenderWare stream chunks and byte order: reading, writing and finding chunk headers, converting
 * memory between the native and the little-endian stream byte order, and reading and writing
 * converted reals and integers. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>
#include "layout_check.h"

/* The core's error plugin ID (inferred) */
#define rwCOREERRORPLUGIN 1

/* The newest stream library version read */
#define rwLIBRARYCURRENTVERSION 0x37002

/* The bytes converted at a time when writing */
#define rwSTREAMCONVERTBUFFER 0x100

/* The chunk types (only those that hold other chunks are named here) */
#define rwID_CAMERA 0x5
#define rwID_MATERIAL 0x7
#define rwID_MATRIX 0xD
#define rwID_FRAMELIST 0xE
#define rwID_GEOMETRY 0xF
#define rwID_CLUMP 0x10
#define rwID_LIGHT 0x12
#define rwID_UNICODESTRING 0x13
#define rwID_ATOMIC 0x14
#define rwID_GEOMETRYLIST 0x1A
#define rwID_STRING 0x2

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* A chunk header as it is stored in a stream */
typedef struct rwChunkHeader {
    RwUInt32 type;
    RwUInt32 length;
    RwUInt32 libraryID; /* the version and build number */
} rwChunkHeader;
CHECK_SIZEOF(rwChunkHeader, 0xC);

RwBool fn_8047CCF8(const RwChunkHeaderInfo *chunkHeaderInfo) {
    switch (chunkHeaderInfo->type) {
    case rwID_STRUCT:
        return FALSE;
    case rwID_STRING:
        return FALSE;
    case rwID_EXTENSION:
        return FALSE;
    case rwID_CAMERA:
        return TRUE;
    case rwID_TEXTURE:
        return TRUE;
    case rwID_MATERIAL:
        return TRUE;
    case rwID_MATLIST:
        return TRUE;
    case rwID_ATOMICSECT:
        return TRUE;
    case rwID_PLANESECT:
        return TRUE;
    case rwID_WORLD:
        return TRUE;
    case rwID_MATRIX:
        return FALSE;
    case rwID_FRAMELIST:
        return TRUE;
    case rwID_GEOMETRY:
        return TRUE;
    case rwID_CLUMP:
        return TRUE;
    case rwID_LIGHT:
        return TRUE;
    case rwID_UNICODESTRING:
        return FALSE;
    case rwID_ATOMIC:
        return TRUE;
    case rwID_GEOMETRYLIST:
        return TRUE;
    default:
        return FALSE;
    }
}

/* Decodes a stored chunk header's library ID into its version and build number */
#define CHUNKHEADERDECODE(header, info)                                                            \
    do {                                                                                           \
        (info).type = (header).type;                                                               \
        (info).length = (header).length;                                                           \
        if (!((header).libraryID & 0xFFFF0000)) {                                                  \
            /* an old library ID is the version alone */                                           \
            (info).version = (header).libraryID << 8;                                              \
            (info).buildNum = 0;                                                                   \
        } else {                                                                                   \
            (info).buildNum = (header).libraryID & 0xFFFF;                                         \
            (info).version = ((((header).libraryID >> 14) & 0x3FF00) + 0x30000) |                  \
                             (((header).libraryID >> 16) & 0x3F);                                  \
        }                                                                                          \
        (info).isComplex = fn_8047CCF8(&(info));                                                   \
    } while (0)

RwBool fn_8047CDB4(RwStream *stream, RwUInt32 *type, RwUInt32 *length, RwUInt32 *version,
                   RwUInt32 *buildNum) {
    rwChunkHeader header;
    RwChunkHeaderInfo info;

    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        RWERROR((E_RW_READ));
        return FALSE;
    }
    fn_8047D254(&header, sizeof(header));
    CHUNKHEADERDECODE(header, info);
    if (type != NULL) {
        *type = info.type;
    }
    if (length != NULL) {
        *length = info.length;
    }
    if (buildNum != NULL) {
        *buildNum = info.buildNum;
    }
    if (version != NULL) {
        *version = info.version;
    }
    return TRUE;
}

RwStream *fn_8047CEF0(RwStream *stream, RwInt32 type, RwInt32 size, RwUInt32 version,
                      RwUInt32 build) {
    rwChunkHeader header;

    header.type = type;
    header.length = size;
    /* the version's 0x3XXYY digits go to the top ten bits, below them its low six bits, and the
     * build number fills the low half */
    header.libraryID =
        (((version - 0x30000) << 14) & 0xFFC00000) | ((version & 0x3F) << 16) | (build & 0xFFFF);
    fn_8047D158(&header, sizeof(header));
    return fn_80480ED0(stream, &header, sizeof(header));
}

/* Finds the next chunk of a type, skipping others; refuses chunks of unsupported versions */
RwInt32 fn_8047CF50(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut) {
    RwUInt32 readType;
    RwUInt32 length;
    RwUInt32 version;

    while (fn_8047CDB4(stream, &readType, &length, &version, NULL)) {
        if (readType == type) {
            if (version < rwLIBRARYBASEVERSION) {
                RWERROR((E_RW_BADVERSION));
                return FALSE;
            }
            if (version > rwLIBRARYCURRENTVERSION) {
                RWERROR((E_RW_BADVERSION));
                return FALSE;
            }
            if (lengthOut != NULL) {
                *lengthOut = length;
            }
            if (versionOut != NULL) {
                *versionOut = version;
            }
            return TRUE;
        }
        if (fn_804810FC(stream, length) == NULL) {
            return FALSE;
        }
    }
    return FALSE;
}

/* Converts 32-bit words between the native byte order and the stream's little-endian order */
void *fn_8047D158(void *mem, RwUInt32 size) {
    RwUInt32 count;
    RwUInt32 *word = mem;

    for (count = size >> 2; count != 0; count--) {
        __stwbrx(*word, word, 0);
        word++;
    }
    return mem;
}

/* Converts 16-bit words between the native byte order and the stream's little-endian order */
void *fn_8047D1C8(void *mem, RwUInt32 size) {
    RwUInt32 count;
    RwUInt16 *word = mem;

    for (count = size >> 1; count != 0; count--) {
        RwUInt16 value = *word;

        *word = (value >> 8) | (value << 8);
        word++;
    }
    return mem;
}

void *fn_8047D254(void *mem, RwUInt32 size) {
    RwUInt32 count;
    RwUInt32 *word = mem;

    for (count = size >> 2; count != 0; count--) {
        __stwbrx(*word, word, 0);
        word++;
    }
    return mem;
}

void *fn_8047D2C4(void *mem, RwUInt32 size) {
    RwUInt32 count;
    RwUInt16 *word = mem;

    for (count = size >> 1; count != 0; count--) {
        RwUInt16 value = *word;

        *word = (value >> 8) | (value << 8);
        word++;
    }
    return mem;
}

RwStream *fn_8047D350(RwStream *stream, const RwReal *reals, RwUInt32 size) {
    RwUInt8 buffer[rwSTREAMCONVERTBUFFER];

    while (size != 0) {
        RwUInt32 bytes = (size < rwSTREAMCONVERTBUFFER) ? size : rwSTREAMCONVERTBUFFER;

        memcpy(buffer, reals, bytes);
        fn_8047D158(buffer, bytes);
        if (fn_80480ED0(stream, buffer, bytes) == NULL) {
            return NULL;
        }
        size -= bytes;
        /* on to the next bytes */
        reals = (const RwReal *)((const RwUInt8 *)reals + bytes);
    }
    return stream;
}

RwStream *fn_8047D458(RwStream *stream, const RwInt32 *ints, RwUInt32 size) {
    RwUInt8 buffer[rwSTREAMCONVERTBUFFER];

    while (size != 0) {
        RwUInt32 bytes = (size < rwSTREAMCONVERTBUFFER) ? size : rwSTREAMCONVERTBUFFER;

        memcpy(buffer, ints, bytes);
        fn_8047D158(buffer, bytes);
        if (fn_80480ED0(stream, buffer, bytes) == NULL) {
            return NULL;
        }
        size -= bytes;
        /* on to the next bytes */
        ints = (const RwInt32 *)((const RwUInt8 *)ints + bytes);
    }
    return stream;
}

RwStream *fn_8047D560(RwStream *stream, const RwInt16 *ints, RwUInt32 size) {
    RwUInt8 buffer[rwSTREAMCONVERTBUFFER];

    while (size != 0) {
        RwUInt32 bytes = (size < rwSTREAMCONVERTBUFFER) ? size : rwSTREAMCONVERTBUFFER;

        memcpy(buffer, ints, bytes);
        fn_8047D1C8(buffer, bytes);
        if (fn_80480ED0(stream, buffer, bytes) == NULL) {
            return NULL;
        }
        size -= bytes;
        /* on to the next bytes */
        ints = (const RwInt16 *)((const RwUInt8 *)ints + bytes);
    }
    return stream;
}

RwStream *fn_8047D684(RwStream *stream, RwInt16 *ints, RwUInt32 size) {
    if (fn_80480D4C(stream, ints, size) == 0) {
        RWERROR((E_RW_READ));
        return NULL;
    }
    fn_8047D2C4(ints, size);
    return stream;
}

RwStream *fn_8047D784(RwStream *stream, RwReal *reals, RwUInt32 size) {
    if (fn_80480D4C(stream, reals, size) == 0) {
        RWERROR((E_RW_READ));
        return NULL;
    }
    fn_8047D254(reals, size);
    return stream;
}

RwStream *fn_8047D868(RwStream *stream, RwInt32 *ints, RwUInt32 size) {
    if (fn_80480D4C(stream, ints, size) == 0) {
        RWERROR((E_RW_READ));
        return NULL;
    }
    fn_8047D254(ints, size);
    return stream;
}

RwStream *fn_8047D94C(RwStream *stream, RwChunkHeaderInfo *chunkHeaderInfo) {
    RwUInt32 type;
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 buildNum;

    if (!fn_8047CDB4(stream, &type, &length, &version, &buildNum)) {
        return NULL;
    }
    chunkHeaderInfo->type = type;
    chunkHeaderInfo->length = length;
    chunkHeaderInfo->version = version;
    chunkHeaderInfo->buildNum = buildNum;
    chunkHeaderInfo->isComplex = fn_8047CCF8(chunkHeaderInfo);
    return stream;
}
