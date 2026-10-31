#ifndef RW_RTFSYST_H
#define RW_RTFSYST_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* RenderWare file system toolkit: file systems registered with a manager under a name and a
 * device name, and the files they open. The type names follow the RenderWare 3 RtFileSystem
 * toolkit; the member names are inferred from the code that uses them, and only the members used
 * so far are mapped. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RtFile RtFile;
typedef struct RtFileSystem RtFileSystem;

/* The toolkit's plugin ID, for its memory hints (inferred) */
#define rwID_FILESYSTEM 0x1BE

/* The toolkit's results (inferred) */
#define RTFS_ERROR_NOERROR 1
#define RTFS_ERROR_TOOMANYFS 2       /* the manager has no room for another file system */
#define RTFS_ERROR_FSNAMEINUSE 6     /* a file system of that name is registered */
#define RTFS_ERROR_DEVICENAMEINUSE 7 /* a file system of that device name is registered */

/* The manager's errors, passed to its error callback (inferred) */
#define RTFSM_ERROR_NOFILESYSTEM 4 /* opening a file without a file system */
#define RTFSM_ERROR_TOOMANYFS 5    /* registering more file systems than it has room for */
#define RTFSM_ERROR_FSNOTFOUND 6   /* no file system of that name or device */
#define RTFSM_ERROR_FSREGISTERED 7 /* a file system of that name is registered */
#define RTFSM_ERROR_NOFREEFILE 8   /* the file system has no free file */

/* A file's states (inferred) */
#define RTFILE_STATUS_FREE 1    /* not open */
#define RTFILE_STATUS_READY 2   /* open, with nothing pending */
#define RTFILE_STATUS_PENDING 3 /* while an operation is pending */
#define RTFILE_STATUS_ERROR 4   /* the last operation failed */
#define RTFILE_STATUS_OPENING 5 /* taken by an open in progress */

/* A file's asynchronous operation states (inferred) */
#define RTFILE_ASYNC_IDLE 1
#define RTFILE_ASYNC_OPEN 2
#define RTFILE_ASYNC_SETPOSITION 4 /* a seek; reports the new position when it completes */
#define RTFILE_ASYNC_READ 5        /* a read; reports the bytes read when it completes */

/* The flags a file is opened with. Only the meanings of 4 and 8 are inferred and not established:
 * the "w", "a" and "r+" mode strings set them in combinations that do not fix their names. */
typedef enum RtFileAccessFlag {
    RTFILE_MODE_READ = 0x1,
    RTFILE_MODE_WRITE = 0x2,
    RTFILE_MODE_FLAG4 = 0x4,
    RTFILE_MODE_FLAG8 = 0x8,
    RTFILE_MODE_ASYNC = 0x10 /* completion is reported to the file's callback */
} RtFileAccessFlag;

/* The origins of a file system's set-position function (inferred) */
#define RTFILE_POS_BEGIN 1
#define RTFILE_POS_CURRENT 2
#define RTFILE_POS_END 3

/* Called when a file's asynchronous operation completes */
typedef void (*RtFileCallBack)(RtFile *file, RwInt32 result, RwInt32 status, RwInt32 asyncStatus,
                               void *data);

/* A file of a file system */
struct RtFile {
    /* 0x00 */ RwInt64 length;
    /* 0x08 */ RwInt64 position;    /* -1 when unknown */
    /* 0x10 */ RwInt64 readStart;   /* the position a read started at */
    /* 0x18 */ RwBool field_0x18;   /* copied from its file system's asyncMode */
    /* 0x1C */ RwInt32 priority;    /* of its device reads; 2 once opened */
    /* 0x20 */ RwInt32 status;      /* RTFILE_STATUS_* */
    /* 0x24 */ RwInt32 field_0x24;  /* 1 once opened, 2 after a refused write */
    /* 0x28 */ RwInt32 asyncStatus; /* RTFILE_ASYNC_* */
    /* 0x2C */ RwInt32 asyncResult; /* what an operation other than a seek reports */
    /* 0x30 */ RtFileCallBack callBack;
    /* 0x34 */ void *callBackData;
    /* 0x38 */ RtFileSystem *fileSystem;
};
CHECK_SIZEOF(struct RtFile, 0x40);

/* A file system (0x64 bytes) */
struct RtFileSystem {
    /* 0x00 */ RtFileSystem *next; /* in the manager's list */
    /* 0x04 */ RwInt32 maxNbOpenFiles;
    /* 0x08 */ RwInt32 field_0x8;
    /* 0x0C */ RwChar *deviceName;
    /* 0x10 */ RwBool asyncMode; /* its files open asynchronously, reporting to their callbacks */
    /* 0x14 */ RtFile *(*getFile)(RtFileSystem *fileSystem, RwInt32 index);
    /* 0x18 */ void (*close)(RtFileSystem *fileSystem);
    /* 0x1C */ const RwChar *(*getPathPrefix)(RtFileSystem *fileSystem); /* put before paths */
    /* 0x20 */ void (*registerNotify)(RwChar *name);   /* told the name when registered */
    /* 0x24 */ void (*unregisterNotify)(RwChar *name); /* told the name when unregistered */
    /* 0x28 */ RwInt32 (*fileOpen)(RtFileSystem *fileSystem, RtFile *file, const RwChar *name,
                                   RwUInt32 mode);
    /* 0x2C */ void (*fileClose)(RtFile *file);
    /* 0x30 */ RwUInt32 (*fileRead)(RtFile *file, void *buffer, RwUInt32 length);
    /* 0x34 */ RwUInt32 (*fileWrite)(RtFile *file, const void *buffer, RwUInt32 length);
    /* 0x38 */ RwInt64 (*fileSetPosition)(RtFile *file, RwInt32 offset,
                                          RwInt32 origin);          /* the new position */
    /* 0x3C */ RwInt32 (*fileUpdate)(RtFile *file, RwInt32 unused); /* polls; its status */
    /* 0x40 */ void (*fileAbort)(RtFile *file);
    /* 0x44 */ RwBool (*fileIsEOF)(RtFile *file);
    /* 0x48 */ RwInt32 (*fileGetStatus)(RtFile *file);
    /* 0x4C */ RwBool (*fileExists)(RtFileSystem *fileSystem, const RwChar *name);
    /* 0x50 */ RwChar name[8];
    /* 0x58 */ void *files; /* the implementation's array of its files */
    /* 0x5C */ void (*errorCallBack)(RwInt32 error, void *data); /* told of device errors */
    /* 0x60 */ void *errorCallBackData;
};
CHECK_SIZEOF(struct RtFileSystem, 0x64);

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
void fn_804799B8(RtFileSystem *fileSystem, RwBool asyncMode);            /* sets asyncMode */
RwBool fn_804799C0(RtFileSystem *fileSystem);                            /* gets asyncMode */
void fn_804799C8(RtFile *file); /* completes an asynchronous operation */

/* The file system manager (0x18 bytes) */
typedef struct RtFSManager {
    /* 0x00 */ RtFileSystem *fileSystems;
    /* 0x04 */ RwInt32 maxNbFS; /* 0 for no limit */
    /* 0x08 */ RwInt32 nbFS;
    /* 0x0C */ RwBool isInit;
    /* 0x10 */ RwInt32 lastError;
    /* 0x14 */ void (*errorCallBack)(RwInt32 error); /* RTFSM_ERROR_* */
} RtFSManager;
CHECK_SIZEOF(RtFSManager, 0x18);

extern RtFSManager lbl_805E2A40;
extern RtFileSystem *lbl_805F1548; /* the default file system */

/* the file system manager (inferred) */
RwBool fn_80479A50(RwInt32 maxNbFS);           /* opens */
void fn_80479A80(void);                        /* closes, unregistering every file system */
RwBool fn_80479B48(void);                      /* whether another one fits */
RwBool fn_80479BAC(RtFileSystem *fileSystem);  /* registers */
RtFileSystem *fn_80479C74(const RwChar *name); /* by name, or NULL */
RwBool fn_80479D08(const RwChar *name);        /* whether a name is in use */
RtFile *fn_80479DCC(RtFileSystem *fileSystem, const RwChar *name, RtFileAccessFlag mode,
                    RtFileCallBack callBack, void *callBackData); /* opens a file */
RtFile *fn_80479F50(const RwChar *name, RtFileAccessFlag mode, RtFileCallBack callBack,
                    void *callBackData);             /* opens a file by its path */
RtFSManager *fn_80479FC4(void);                      /* the manager */
RtFileSystem *fn_80479FD0(const RwChar *deviceName); /* by device name, or NULL */
RtFileSystem *fn_8047A044(const RwChar *path);       /* of a path, or the default */
void fn_8047A168(RtFileSystem *fileSystem);          /* sets the default */
RtFileSystem *fn_8047A170(void);                     /* the default */

#ifdef __cplusplus
}
#endif

#endif
