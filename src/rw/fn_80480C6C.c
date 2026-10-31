/* RenderWare streams: reading, writing and skipping through files, memory and custom functions.
 * The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>
#include "layout_check.h"

/* The size a memory stream's block starts at and grows by at least */
#define rwSTREAMSTREAMBLOCKSIZE 512

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

#define RWSTREAMGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwStreamGlobals, lbl_805F15B4, lbl_805F1580.globalsOffset)->var)

/* The engine globals of the stream module */
typedef struct rwStreamGlobals {
    /* 0x0 */ RwFreeList *streamFreeList;
} rwStreamGlobals;
CHECK_SIZEOF(rwStreamGlobals, 0x4);

RwFreeList lbl_805E2AA8; /* the stream free list */

RwInt32 lbl_805EDEA8 = 16; /* streams per free list block */
RwInt32 lbl_805EDEAC = 1;  /* free list blocks to preallocate */

RwModuleInfo lbl_805F1580; /* the stream module */

void *fn_80480C6C(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1580.globalsOffset = offset;
    RWSTREAMGLOBAL(streamFreeList) =
        fn_8047FC04(sizeof(RwStream), lbl_805EDEA8, 4, lbl_805EDEAC, &lbl_805E2AA8,
                    rwMEMHINTDUR_GLOBAL | rwID_STREAMMODULE);
    if (RWSTREAMGLOBAL(streamFreeList) == NULL) {
        return NULL;
    }
    lbl_805F1580.numInstances++;
    return instance;
}

void *fn_80480CF8(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWSTREAMGLOBAL(streamFreeList) != NULL) {
        fn_8047FCDC(RWSTREAMGLOBAL(streamFreeList));
    }
    lbl_805F1580.numInstances--;
    return instance;
}

/* Reads up to length bytes into buffer, returning the number read */
RwUInt32 fn_80480D4C(RwStream *stream, void *buffer, RwUInt32 length) {
    switch (stream->type) {
    case rwSTREAMFILE:
    case rwSTREAMFILENAME: {
        void *fp = stream->Type.fpFile;
        RwUInt32 bytesRead = fn_8047A3E0(buffer, 1, length, fp);

        if (bytesRead != length) {
            if (fn_8047A650(fp)) {
                RWERROR((E_RW_ENDOFSTREAM));
            } else {
                RWERROR((E_RW_READ));
            }
        }
        return bytesRead;
    }
    case rwSTREAMMEMORY: {
        RwUInt32 available = stream->Type.memory.nSize - stream->Type.memory.position;

        if (length > available) {
            length = available;
            RWERROR((E_RW_ENDOFSTREAM));
        }
        memcpy(buffer, stream->Type.memory.memBlock + stream->Type.memory.position, length);
        stream->Type.memory.position += length;
        return length;
    }
    case rwSTREAMCUSTOM:
        return stream->Type.custom.sfnread(stream->Type.custom.data, buffer, length);
    default:
        RWERROR((E_RW_BADSTREAMTYPE));
        return 0;
    }
}

/* Writes length bytes from buffer; a memory stream's block grows to fit */
RwStream *fn_80480ED0(RwStream *stream, const void *buffer, RwUInt32 length) {
    switch (stream->type) {
    case rwSTREAMFILE:
    case rwSTREAMFILENAME: {
        RwUInt32 bytesWritten = fn_8047A428(buffer, 1, length, stream->Type.fpFile);

        if (bytesWritten != length) {
            RWERROR((E_RW_WRITE));
            return NULL;
        }
        return stream;
    }
    case rwSTREAMMEMORY:
        if (stream->Type.memory.memBlock == NULL) {
            stream->Type.memory.memBlock =
                RwMalloc(rwSTREAMSTREAMBLOCKSIZE, rwMEMHINTDUR_EVENT | rwID_STREAMMODULE);
            if (stream->Type.memory.memBlock == NULL) {
                RWERROR((E_RW_NOMEM, rwSTREAMSTREAMBLOCKSIZE));
                return NULL;
            }
            stream->Type.memory.nSize = rwSTREAMSTREAMBLOCKSIZE;
        }
        if (stream->Type.memory.nSize - stream->Type.memory.position < length) {
            RwUInt32 newSize = length + stream->Type.memory.nSize;
            RwUInt8 *memBlock;

            if (length < rwSTREAMSTREAMBLOCKSIZE) {
                newSize = stream->Type.memory.nSize + rwSTREAMSTREAMBLOCKSIZE;
            }
            memBlock = RwRealloc(stream->Type.memory.memBlock, newSize,
                                 rwMEMHINTDUR_EVENT | rwMEMHINTFLAG_RESIZABLE | rwID_STREAMMODULE);
            if (memBlock == NULL) {
                RWERROR((E_RW_NOMEM, newSize - stream->Type.memory.nSize));
                return NULL;
            }
            stream->Type.memory.memBlock = memBlock;
            stream->Type.memory.nSize = newSize;
        }
        memcpy(stream->Type.memory.memBlock + stream->Type.memory.position, buffer, length);
        stream->Type.memory.position += length;
        return stream;
    case rwSTREAMCUSTOM:
        if (stream->Type.custom.sfnwrite(stream->Type.custom.data, buffer, length)) {
            return stream;
        }
        return NULL;
    default:
        RWERROR((E_RW_BADSTREAMTYPE));
        return NULL;
    }
}

/* Skips offset bytes */
RwStream *fn_804810FC(RwStream *stream, RwUInt32 offset) {
    if (offset == 0) {
        return stream;
    }

    switch (stream->type) {
    case rwSTREAMFILE:
    case rwSTREAMFILENAME: {
        void *fp = stream->Type.fpFile;

        if (fn_8047A470(fp, offset, 1)) {
            if (fn_8047A650(fp)) {
                RWERROR((E_RW_ENDOFSTREAM));
            }
            return NULL;
        }
        return stream;
    }
    case rwSTREAMMEMORY:
        if (stream->Type.memory.position + offset > stream->Type.memory.nSize) {
            stream->Type.memory.position = stream->Type.memory.nSize;
            RWERROR((E_RW_ENDOFSTREAM));
            return NULL;
        }
        stream->Type.memory.position += offset;
        return stream;
    case rwSTREAMCUSTOM:
        if (stream->Type.custom.sfnskip(stream->Type.custom.data, offset)) {
            return stream;
        }
        return NULL;
    default:
        RWERROR((E_RW_BADSTREAMTYPE));
        return NULL;
    }
}

/* Closes a stream: a file name stream's file is closed, and a written memory stream's block is
 * given to data */
RwBool fn_8048124C(RwStream *stream, void *data) {
    RwBool result;

    switch (stream->type) {
    case rwSTREAMFILE:
        result = TRUE;
        break;
    case rwSTREAMFILENAME:
        result = !fn_8047A388(stream->Type.fpFile);
        break;
    case rwSTREAMMEMORY:
        if (stream->accessType != rwSTREAMREAD && data != NULL) {
            RwMemory *memory = (RwMemory *)data;

            memory->start = stream->Type.memory.memBlock;
            memory->length = stream->Type.memory.position;
        }
        result = TRUE;
        break;
    case rwSTREAMCUSTOM:
        if (stream->Type.custom.sfnclose != NULL) {
            stream->Type.custom.sfnclose(stream->Type.custom.data);
        }
        result = TRUE;
        break;
    default:
        RWERROR((E_RW_BADSTREAMTYPE));
        return FALSE;
    }

    if (stream->rwOwned) {
        RwFreeListFree(RWSTREAMGLOBAL(streamFreeList), stream);
    }
    return result;
}

/* Sets up a memory stream to read or append to memory, or to write a new block (inferred) */
static inline RwStream *StreamOpenMemory(RwStream *stream, RwInt32 accessType,
                                         const RwMemory *memory) {
    RwStream *result = NULL;

    switch (accessType) {
    case rwSTREAMREAD:
        stream->Type.memory.position = 0;
        stream->Type.memory.nSize = memory->length;
        stream->Type.memory.memBlock = memory->start;
        result = stream;
        break;
    case rwSTREAMWRITE:
        stream->Type.memory.position = 0;
        stream->Type.memory.nSize = 0;
        stream->Type.memory.memBlock = NULL;
        result = stream;
        break;
    case rwSTREAMAPPEND:
        stream->Type.memory.position = memory->length;
        stream->Type.memory.nSize = memory->length;
        stream->Type.memory.memBlock = memory->start;
        result = stream;
        break;
    default:
        RWERROR((E_RW_BADACCESSTYPE));
        break;
    }
    return result;
}

/* Initializes a stream of the type and access type from data (inferred); returns NULL when it
 * cannot */
static inline RwStream *StreamInitialize(RwStream *stream, RwInt32 type, RwInt32 accessType,
                                         const void *data) {
    RwStream *result = NULL;

    if (stream == NULL) {
        return NULL;
    }
    stream->type = type;
    stream->accessType = accessType;
    stream->rwOwned = TRUE;

    switch (type) {
    case rwSTREAMFILE:
        if (fn_8047A7C0((void *)data) == -1) {
            result = NULL;
        } else {
            stream->Type.fpFile = (void *)data;
            result = stream;
        }
        break;
    case rwSTREAMFILENAME: {
        void *fp = NULL;

        switch (accessType) {
        case rwSTREAMREAD:
            fp = fn_8047A178(data, "rb");
            break;
        case rwSTREAMWRITE:
            fp = fn_8047A178(data, "wb");
            break;
        case rwSTREAMAPPEND:
            fp = fn_8047A178(data, "ab");
            break;
        default:
            RWERROR((E_RW_BADACCESSTYPE));
            break;
        }
        if (fp != NULL) {
            stream->Type.fpFile = fp;
            result = stream;
        } else {
            RWERROR((E_RW_FILEOPEN, data));
        }
        break;
    }
    case rwSTREAMMEMORY:
        result = StreamOpenMemory(stream, accessType, (const RwMemory *)data);
        break;
    case rwSTREAMCUSTOM:
        memcpy(&stream->Type.custom, data, sizeof(RwStreamCustom));
        result = stream;
        break;
    default:
        RWERROR((E_RW_BADSTREAMTYPE));
        break;
    }
    return result;
}

/* Opens a stream: data is a file, a file name, the RwMemory to read or NULL, or the
 * RwStreamCustom */
RwStream *fn_80481368(RwInt32 type, RwInt32 accessType, const void *data) {
    RwStream *stream =
        RwFreeListAlloc(RWSTREAMGLOBAL(streamFreeList), rwMEMHINTDUR_EVENT | rwID_STREAMMODULE);
    if (StreamInitialize(stream, type, accessType, data) == NULL) {
        RwFreeListFree(RWSTREAMGLOBAL(streamFreeList), stream);
        stream = NULL;
    }
    return stream;
}
