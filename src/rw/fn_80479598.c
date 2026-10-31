/* RenderWare file system toolkit: setting up file systems, device names and paths, and the
 * default file operations. The original file name is unknown. */
#include "rw/rtfsyst.h"
#include <string.h>

/* The toolkit's plugin ID, for its memory hints (inferred) */
#define rwID_FILESYSTEM 0x1BE

RwInt32 fn_80479598(RtFileSystem *fileSystem, RwInt32 maxNbOpenFiles, const RwChar *name,
                    const RwChar *deviceName) {
    RwInt32 i;
    RwInt32 result;

    if (fn_80479B48()) {
        if (fn_80479D08(name)) {
            return RTFS_ERROR_FSNAMEINUSE;
        }
        rwstrcpy(fileSystem->name, name);
        fileSystem->maxNbOpenFiles = maxNbOpenFiles;
        fileSystem->asyncMode = FALSE;
        for (i = 0; i < maxNbOpenFiles; i++) {
            RtFile *file = fileSystem->getFile(fileSystem, i);

            file->status = RTFILE_STATUS_FREE;
            file->field_0x18 = fileSystem->asyncMode;
        }
    } else {
        return RTFS_ERROR_TOOMANYFS;
    }
    fileSystem->deviceName = NULL;
    result = fn_804798FC(fileSystem, deviceName);
    if (result != RTFS_ERROR_NOERROR) {
        return result;
    }
    fileSystem->field_0x8 = 2;
    fileSystem->registerNotify = NULL;
    fileSystem->unregisterNotify = NULL;
    return RTFS_ERROR_NOERROR;
}

const RwChar *fn_80479698(const RwChar *path) {
    const RwChar *character;
    RwInt32 i = 0;

    character = path;

    while (i < (RwInt32)rwstrlen(path)) {
        if (*character == ':') {
            path += i + 1;
            break;
        }
        i++;
        character++;
    }
    return path;
}

/* Copies a prefix and then a path into a buffer, with the path's slashes and backslashes replaced
 * by a separator; returns NULL if they do not fit */
RwChar *fn_8047971C(RwChar *buffer, RwInt32 bufferSize, const RwChar *prefix, const RwChar *path,
                    RwChar separator) {
    RwInt32 prefixLength = strlen(prefix);
    RwInt32 pathLength = strlen(path);
    const RwChar *in;
    RwChar *out;
    RwInt32 i;

    if (prefixLength + pathLength > bufferSize - 1) {
        return NULL;
    }
    rwstrcpy(buffer, prefix);
    in = path;
    out = buffer + prefixLength;
    /* the path's terminator is copied too */
    for (i = 0; i <= pathLength; i++) {
        RwChar character = *in;

        if (character == '/' || character == '\\') {
            *out = separator;
        } else {
            *out = character;
        }
        out++;
        in++;
    }
    return buffer;
}

RwBool fn_804797F0(RtFileSystem *fileSystem, const RwChar *name) {
    RtFile *file = fn_80479DCC(fileSystem, name, RTFILE_MODE_READ, NULL, NULL);

    if (file != NULL) {
        fileSystem->fileClose(file);
        return TRUE;
    }
    return FALSE;
}

RwBool fn_80479848(RtFile *file) {
    if (file->position == -1) {
        return TRUE;
    }
    return file->position >= file->length;
}

void fn_80479894(RtFileSystem *fileSystem, RwInt32 notification) {
    switch (notification) {
    case RTFS_NOTIFY_REGISTER:
        if (fileSystem->registerNotify != NULL) {
            fileSystem->registerNotify(fileSystem->name);
        }
        break;
    case RTFS_NOTIFY_UNREGISTER:
        if (fileSystem->unregisterNotify != NULL) {
            fileSystem->unregisterNotify(fileSystem->name);
        }
        break;
    }
}

RwInt32 fn_804798FC(RtFileSystem *fileSystem, const RwChar *deviceName) {
    if (fn_80479FD0(deviceName) == NULL) {
        if (fileSystem->deviceName != NULL) {
            RwFree(fileSystem->deviceName);
        }
        fileSystem->deviceName =
            RwMalloc(rwstrlen(deviceName) + 1, rwMEMHINTDUR_GLOBAL | rwID_FILESYSTEM);
        rwstrcpy(fileSystem->deviceName, deviceName);
    } else {
        return RTFS_ERROR_DEVICENAMEINUSE;
    }
    return RTFS_ERROR_NOERROR;
}

void fn_804799B8(RtFileSystem *fileSystem, RwBool asyncMode) {
    fileSystem->asyncMode = asyncMode;
}

RwBool fn_804799C0(RtFileSystem *fileSystem) {
    return fileSystem->asyncMode;
}

void fn_804799C8(RtFile *file) {
    if (file != NULL && file->field_0x18) {
        /* first the operation that completed, then its result */
        RwInt32 value = file->asyncStatus;

        if (value != RTFILE_ASYNC_IDLE && file->callBack != NULL) {
            RwInt32 asyncStatus = value;

            if (asyncStatus == RTFILE_ASYNC_SETPOSITION) {
                value = (RwInt32)file->position;
            } else {
                value = file->asyncResult;
            }
            file->asyncStatus = RTFILE_ASYNC_IDLE;
            file->callBack(file, value, file->status, asyncStatus, file->callBackData);
        } else {
            file->asyncStatus = RTFILE_ASYNC_IDLE;
        }
    }
}
