#ifndef CRI_CVFS_H
#define CRI_CVFS_H

/* CRI CVFS virtual file system (cri_cvfs.c) entry points used by other CRI units. A device
 * registers its interface table through its xxCiGetInterface function.
 */
#include "CRI/cri_xpt.h"
#include "layout_check.h"

typedef struct CVFS_IF CVFS_IF;
typedef struct CVFS_OBJ CVFS_OBJ;

typedef void (*CVFS_USRERRFN)(void *obj, const Char8 *msg, void *hn);
typedef CVFS_IF *(*CVFS_GETIFFN)(void);

/* seek origins of the Seek entry */
#define CVFS_SEEK_SET 0
#define CVFS_SEEK_CUR 1
#define CVFS_SEEK_END 2

/* device interface table: every device (mfci.c, gcci.c) fills one, and cri_cvfs.c calls through
 * it */
struct CVFS_IF {
    void (*ExecServer)(void);                                   /* 0x00 */
    void (*EntryErrFunc)(CVFS_USRERRFN func, void *obj);        /* 0x04 */
    Sint32 (*GetFileSize)(const Char8 *fname);                  /* 0x08 */
    void *x0c;                                                  /* 0x0C */
    void *(*Open)(const Char8 *fname, void *dir, Sint32 rw);    /* 0x10 */
    void (*Close)(void *hn);                                    /* 0x14 */
    Sint32 (*Seek)(void *hn, Sint32 pos, Sint32 type);          /* 0x18 */
    Sint32 (*Tell)(void *hn);                                   /* 0x1C */
    Sint32 (*ReqRd)(void *hn, Sint32 nsct, void *buf);          /* 0x20 */
    void *x24;                                                  /* 0x24 */
    void (*StopTr)(void *hn);                                   /* 0x28 */
    Sint32 (*GetStat)(void *hn);                                /* 0x2C */
    Sint32 (*GetSctLen)(void *hn);                              /* 0x30 */
    void (*SetSctLen)(void *hn, Sint32 sctlen);                 /* 0x34 */
    Sint32 (*GetNumTr)(void *hn);                               /* 0x38 */
    void *x3c[9];                                               /* 0x3C */
    Sint32 (*OptFn)(void *hn, Sint32 fnid, Sint32 a, Sint32 b); /* 0x60 (mfci.c only) */
    void *x64;                                                  /* 0x64 */
};
CHECK_SIZEOF(struct CVFS_IF, 0x68);

#ifdef __cplusplus
extern "C" {
#endif

/* cri_cvfs.c */
void cvFsEntryErrFunc(CVFS_USRERRFN func, void *obj);
CVFS_OBJ *cvFsOpen(const Char8 *fname, void *dir, Sint32 rw);
void cvFsClose(CVFS_OBJ *obj);
Sint32 cvFsSeek(CVFS_OBJ *obj, Sint32 pos, Sint32 type);
Sint32 cvFsTell(CVFS_OBJ *obj);
Sint32 cvFsReqRd(CVFS_OBJ *obj, Sint32 nsct, void *buf);
void cvFsStopTr(CVFS_OBJ *obj);
Sint32 cvFsGetStat(CVFS_OBJ *obj);
Sint32 cvFsGetFileSize(const Char8 *fname);
void cvFsExecServer(void);
void cvFsSetDefDev(Char8 *devname);
void cvFsAddDev(Char8 *devname, CVFS_GETIFFN getif, void *prm);

/* device interfaces: mfci.c (memory files), gcci.c (DVD) */
CVFS_IF *mfCiGetInterface(void);
CVFS_IF *gcCiGetInterface(void);
void gcCiSetRdMode(void *obj, Sint32 a, Sint32 b, Sint32 mode);

#ifdef __cplusplus
}
#endif

#endif
