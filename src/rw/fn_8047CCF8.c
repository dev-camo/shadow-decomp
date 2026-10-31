/* RenderWare stream chunks and byte order: reading, writing and finding chunk headers, converting
 * memory between the native and the little-endian stream byte order, and reading and writing
 * converted reals and integers. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>
#include "layout_check.h"

/* The bytes converted at a time when writing */
#define rwSTREAMCONVERTBUFFER 0x100

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
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

/* Whether a chunk of this type holds other chunks. The result is set in each case and returned
 * once: with a return in each case the compiler inlines this function into its callers, which
 * call it out of line in the original. */
RwBool fn_8047CCF8(const RwChunkHeaderInfo *chunkHeaderInfo) {
    RwBool result = FALSE;

    switch (chunkHeaderInfo->type) {
    case rwID_STRUCT:
        result = FALSE;
        break;
    case rwID_STRING:
        result = FALSE;
        break;
    case rwID_EXTENSION:
        result = FALSE;
        break;
    case rwID_CAMERA:
        result = TRUE;
        break;
    case rwID_TEXTURE:
        result = TRUE;
        break;
    case rwID_MATERIAL:
        result = TRUE;
        break;
    case rwID_MATLIST:
        result = TRUE;
        break;
    case rwID_ATOMICSECT:
        result = TRUE;
        break;
    case rwID_PLANESECT:
        result = TRUE;
        break;
    case rwID_WORLD:
        result = TRUE;
        break;
    case rwID_MATRIX:
        result = FALSE;
        break;
    case rwID_FRAMELIST:
        result = TRUE;
        break;
    case rwID_GEOMETRY:
        result = TRUE;
        break;
    case rwID_CLUMP:
        result = TRUE;
        break;
    case rwID_LIGHT:
        result = TRUE;
        break;
    case rwID_UNICODESTRING:
        result = FALSE;
        break;
    case rwID_ATOMIC:
        result = TRUE;
        break;
    case rwID_GEOMETRYLIST:
        result = TRUE;
        break;
    default:
        result = FALSE;
        break;
    }
    return result;
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
            (info).version = ((((header).libraryID >> 14) & 0x3FF00) + 0x30000) |                  \
                             (((header).libraryID >> 16) & 0x3F);                                  \
            (info).buildNum = (header).libraryID & 0xFFFF;                                         \
        }                                                                                          \
        (info).isComplex = fn_8047CCF8(&(info));                                                   \
    } while (0)

RwBool fn_8047CDB4(RwStream *stream, RwUInt32 *type, RwUInt32 *length, RwUInt32 *version,
                   RwUInt32 *buildNum) {
    rwChunkHeader header;
    RwChunkHeaderInfo info;
    RwBool readOk;

    /* Matching workaround: the read check is computed as a value before it is tested. Testing
     * `fn_80480D4C(...) != sizeof(header)` directly makes fn_8047CDB4 8 bytes smaller than the
     * original (0x134 against 0x13C). Revisit when the original form of the check is found. */
    readOk = fn_80480D4C(stream, &header, sizeof(header)) == sizeof(header);
    if (!readOk) {
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
            if (version > RW_STREAM_VERSION) {
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

/* Converts 32-bit words from the native byte order to the stream's little-endian order (the
 * compiler turns the byte swap and the store into stwbrx) */
void *fn_8047D158(void *mem, RwUInt32 size) {
    RwUInt32 *word = mem;

    for (size >>= 2; size > 0; size--) {
        *word = (*word >> 24) | ((*word >> 8) & 0xFF00) | ((*word << 8) & 0xFF0000) | (*word << 24);
        word++;
    }
    return mem;
}

/* Converts 16-bit words from the native byte order to the stream's little-endian order */
void *fn_8047D1C8(void *mem, RwUInt32 size) {
    RwUInt16 *word = mem;

    for (size >>= 1; size > 0; size--) {
        *word = (*word >> 8) | (*word << 8);
        word++;
    }
    return mem;
}

/* Converts 32-bit words from the stream's little-endian order to the native byte order; the
 * same swap as fn_8047D158 */
void *fn_8047D254(void *mem, RwUInt32 size) {
    RwUInt32 *word = mem;

    for (size >>= 2; size > 0; size--) {
        *word = (*word >> 24) | ((*word >> 8) & 0xFF00) | ((*word << 8) & 0xFF0000) | (*word << 24);
        word++;
    }
    return mem;
}

/* Converts 16-bit words from the stream's little-endian order to the native byte order; the
 * same swap as fn_8047D1C8 */
void *fn_8047D2C4(void *mem, RwUInt32 size) {
    RwUInt16 *word = mem;

    for (size >>= 1; size > 0; size--) {
        *word = (*word >> 8) | (*word << 8);
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
