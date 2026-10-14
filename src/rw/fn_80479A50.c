/* RenderWare file system manager: the registered file systems, finding them by name and by the
 * device name a path starts with, opening files on them, and the file functions RenderWare reads
 * and writes through. The original file name is unknown. */
#include "rw/rtfsyst.h"
#include <dolphin/os.h>
#include <string.h>

/* The longest device name a path may start with, its colon included (the exact size is not
 * established; the original's stack frames fit any size from 97 to 108 bytes) */
#define rtFSMANAGERMAXDEVICENAME 100

/* The origins of the standard seek (no project header defines them) */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

RtFSManager lbl_805E2A40;
RtFileSystem *lbl_805F1548;

RwBool fn_80479A50(RwInt32 maxNbFS) {
    lbl_805E2A40.maxNbFS = maxNbFS;
    lbl_805E2A40.nbFS = 0;
    lbl_805E2A40.errorCallBack = NULL;
    lbl_805F1548 = NULL;
    lbl_805E2A40.fileSystems = NULL;
    lbl_805E2A40.isInit = TRUE;
    return TRUE;
}

void fn_80479A80(void) {
    if (lbl_805E2A40.isInit == TRUE) {
        RtFileSystem *next;
        RtFileSystem *fileSystem = lbl_805E2A40.fileSystems;

        while (fileSystem != NULL) {
            next = fileSystem->next;

            fileSystem->close(fileSystem);
            RwFree(fileSystem->deviceName);
            fileSystem->deviceName = NULL;
            fn_80479894(fileSystem, RTFS_NOTIFY_UNREGISTER);
            RwFree(fileSystem);
            fileSystem = next;
        }
        lbl_805E2A40.isInit = FALSE;
        lbl_805E2A40.nbFS = 0;
    }
}

/* Reports an error to the manager's error callback */
#define RTFSMERROR(error)                                                                          \
    do {                                                                                           \
        if (lbl_805E2A40.errorCallBack != NULL) {                                                  \
            lbl_805E2A40.errorCallBack(error);                                                     \
        }                                                                                          \
    } while (0)

RwBool fn_80479B48(void) {
    if (lbl_805E2A40.maxNbFS > 0 && lbl_805E2A40.nbFS + 1 > lbl_805E2A40.maxNbFS) {
        RTFSMERROR(RTFSM_ERROR_TOOMANYFS);
        return FALSE;
    }
    return TRUE;
}

RwBool fn_80479BAC(RtFileSystem *fileSystem) {
    if (fn_80479B48()) {
        if (fn_80479C74(fileSystem->name) == NULL) {
            fn_80479894(fileSystem, RTFS_NOTIFY_REGISTER);
            fileSystem->next = lbl_805E2A40.fileSystems;
            lbl_805E2A40.fileSystems = fileSystem;
            lbl_805E2A40.nbFS++;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

RtFileSystem *fn_80479C74(const RwChar *name) {
    RtFileSystem *fileSystem;

    for (fileSystem = lbl_805E2A40.fileSystems; fileSystem != NULL; fileSystem = fileSystem->next) {
        if (rwstrcmp(fileSystem->name, name) == 0) {
            return fileSystem;
        }
    }
    RTFSMERROR(RTFSM_ERROR_FSNOTFOUND);
    return NULL;
}

RwBool fn_80479D08(const RwChar *name) {
    if (fn_80479C74(name) != NULL) {
        RTFSMERROR(RTFSM_ERROR_FSREGISTERED);
        return TRUE;
    }
    return FALSE;
}

RtFile *fn_80479DCC(RtFileSystem *fileSystem, const RwChar *name, RwUInt32 mode,
                    RtFileCallBack callBack, void *callBackData) {
    RtFile *file = NULL;

    if (fileSystem != NULL) {
        RwInt32 i;

        /* take a free file before anything else can */
        OSDisableInterrupts();
        for (i = 0; i < fileSystem->maxNbOpenFiles; i++) {
            RtFile *candidate = fileSystem->getFile(fileSystem, i);

            if (candidate->status == RTFILE_STATUS_FREE) {
                file = candidate;
                candidate->status = RTFILE_STATUS_OPENING;
                break;
            }
        }
        OSEnableInterrupts();

        if (file == NULL) {
            fn_80479FC4()->lastError = RTFSM_ERROR_NOFREEFILE;
            RTFSMERROR(RTFSM_ERROR_NOFREEFILE);
            return NULL;
        }
        if ((mode & RTFILE_MODE_ASYNC) || file->field_0x18) {
            file->asyncStatus = RTFILE_ASYNC_OPEN;
            file->callBack = callBack;
            file->callBackData = callBackData;
        }
        if (fileSystem->asyncMode) {
            mode |= RTFILE_MODE_ASYNC;
            file->asyncStatus = RTFILE_ASYNC_OPEN;
            file->callBack = callBack;
            file->callBackData = callBackData;
        }
        if (fileSystem->fileOpen(fileSystem, file, name, mode) != RTFS_ERROR_NOERROR) {
            file->status = RTFILE_STATUS_FREE;
            file = NULL;
        }
        if (file != NULL) {
            file->field_0x1C = 2;
        }
        return file;
    }
    RTFSMERROR(RTFSM_ERROR_NOFILESYSTEM);
    return NULL;
}

RtFile *fn_80479F50(const RwChar *name, RwUInt32 mode, RtFileCallBack callBack,
                    void *callBackData) {
    RtFileSystem *fileSystem = fn_8047A044(name);

    if (fileSystem != NULL) {
        return fn_80479DCC(fileSystem, name, mode, callBack, callBackData);
    }
    return NULL;
}

RtFSManager *fn_80479FC4(void) {
    return &lbl_805E2A40;
}

RtFileSystem *fn_80479FD0(const RwChar *deviceName) {
    RtFileSystem *fileSystem;

    for (fileSystem = lbl_805E2A40.fileSystems; fileSystem != NULL; fileSystem = fileSystem->next) {
        if (rwstricmp(deviceName, fileSystem->deviceName) == 0) {
            return fileSystem;
        }
    }
    return NULL;
}

RtFileSystem *fn_8047A044(const RwChar *path) {
    if (lbl_805E2A40.nbFS != 0) {
        RwUInt32 length = rwstrlen(path);
        RwChar deviceName[rtFSMANAGERMAXDEVICENAME];
        RwUInt32 i;

        for (i = 0; i < length; i++) {
            if (path[i] == ':') {
                RtFileSystem *fileSystem;

                strncpy(deviceName, path, i + 1);
                deviceName[i + 1] = '\0';
                fileSystem = fn_80479FD0(deviceName);
                if (fileSystem != NULL) {
                    return fileSystem;
                }
            }
        }
        if (lbl_805F1548 != NULL) {
            return lbl_805F1548;
        }
    }
    RTFSMERROR(RTFSM_ERROR_FSNOTFOUND);
    return NULL;
}

void fn_8047A168(RtFileSystem *fileSystem) {
    lbl_805F1548 = fileSystem;
}

RtFileSystem *fn_8047A170(void) {
    return lbl_805F1548;
}

/* Opens a file with a standard access mode string */
void *fn_8047A178(const RwChar *name, const RwChar *access) {
    RwUInt32 mode = 0;
    RwBool update = FALSE;
    RwBool binary = FALSE;
    RtFileSystem *fileSystem;

    if (strstr(access, "+") != NULL) {
        update = TRUE;
    }
    if (strstr(access, "b") != NULL) {
        binary = TRUE;
    }
    if (strstr(access, "r") != NULL) {
        mode |= RTFILE_MODE_READ;
        if (update) {
            mode |= RTFILE_MODE_WRITE;
            mode |= RTFILE_MODE_FLAG8;
        }
    }
    if (strstr(access, "w") != NULL) {
        mode |= RTFILE_MODE_WRITE;
        mode |= RTFILE_MODE_FLAG4;
        if (update) {
            mode |= RTFILE_MODE_READ;
        }
    }
    if (strstr(access, "a") != NULL) {
        mode |= RTFILE_MODE_WRITE;
        mode |= RTFILE_MODE_FLAG8;
        if (update) {
            mode |= RTFILE_MODE_READ;
        }
        if (binary) {
            mode |= RTFILE_MODE_FLAG4;
        }
    }
    fileSystem = fn_8047A044(name);
    if (fileSystem != NULL) {
        return fn_80479DCC(fileSystem, name, mode, NULL, NULL);
    }
    return NULL;
}

RwInt32 fn_8047A388(void *fptr) {
    RtFileSystem *fileSystem;
    RtFile *file = fptr;

    fileSystem = file->fileSystem;
    OSDisableInterrupts();
    file->status = RTFILE_STATUS_FREE;
    fileSystem->fileClose(file);
    OSEnableInterrupts();
    return 0;
}

RwUInt32 fn_8047A3E0(void *buffer, RwUInt32 size, RwUInt32 count, void *fptr) {
    RtFile *file = fptr;

    return file->fileSystem->fileRead(file, buffer, size * count) / size;
}

RwUInt32 fn_8047A428(const void *buffer, RwUInt32 size, RwUInt32 count, void *fptr) {
    RtFile *file = fptr;

    return file->fileSystem->fileWrite(file, buffer, size * count) / size;
}

RwInt32 fn_8047A470(void *fptr, RwInt32 offset, RwInt32 origin) {
    RtFileSystem *fileSystem;
    RtFile *file = fptr;

    fileSystem = file->fileSystem;
    if (file->field_0x18) {
        file->asyncStatus = RTFILE_ASYNC_SETPOSITION;
        file->status = RTFILE_STATUS_PENDING;
    }
    switch (origin) {
    case SEEK_CUR:
        fileSystem->fileSetPosition(file, offset, RTFILE_POS_CURRENT);
        break;
    case SEEK_END:
        fileSystem->fileSetPosition(file, offset, RTFILE_POS_END);
        break;
    case SEEK_SET:
        fileSystem->fileSetPosition(file, offset, RTFILE_POS_BEGIN);
        break;
    default:
        return -1;
    }
    return 0;
}

/* Reads a line of at most maxLen - 1 characters, ending after its newline; carriage returns
 * before a newline are dropped, and the file is left just after the line */
RwChar *fn_8047A518(RwChar *buffer, RwInt32 maxLen, void *fptr) {
    RtFile *file = fptr;
    RwInt32 i = 0;
    RwInt32 numRead = file->fileSystem->fileRead(file, buffer, maxLen - 1);

    if (numRead == 0) {
        return NULL;
    }
    while (i < numRead) {
        if (buffer[i] == '\n') {
            i++;
            buffer[i] = '\0';
            fn_8047A470(file, i - numRead, SEEK_CUR);
            return buffer;
        }
        if (buffer[i] == '\r') {
            if (i < numRead - 1 && buffer[i + 1] == '\n') {
                memcpy(&buffer[i], &buffer[i + 1], numRead - i - 1);
                numRead--;
            } else {
                i++;
            }
        } else {
            i++;
        }
    }
    buffer[numRead] = '\0';
    return buffer;
}

RwBool fn_8047A650(void *fptr) {
    RtFile *file = fptr;

    return file->fileSystem->fileIsEOF(file);
}

RwBool fn_8047A67C(const RwChar *name) {
    RtFileSystem *fileSystem = fn_8047A044(name);

    if (fileSystem == NULL) {
        return FALSE;
    }
    return fileSystem->fileExists(fileSystem, name);
}

RwInt32 fn_8047A7C0(void *fptr) {
    RtFile *file = fptr;

    return (RwInt32)file->position;
}
