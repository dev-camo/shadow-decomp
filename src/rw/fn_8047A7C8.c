/* RenderWare file system on the GameCube disc: opening, reading, seeking and polling files through
 * the DVD library, and creating the file system. The original file name is unknown. */
#include "rw/rtgcnfs.h"
#include <dolphin/os.h>
#include <string.h>

/* The longest path the device is opened with, its prefix included */
#define rtGCNFSMAXPATH 0x100

/* The open results (inferred) */
#define RTGCNFS_OPEN_NOERROR 1
#define RTGCNFS_OPEN_NOTFOUND 3
#define RTGCNFS_OPEN_BADMODE 4 /* the disc cannot be written */
#define RTGCNFS_OPEN_PATHTOOLONG 5

/* The device state (RTGCNFS_ERROR_*).
 *
 * Matching workaround: the state is declared volatile. After each store fn_8047B154 loads it back
 * as the error callback's argument (USA: the store at 0x8047B1A4 and the load at 0x8047B1AC), and
 * the compiler emits that reload only for a volatile object; as a plain global it passes the
 * stored constant and fn_8047B154 differs in 20 words. Nothing is known to write the state
 * asynchronously (the DVD callback fn_8047A970 does not touch it), so the qualifier is taken as the
 * original declaration of a device status global, not as a threading need. Revisit when the
 * original declaration is found. */
volatile RwInt32 lbl_805EDE90 = RTGCNFS_ERROR_NONE;

/* Tells the file system's error callback of a device state */
static inline void GCNFSError(RtFileSystem *fileSystem, RwInt32 error) {
    if (fileSystem->errorCallBack != NULL) {
        fileSystem->errorCallBack(error, fileSystem->errorCallBackData);
    }
}

RtFile *fn_8047A7C8(RtFileSystem *fileSystem, RwInt32 index) {
    if ((RwUInt32)index < (RwUInt32)fileSystem->maxNbOpenFiles) {
        return &((RtGCNFile *)fileSystem->files)[index].file;
    }
    return NULL;
}

const RwChar *fn_8047A7EC(RtFileSystem *fileSystem) {
    return "";
}

void fn_8047A7F4(RtFileSystem *fileSystem) {
    RwFree(fileSystem->files);
}

RwInt32 fn_8047A824(RtFileSystem *fileSystem, RtFile *file, const RwChar *name, RwUInt32 mode) {
    RtGCNFile *gcnFile = (RtGCNFile *)file;
    RwChar path[rtGCNFSMAXPATH];
    const RwChar *devicePath;

    if (mode & (RTFILE_MODE_WRITE | RTFILE_MODE_FLAG4 | RTFILE_MODE_FLAG8)) {
        return RTGCNFS_OPEN_BADMODE;
    }
    devicePath = fn_80479698(name);
    if (fn_8047971C(path, rtGCNFSMAXPATH, fileSystem->getPathPrefix(fileSystem), devicePath, '/') ==
        NULL) {
        return RTGCNFS_OPEN_PATHTOOLONG;
    }
    if (!DVDOpen(path, &gcnFile->fileInfo)) {
        return RTGCNFS_OPEN_NOTFOUND;
    }
    file->status = RTFILE_STATUS_READY;
    file->field_0x24 = 1;
    file->position = 0;
    gcnFile->cacheValid = FALSE;
    gcnFile->cacheOffset = 0;
    file->length = gcnFile->fileInfo.length;
    if (mode & RTFILE_MODE_ASYNC) {
        file->field_0x18 = TRUE;
        file->status = RTFILE_STATUS_PENDING;
    } else {
        file->field_0x18 = FALSE;
        file->status = RTFILE_STATUS_READY;
    }
    file->priority = 2;
    return RTGCNFS_OPEN_NOERROR;
}

void fn_8047A938(RtFile *file) {
    DVDClose(&((RtGCNFile *)file)->fileInfo);
    file->status = RTFILE_STATUS_FREE;
}

void fn_8047A970(s32 result, DVDFileInfo *fileInfo) {
    fileInfo->cb.userData = (void *)result;
}

/* Records the drive state while recovering from an error: busy sets the busy error state,
 * anything else clears it. Returns FALSE while the drive is busy (name inferred) */
static inline RwBool GCNFSUpdateDriveState(RtFileSystem *fileSystem, RwInt32 driveStatus) {
    if (driveStatus == DVD_STATE_BUSY) {
        fileSystem->field_0x8 = 1;
        lbl_805EDE90 = RTGCNFS_ERROR_BUSY;
        return FALSE;
    }
    fileSystem->field_0x8 = 2;
    lbl_805EDE90 = RTGCNFS_ERROR_NONE;
    return TRUE;
}

/* Reads whole 32-byte blocks from the device; an asynchronous read returns at once, and a
 * synchronous one waits, reporting device errors as it does */
RwInt32 fn_8047A978(RtGCNFile *file, void *buffer, RwInt32 length, RwInt32 offset, RwBool async) {
    RtFileSystem *fileSystem = file->file.fileSystem;
    RwInt32 alignedLength = (length + 31) & ~31;
    RwInt32 driveStatus;

    BOOL started = DVDReadAsyncPrio(&file->fileInfo, buffer, alignedLength, offset, fn_8047A970,
                                    file->file.priority);

    /* the file's own mode decides; the async argument is not read */
    if (file->file.field_0x18) {
        if (started) {
            file->file.status = RTFILE_STATUS_PENDING;
            lbl_805EDE90 = RTGCNFS_ERROR_NONE;
            return alignedLength;
        }
        return 0;
    }

    do {
        OSYieldThread();
        driveStatus = DVDGetDriveStatus();
        switch (driveStatus) {
        case DVD_STATE_FATAL_ERROR:
            file->file.status = RTFILE_STATUS_ERROR;
            fileSystem->field_0x8 = 7;
            lbl_805EDE90 = RTGCNFS_ERROR_FATAL;
            GCNFSError(fileSystem, RTGCNFS_ERROR_FATAL);
            break;
        case DVD_STATE_NO_DISK:
            file->file.status = RTFILE_STATUS_ERROR;
            fileSystem->field_0x8 = 4;
            lbl_805EDE90 = RTGCNFS_ERROR_NODISK;
            GCNFSError(fileSystem, RTGCNFS_ERROR_NODISK);
            break;
        case DVD_STATE_COVER_OPEN:
            file->file.status = RTFILE_STATUS_ERROR;
            fileSystem->field_0x8 = 3;
            lbl_805EDE90 = RTGCNFS_ERROR_COVEROPEN;
            GCNFSError(fileSystem, RTGCNFS_ERROR_COVEROPEN);
            break;
        case DVD_STATE_WRONG_DISK:
            file->file.status = RTFILE_STATUS_ERROR;
            fileSystem->field_0x8 = 5;
            lbl_805EDE90 = RTGCNFS_ERROR_WRONGDISK;
            GCNFSError(fileSystem, RTGCNFS_ERROR_WRONGDISK);
            break;
        case DVD_STATE_RETRY:
            file->file.status = RTFILE_STATUS_ERROR;
            fileSystem->field_0x8 = 5;
            lbl_805EDE90 = RTGCNFS_ERROR_RETRY;
            GCNFSError(fileSystem, RTGCNFS_ERROR_RETRY);
            break;
        case DVD_STATE_BUSY:
            if (lbl_805EDE90 != RTGCNFS_ERROR_NONE || file->file.status == RTFILE_STATUS_ERROR) {
                /* Always busy here; the original keeps the shared helper's busy test */
                GCNFSUpdateDriveState(fileSystem, driveStatus);
            } else {
                file->file.status = RTFILE_STATUS_PENDING;
                lbl_805EDE90 = RTGCNFS_ERROR_NONE;
            }
            break;
        case DVD_STATE_END:
        case DVD_STATE_WAITING:
        case DVD_STATE_MOTOR_STOPPED:
        case DVD_STATE_PAUSING:
            file->file.status = RTFILE_STATUS_ERROR;
            GCNFSError(fileSystem, RTGCNFS_ERROR_OTHER);
            break;
        default:
            GCNFSError(fileSystem, RTGCNFS_ERROR_OTHER);
            break;
        }
    } while (driveStatus != DVD_STATE_END);

    if (driveStatus == DVD_STATE_END) {
        RwInt32 numRead = (RwInt32)file->fileInfo.cb.userData;

        if (numRead < 0) {
            numRead = 0;
        }
        file->file.status = RTFILE_STATUS_READY;
        return numRead;
    }
    return 0;
}

/* Reads from the file's position. The device reads whole aligned blocks, so a synchronous read
 * that is not aligned goes through the file's cache; an asynchronous one must be aligned */
RwUInt32 fn_8047AC10(RtFile *file, void *buffer, RwUInt32 length) {
    RtGCNFile *gcnFile = (RtGCNFile *)file;
    RwUInt8 *out = buffer;
    RwInt32 numFailed = 0;
    RwInt32 bufferMisalign;
    RwInt32 positionMisalign;
    RwInt32 remaining;

    file->readStart = file->position;
    if (file->field_0x18) {
        file->asyncStatus = RTFILE_ASYNC_READ;
    }
    if (length == 0) {
        return 0;
    }
    if (file->position + length > file->length) {
        length = (RwUInt32)(file->length - file->position);
        if (file->field_0x18) {
            length = (length + 31) & ~31;
        }
    }
    if (length == 0) {
        return 0;
    }

    remaining = length;
    bufferMisalign = (RwUInt32)out & 31;
    positionMisalign = (RwInt32)file->position & 31;
    if (file->field_0x18) {
        RwInt32 lengthMisalign = remaining % 32;

        if (bufferMisalign == 0 && positionMisalign == 0 && lengthMisalign == 0) {
            while (remaining > 0 && numFailed < 1) {
                remaining =
                    fn_8047A978(gcnFile, out, remaining, (RwInt32)file->position, file->field_0x18);
                if (remaining > 0) {
                    file->position += remaining;
                    remaining = 0;
                    numFailed = 0;
                    gcnFile->cacheValid = FALSE;
                    gcnFile->cacheOffset = 0;
                } else {
                    numFailed++;
                }
            }
            if (remaining > 0) {
                return 0;
            }
            return length;
        }
        OSReport("Unaligned async read failed\n");
        if (bufferMisalign != 0) {
            OSReport("Buffer offset not multiple of 32 %p\n", out);
        }
        if (positionMisalign != 0) {
            OSReport("File pointer offset not multiple of 32 %d\n",
                     *(const RtGCNFileOffset *)&file->position);
        }
        if (lengthMisalign != 0) {
            OSReport("Num bytes to read not multiple of 32 %d\n", remaining);
        }
        return 0;
    }

    while (remaining > 0 && numFailed < 1) {
        RwInt32 numRead;

        OSYieldThread();
        if (file->position < 0 || file->position >= file->length) {
            return 0;
        }
        if (bufferMisalign == 0 && positionMisalign == 0 && remaining >= 32) {
            numRead = fn_8047A978(gcnFile, out, remaining & ~31, (RwInt32)file->position,
                                  file->field_0x18);
            if (numRead > 0) {
                remaining -= numRead;
                file->position += numRead;
                out += numRead;
                numFailed = 0;
                gcnFile->cacheValid = FALSE;
                gcnFile->cacheOffset = 0;
            } else {
                numFailed++;
            }
        } else if (file->field_0x18) {
            OSReport("Error: Unaligned async read failed\n");
            return 0;
        } else if (gcnFile->cacheValid == TRUE) {
            RwInt32 numCopied = gcnFile->cacheSize - gcnFile->cacheOffset;

            if (remaining < numCopied) {
                numCopied = remaining;
            }
            memcpy(out, gcnFile->cache + gcnFile->cacheOffset, numCopied);
            bufferMisalign = (bufferMisalign + numCopied) % 32;
            gcnFile->cacheOffset += numCopied;
            if (gcnFile->cacheOffset >= gcnFile->cacheSize) {
                gcnFile->cacheValid = FALSE;
                gcnFile->cacheOffset = 0;
            }
            file->position += numCopied;
            remaining -= numCopied;
            out += numCopied;
            positionMisalign = (positionMisalign + numCopied) % 32;
        } else {
            /* fill the cache from the aligned block the position is in */
            RwInt32 blockStart = (RwInt32)file->position - positionMisalign;
            RwInt32 alignedLength = (RwInt32)((file->length + 31) & ~31);
            RwInt32 cacheLength = gcnFile->cacheSize;

            if (alignedLength < cacheLength + blockStart) {
                cacheLength = alignedLength - blockStart;
            }
            numRead =
                fn_8047A978(gcnFile, gcnFile->cache, cacheLength, blockStart, file->field_0x18);
            if (numRead > 0) {
                gcnFile->cacheOffset = positionMisalign;
                numFailed = 0;
                gcnFile->cacheValid = TRUE;
            } else {
                numFailed++;
            }
        }
    }
    if (!file->field_0x18) {
        file->status = RTFILE_STATUS_READY;
    } else {
        file->status = RTFILE_STATUS_PENDING;
    }
    return length - remaining;
}

/* The disc cannot be written */
RwUInt32 fn_8047B050(RtFile *file, const void *buffer, RwUInt32 length) {
    file->field_0x24 = 2;
    return 0;
}

RwInt64 fn_8047B060(RtFile *file, RwInt32 offset, RwInt32 origin) {
    RtGCNFile *gcnFile = (RtGCNFile *)file;

    switch (origin) {
    case RTFILE_POS_BEGIN:
        file->position = offset;
        gcnFile->cacheValid = FALSE;
        gcnFile->cacheOffset = 0;
        break;
    case RTFILE_POS_CURRENT:
        file->position += offset;
        if (gcnFile->cacheValid) {
            /* keep the cache while the position stays inside it */
            gcnFile->cacheOffset += offset;
            if (gcnFile->cacheOffset >= gcnFile->cacheSize || gcnFile->cacheOffset < 0) {
                gcnFile->cacheValid = FALSE;
                gcnFile->cacheOffset = 0;
            }
        }
        break;
    case RTFILE_POS_END:
        file->position = file->length + offset;
        gcnFile->cacheValid = FALSE;
        gcnFile->cacheOffset = 0;
        break;
    }
    if (!file->field_0x18) {
        file->status = RTFILE_STATUS_READY;
    } else {
        file->status = RTFILE_STATUS_PENDING;
    }
    return file->position;
}

/* Polls the device for a pending operation, reporting device errors and completing the
 * operation once its command block is done; returns the file's status */
RwInt32 fn_8047B154(RtFile *file, RwInt32 unused) {
    RtGCNFile *gcnFile = (RtGCNFile *)file;
    RtFileSystem *fileSystem = file->fileSystem;
    RwInt32 driveStatus = DVDGetDriveStatus();

    switch (driveStatus) {
    case DVD_STATE_FATAL_ERROR:
        file->status = RTFILE_STATUS_ERROR;
        lbl_805EDE90 = RTGCNFS_ERROR_FATAL;
        GCNFSError(fileSystem, lbl_805EDE90);
        break;
    case DVD_STATE_NO_DISK:
        file->status = RTFILE_STATUS_ERROR;
        lbl_805EDE90 = RTGCNFS_ERROR_NODISK;
        GCNFSError(fileSystem, lbl_805EDE90);
        break;
    case DVD_STATE_COVER_OPEN:
        file->status = RTFILE_STATUS_ERROR;
        lbl_805EDE90 = RTGCNFS_ERROR_COVEROPEN;
        GCNFSError(fileSystem, lbl_805EDE90);
        break;
    case DVD_STATE_WRONG_DISK:
        file->status = RTFILE_STATUS_ERROR;
        lbl_805EDE90 = RTGCNFS_ERROR_WRONGDISK;
        GCNFSError(fileSystem, lbl_805EDE90);
        break;
    case DVD_STATE_RETRY:
        file->status = RTFILE_STATUS_ERROR;
        lbl_805EDE90 = RTGCNFS_ERROR_RETRY;
        GCNFSError(fileSystem, lbl_805EDE90);
        break;
    default:
        if (lbl_805EDE90 != RTGCNFS_ERROR_NONE || file->status == RTFILE_STATUS_ERROR) {
            if (!GCNFSUpdateDriveState(fileSystem, driveStatus)) {
                break;
            }
        }
        switch (DVDGetCommandBlockStatus(&gcnFile->fileInfo.cb)) {
        case DVD_STATE_END:
            file->status = RTFILE_STATUS_READY;
            break;
        case DVD_STATE_BUSY:
            file->status = RTFILE_STATUS_PENDING;
            break;
        case DVD_STATE_CANCELED:
            file->status = RTFILE_STATUS_READY;
            break;
        case DVD_STATE_FATAL_ERROR:
        case DVD_STATE_WAITING:
        case DVD_STATE_RETRY:
        default:
            file->status = RTFILE_STATUS_PENDING;
            break;
        }
        break;
    }
    if (file->field_0x18 && file->status == RTFILE_STATUS_READY) {
        if (file->asyncStatus == RTFILE_ASYNC_READ) {
            file->asyncResult = (RwInt32)(file->position - file->readStart);
        }
        fn_804799C8(file);
    }
    return file->status;
}

void fn_8047B384(RtFile *file) {
    DVDCancelAsync(&((RtGCNFile *)file)->fileInfo.cb, NULL);
}

RwInt32 fn_8047B3AC(RtFile *file) {
    if (file->status != RTFILE_STATUS_FREE) {
        fn_8047B154(file, 0);
    }
    return file->status;
}

RwBool fn_8047B3EC(RtFileSystem *fileSystem, const RwChar *name) {
    return fn_804797F0(fileSystem, name);
}

RtFileSystem *fn_8047B40C(RwInt32 maxNbOpenFiles, RwUInt8 *cacheBuffers, RwInt32 cacheSize,
                          const RwChar *deviceName, const RwChar *name) {
    RtFileSystem *fileSystem;
    RwInt32 i;
    RtGCNFile *files;

    if (fn_80479C74(name) != NULL) {
        return NULL;
    }
    fileSystem = RwMalloc(sizeof(RtFileSystem), rwMEMHINTDUR_GLOBAL | rwID_FILESYSTEM);
    fileSystem->getFile = fn_8047A7C8;
    fileSystem->close = fn_8047A7F4;
    fileSystem->getPathPrefix = fn_8047A7EC;
    fileSystem->fileOpen = fn_8047A824;
    fileSystem->fileClose = fn_8047A938;
    fileSystem->fileRead = fn_8047AC10;
    fileSystem->fileWrite = fn_8047B050;
    fileSystem->fileSetPosition = fn_8047B060;
    fileSystem->fileUpdate = fn_8047B154;
    fileSystem->fileAbort = fn_8047B384;
    fileSystem->fileIsEOF = fn_80479848;
    fileSystem->fileGetStatus = fn_8047B3AC;
    fileSystem->fileExists = fn_8047B3EC;
    fileSystem->files =
        RwCalloc(maxNbOpenFiles, sizeof(RtGCNFile), rwMEMHINTDUR_GLOBAL | rwID_FILESYSTEM);
    files = fileSystem->files;
    for (i = 0; i < maxNbOpenFiles; i++) {
        files[i].file.fileSystem = fileSystem;
        files[i].cache = cacheBuffers + i * cacheSize;
        files[i].cacheSize = cacheSize;
        files[i].cacheValid = FALSE;
        files[i].cacheOffset = 0;
    }
    if (fn_80479598(fileSystem, maxNbOpenFiles, name, deviceName) != RTFS_ERROR_NOERROR) {
        RwFree(fileSystem->files);
        RwFree(fileSystem);
        return NULL;
    }
    return fileSystem;
}
