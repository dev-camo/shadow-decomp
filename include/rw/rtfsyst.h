#ifndef RW_RTFSYST_H
#define RW_RTFSYST_H

#include "rw/rwcore.h"

/* RenderWare file system toolkit: file systems registered with a manager under a name and a
 * device name, and the files they open. The type names follow the RenderWare 3 RtFileSystem
 * toolkit; the member names are inferred from the code that uses them, and only the members used
 * so far are mapped. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RtFile RtFile;
typedef struct RtFileSystem RtFileSystem;

/* The toolkit's results (inferred) */
#define RTFS_ERROR_NOERROR 1
#define RTFS_ERROR_TOOMANYFS 2       /* the manager has no room for another file system */
#define RTFS_ERROR_FSNAMEINUSE 6     /* a file system of that name is registered */
#define RTFS_ERROR_DEVICENAMEINUSE 7 /* a file system of that device name is registered */

/* A file's states (inferred) */
#define RTFILE_STATUS_FREE 1    /* not open */
#define RTFILE_STATUS_OPENING 5 /* taken by an open in progress */

/* A file's asynchronous operation states (inferred) */
#define RTFILE_ASYNC_IDLE 1
#define RTFILE_ASYNC_SETPOSITION 4 /* a seek; reports the new position when it completes */

/* Called when a file's asynchronous operation completes */
typedef void (*RtFileCallBack)(RtFile *file, RwInt32 result, RwInt32 status, RwInt32 asyncStatus,
                               void *data);

/* A file of a file system */
struct RtFile {
    /* 0x00 */ RwInt64 length;
    /* 0x08 */ RwInt64 position; /* -1 when unknown */
    /* 0x10 */ RwInt32 field_0x10;
    /* 0x14 */ RwUInt8 field_0x14[0x4];
    /* 0x18 */ RwBool field_0x18; /* copied from its file system's field_0x10 */
    /* 0x1C */ RwUInt8 field_0x1C[0x4];
    /* 0x20 */ RwInt32 status; /* RTFILE_STATUS_* */
    /* 0x24 */ RwUInt8 field_0x24[0x4];
    /* 0x28 */ RwInt32 asyncStatus; /* RTFILE_ASYNC_* */
    /* 0x2C */ RwInt32 field_0x2C;
    /* 0x30 */ RtFileCallBack callBack;
    /* 0x34 */ void *callBackData;
    /* 0x38 */ RtFileSystem *fileSystem;
};

/* A file system (0x64 bytes) */
struct RtFileSystem {
    /* 0x00 */ RtFileSystem *next; /* in the manager's list */
    /* 0x04 */ RwInt32 maxNbOpenFiles;
    /* 0x08 */ RwInt32 field_0x8;
    /* 0x0C */ RwChar *deviceName;
    /* 0x10 */ RwBool field_0x10;
    /* 0x14 */ RtFile *(*getFile)(RtFileSystem *fileSystem, RwInt32 index);
    /* 0x18 */ void (*close)(RtFileSystem *fileSystem);
    /* 0x1C */ RwUInt8 field_0x1C[0x4];
    /* 0x20 */ void (*registerNotify)(RwChar *name);   /* told the name when registered */
    /* 0x24 */ void (*unregisterNotify)(RwChar *name); /* told the name when unregistered */
    /* 0x28 */ RwUInt8 field_0x28[0x4];
    /* 0x2C */ void (*fileClose)(RtFile *file);
    /* 0x30 */ RwUInt8 field_0x30[0x20];
    /* 0x50 */ RwChar name[8];
    /* 0x58 */ RwUInt8 field_0x58[0xC];
};

/* The notifications of a file system (inferred) */
#define RTFS_NOTIFY_REGISTER 1
#define RTFS_NOTIFY_UNREGISTER 2

/* file systems (inferred) */
RwInt32 fn_80479598(RtFileSystem *fileSystem, RwInt32 maxNbOpenFiles, const RwChar *name,
                    const RwChar *deviceName); /* sets up a file system */
const RwChar *fn_80479698(const RwChar *path); /* the path after its device name */
RwChar *fn_8047971C(RwChar *buffer, RwInt32 bufferSize, const RwChar *prefix, const RwChar *path,
                    RwChar separator);                            /* joins a prefix and a path */
RwBool fn_804797F0(RtFileSystem *fileSystem, const RwChar *name); /* whether a file exists */
RwBool fn_80479848(RtFile *file);                                 /* whether at its end */
void fn_80479894(RtFileSystem *fileSystem, RwInt32 notification);
RwInt32 fn_804798FC(RtFileSystem *fileSystem, const RwChar *deviceName); /* sets the device */
void fn_804799B8(RtFile *file, RwInt32 value);                           /* sets field_0x10 */
RwInt32 fn_804799C0(RtFile *file);                                       /* gets field_0x10 */
void fn_804799C8(RtFile *file); /* completes an asynchronous operation */

/* the file system manager (inferred) */
RwBool fn_80479B48(void);                            /* whether another one fits */
RwBool fn_80479D08(const RwChar *name);              /* whether a name is in use */
RtFileSystem *fn_80479FD0(const RwChar *deviceName); /* by device name, or NULL */
RtFile *fn_80479DCC(RtFileSystem *fileSystem, const RwChar *name, RwUInt32 flags,
                    RtFileCallBack callBack, void *callBackData); /* opens a file */

#ifdef __cplusplus
}
#endif

#endif
