#ifndef CRI_CVFS_H
#define CRI_CVFS_H

/* CRI CVFS virtual file system (cri_cvfs.c) entry points used by other CRI units. A device
 * registers its interface table through its xxCiGetInterface function.
 */
#include "CRI/cri_xpt.h"

typedef struct CVFS_IF CVFS_IF;
typedef struct CVFS_OBJ CVFS_OBJ;

typedef void (*CVFS_USRERRFN)(void *obj, const Char8 *msg, void *hn);
typedef CVFS_IF *(*CVFS_GETIFFN)(void);

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
