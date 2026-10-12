/* CRI CVFS (virtual file system) front end (cri_cvfs.c, CVFS/GC Ver.2.39): a table of named
 * devices, a default device, file handles pairing a device table with the device's handle,
 * and the "DEV:path" name split.
 *
 * Bodies follow the RE4 GameCube decompilation of Ver.2.37. Only cvFsEntryErrFunc,
 * cvFsGetFileSize, cvFsGetStat, cvFsExecServer, cvFsStopTr, cvFsReqRd, cvFsSeek, cvFsTell,
 * cvFsClose, cvFsOpen, cvFsSetDefDev, cvFsAddDev and cvFsCallUsrErrFn are referenced and
 * survive linking; they are verified against this game. Every other function here
 * (initialisation, volumes, directories, option functions, write, the remaining queries and
 * the out-of-line copies of the static helpers) is stripped by the linker: their bodies are
 * reference-derived and unverified, and only the error literals and storage order they
 * produce are checked.
 *
 * Several functions test `dev == 0` after splitting a name into the local array `dev`. The test is
 * always false, but it is the original source and keeps its error message in .rodata order.
 */
#include <string.h>
#include <stdio.h>

#include "CRI/cvfs.h"

#define CVFS_MAX_DEV 32
#define CVFS_MAX_HN 40
#define CVFS_NAME_LEN 0x129
#define CVFS_DEVNAME_LEN 12

typedef struct {
    CVFS_IF *vtbl;                /* 0x00 */
    Char8 name[CVFS_DEVNAME_LEN]; /* 0x04 */
} CVFS_DEV;

struct CVFS_OBJ {
    CVFS_IF *vtbl; /* 0x00 */
    void *hn;      /* 0x04 the device's handle */
};

CVFS_USRERRFN cvfs_errfn;
static void *cvfs_errobj;
Sint32 cvfs_init_cnt;
Char8 add_dev_tmp[CVFS_NAME_LEN];
Char8 cvfs_defdev[9];
static CVFS_DEV cvfs_tbl[CVFS_MAX_DEV];
CVFS_OBJ cvfs_obj[CVFS_MAX_HN];

static const Char8 *const volatile cvfs_build = "\nCVFS/GC Ver.2.39 Build:Feb 28 2005 15:48:34\n";

void cvFsCallUsrErrFn(void *obj, const Char8 *msg, void *hn);

/* dead */
void cvFsInit(void) {
    cvfs_build;
    cvfs_errfn = 0;
    cvfs_errobj = 0;
    if (cvfs_init_cnt == 0) {
        add_dev_tmp[0] = '\0';
        cvfs_defdev[0] = '\0';
        memset(cvfs_tbl, 0, sizeof(cvfs_tbl));
        memset(cvfs_obj, 0, sizeof(cvfs_obj));
    }
    cvfs_init_cnt++;
}

/* dead */
void cvFsFinish(void) {
    cvfs_init_cnt--;
}

/* the user error callback */
static void cvfs_Error(const Char8 *msg) {
    if (cvfs_errfn != 0) {
        cvfs_errfn(cvfs_errobj, msg, 0);
    }
}

/* upper-case a name (in place) */
static void cvfs_StrUpr(Char8 *s) {
    Uint32 i;
    Uint32 n;
    Char8 *p;

    n = strlen(s) + 1;
    p = s;
    for (i = 0; i < n; i++) {
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - 0x20;
        }
        p++;
    }
}

/* is the name a registered device (its first `len` characters) */
static Sint32 cvfs_IsExistDev(const Char8 *name, Sint32 len) {
    Sint32 i;
    CVFS_DEV *dev;

    for (i = 0; i < CVFS_MAX_DEV; i++) {
        dev = &cvfs_tbl[i];
        if (strncmp(name, dev->name, len) == 0) {
            return 1;
        }
    }
    return 0;
}

/* the device of a name (0: the default device) */
static CVFS_IF *cvFsGetDevIf(CVFS_DEV *tbl, const Char8 *name) {
    Uint32 i;
    Sint32 len;
    CVFS_DEV *dev;

    if (name == 0) {
        name = cvfs_defdev;
    }
    len = strlen(name);
    dev = tbl;
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (strncmp(name, dev->name, len) == 0) {
            return cvfs_tbl[i].vtbl;
        }
        dev++;
    }
    return 0;
}

/* Device search by table index. cvfs_ResolveDev's second and third searches must be index loops:
 * a pointer local (`dev = tbl`) gives the stepping pointer a different register than the original. */
static CVFS_IF *cvfs_SearchDev(CVFS_DEV *tbl, const Char8 *name) {
    Uint32 i;
    Sint32 len;

    len = strlen(name);
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (strncmp(name, tbl[i].name, len) == 0) {
            return cvfs_tbl[i].vtbl;
        }
    }
    return 0;
}

/* the device's option function (fnid 100: the device wants the "DEV:path" form) */
static Sint32 cvfs_OptFn(CVFS_IF *vtbl, void *hn, Sint32 fnid, Sint32 a, Sint32 b) {
    Sint32 ret;

    if (vtbl == 0) {
        ret = 0;
    } else if (vtbl->OptFn != 0) {
        ret = vtbl->OptFn(hn, fnid, a, b);
    } else {
        ret = 0;
    }
    return ret;
}

/* "DEV:path" -> upper-cased device name and path (a one-character device is a drive letter and
 * stays in the path; no ':' means no device) */
static inline void cvfs_SplitFname(const Char8 *fname, Char8 *dev, Char8 *path) {
    Sint32 i;
    Sint32 j;
    Sint32 n;

    if (fname == 0) {
        return;
    }
    for (i = 0; i < CVFS_NAME_LEN; i++) {
        if (fname[i] == ':' || fname[i] == '\0') {
            break;
        }
        dev[i] = fname[i];
    }
    if (fname[i] == '\0') {
        dev[i] = '\0';
        memcpy(path, dev, strlen(dev) + 1);
        dev[0] = '\0';
        return;
    }
    dev[i] = '\0';
    i++;
    if (i == 2) {
        i = 0;
        dev[0] = '\0';
    }
    n = i;
    for (j = i; j < CVFS_NAME_LEN; j++) {
        if (fname[j] == '\0') {
            break;
        }
        path[n++ - i] = fname[j];
    }
    path[n - i] = '\0';
    cvfs_StrUpr(dev);
}

/* the default device name into dev (empty when none) */
static void cvfs_GetDefDev(Char8 *dev) {
    Sint32 len;

    len = strlen(cvfs_defdev);
    if (cvfs_defdev[0] == '\0') {
        dev[0] = '\0';
    } else {
        memcpy(dev, cvfs_defdev, len + 1);
    }
}

/* The search loop of cvFsGetDevIf with the length computed by the caller. The original loads the
 * table base after the first strlen, so the name check and strlen stay in cvfs_ResolveDev and
 * `tbl = cvfs_tbl` follows them. */
static CVFS_IF *cvfs_FindDev(CVFS_DEV *tbl, const Char8 *name, Sint32 len) {
    CVFS_DEV *dev;
    Uint32 i;

    dev = tbl;
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (strncmp(name, dev->name, len) == 0) {
            return cvfs_tbl[i].vtbl;
        }
        dev++;
    }
    return 0;
}

/* Whether the named device asks for the "DEV:path" form (option 100). This first search sits one
 * inlining level deeper than the two cvfs_SearchDev copies, which gives its loop variables the
 * original register order. */
static Sint32 cvfs_WantsDevForm(CVFS_DEV *tbl, const Char8 *name, Sint32 len) {
    return cvfs_OptFn(cvfs_FindDev(tbl, name, len), 0, 100, 0, 0) == 1;
}

/* Finds the device table for a split file name: an empty device part takes the default device; a
 * device that asked for it (option 100) gets the "DEV:path" form back in `path`; an unknown device
 * name falls back to the default device with the whole original name as the path. */
static CVFS_IF *cvfs_ResolveDev(const Char8 *fname, Char8 *dev, Char8 *path) {
    CVFS_IF *vtbl;
    CVFS_DEV *tbl;
    const Char8 *name;
    Sint32 len;

    if (dev[0] == '\0') {
        cvfs_GetDefDev(dev);
        if (dev[0] == '\0') {
            return 0;
        }
    }
    name = dev;
    if (name == 0) {
        name = cvfs_defdev;
    }
    len = strlen(name);
    tbl = cvfs_tbl;
    if (cvfs_WantsDevForm(tbl, name, len)) {
        strcpy(add_dev_tmp, path);
        sprintf(path, "%s:%s", name,
                add_dev_tmp); /* the target passes the checked `name` (r26), not `dev` */
    }
    vtbl = cvfs_SearchDev(tbl, dev);
    if (vtbl == 0) {
        cvfs_GetDefDev(dev);
        vtbl = cvfs_SearchDev(tbl, dev);
        if (vtbl == 0) {
            vtbl = 0;
        } else {
            strcpy(path, fname);
        }
    }
    return vtbl;
}

/* dead */
void cvFsSetDefVol(const Char8 *devname, const Char8 *volname) {
    CVFS_IF *vtbl;

    if (devname == 0) {
        cvfs_Error("cvFsSetDefVol #1:illegal device name");
        return;
    }
    if (volname == 0) {
        cvfs_Error("cvFsSetDefVol #2:illegal volume name");
        return;
    }
    vtbl = cvfs_SearchDev(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsSetDefVol #3:device not found");
        return;
    }
    cvfs_OptFn(vtbl, 0, 1, (Sint32)volname, 0);
}

/* dead */
Sint32 cvFsGetVolumeInfo(const Char8 *devname, const Char8 *volname, void *inf) {
    CVFS_IF *vtbl;

    if (devname == 0) {
        cvfs_Error("cvFsGetVolumeInfo #1:illegal device name");
        return 0;
    }
    if (volname == 0) {
        cvfs_Error("cvFsGetVolumeInfo #2:illegal volume name");
        return 0;
    }
    vtbl = cvfs_SearchDev(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsGetVolumeInfo #3:device not found");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 2, (Sint32)volname, (Sint32)inf);
}

/* dead */
void cvFsDelVolume(const Char8 *devname, const Char8 *volname) {
    CVFS_IF *vtbl;

    if (devname == 0) {
        cvfs_Error("cvFsDelVolume #1:illegal device name");
        return;
    }
    if (volname == 0) {
        cvfs_Error("cvFsDelVolume #2:illegal volume name");
        return;
    }
    vtbl = cvfs_SearchDev(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsDelVolume #3:device not found");
        return;
    }
    cvfs_OptFn(vtbl, 0, 3, (Sint32)volname, 0);
}

/* dead */
void cvFsAddVolumeEx(const Char8 *devname, const Char8 *volname, void *img) {
    CVFS_IF *vtbl;

    if (devname == 0) {
        cvfs_Error("cvFsAddVolumeEx #1:illegal device name");
        return;
    }
    if (volname == 0) {
        cvfs_Error("cvFsAddVolumeEx #2:illegal volume name");
        return;
    }
    if (img == 0) {
        cvfs_Error("cvFsAddVolumeEx #3:illegal image handle");
        return;
    }
    vtbl = cvfs_SearchDev(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsAddVolumeEx #3:device not found");
        return;
    }
    cvfs_OptFn(vtbl, 0, 4, (Sint32)volname, (Sint32)img);
}

/* dead */
void cvFsSetCurVolume(const Char8 *devname, void *img) {
    CVFS_IF *vtbl;

    if (devname == 0) {
        cvfs_Error("cvFsSetCurVolume #1:illegal device name");
        return;
    }
    if (img == 0) {
        cvfs_Error("cvFsSetCurVolume #2:illegal image handle");
        return;
    }
    vtbl = cvfs_SearchDev(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsSetCurVolume #3:device not found");
        return;
    }
    cvfs_OptFn(vtbl, 0, 5, (Sint32)img, 0);
}

/* dead */
Sint32 cvFsOptFn2(CVFS_OBJ *obj, Sint32 fnid, Sint32 a, Sint32 b) {
    if (obj == 0) {
        cvfs_Error("cvFsOptFn2 #1:handle error");
        return 0;
    }
    if (obj->vtbl->OptFn == 0) {
        cvfs_Error("cvFsOptFn2 #2:vtbl error");
        return 0;
    }
    return obj->vtbl->OptFn(obj->hn, fnid, a, b);
}

/* dead */
Sint32 cvFsOptFn1(CVFS_OBJ *obj, Sint32 fnid, Sint32 a) {
    if (obj == 0) {
        cvfs_Error("cvFsOptFn1 #1:handle error");
        return 0;
    }
    if (obj->vtbl->OptFn == 0) {
        cvfs_Error("cvFsOptFn1 #2:vtbl error");
        return 0;
    }
    return obj->vtbl->OptFn(obj->hn, fnid, a, 0);
}

/* dead */
const Char8 *cvFsGetDevName(CVFS_OBJ *obj) {
    Sint32 i;

    if (obj == 0 || obj->vtbl == 0) {
        cvfs_Error("cvFsGetDevName #1:vtbl error");
        return 0;
    }
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (cvfs_tbl[i].vtbl == obj->vtbl) {
            return cvfs_tbl[i].name;
        }
    }
    return 0;
}

/* dead */
Sint32 cvFsDeleteFile(const Char8 *fname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    if (fname == 0) {
        cvfs_Error("cvFsDeleteFile #1:illegal file name");
        return 0;
    }
    cvfs_SplitFname(fname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsDeleteFile #2:illegal device name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(fname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsDeleteFile #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsDeleteFile #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 10, (Sint32)path, 0);
}

/* dead */
Sint32 cvFsRemoveDir(const Char8 *dname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    if (dname == 0) {
        cvfs_Error("cvFsRemoveDir #1:illegal directory name");
        return 0;
    }
    cvfs_SplitFname(dname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsRemoveDir #2:illegal device name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(dname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsRemoveDir #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsRemoveDir #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 11, (Sint32)path, 0);
}

/* dead */
Sint32 cvFsMakeDir(const Char8 *dname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    if (dname == 0) {
        cvfs_Error("cvFsMakeDir #1:illegal directory name");
        return 0;
    }
    cvfs_SplitFname(dname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsMakeDir #2:illegal device name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(dname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsMakeDir #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsMakeDir #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 12, (Sint32)path, 0);
}

/* dead */
Sint32 cvFsGetMaxByteRate(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsGetMaxByteRate #1:handle error");
        return 0;
    }
    if (obj->vtbl->OptFn == 0) {
        cvfs_Error("cvFsGetMaxByteRate #2:vtbl error");
        return 0;
    }
    return obj->vtbl->OptFn(obj->hn, 13, 0, 0);
}

/* dead */
Sint32 cvFsIsExistFile(const Char8 *fname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    cvfs_SplitFname(fname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsIsExistFile #2:illegal device name");
        return 0;
    }
    if (fname == 0) {
        cvfs_Error("cvFsIsExistFile #1:illegal file name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(fname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsIsExistFile #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsIsExistFile #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 14, (Sint32)path, 0);
}

/* dead */
Sint32 cvFsChangeDir(const Char8 *dname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    if (dname == 0) {
        cvfs_Error("cvFsChangeDir #1:illegal directory name");
        return 0;
    }
    cvfs_SplitFname(dname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsChangeDir #2:illegal device name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(dname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsChangeDir #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsChangeDir #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 15, (Sint32)path, 0);
}

/* dead */
Sint32 cvFsGetNumTr(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsGetNumTr #1:handle error");
        return 0;
    }
    if (obj->vtbl->GetNumTr == 0) {
        cvfs_Error("cvFsGetNumTr #2:vtbl error");
        return 0;
    }
    return obj->vtbl->GetNumTr(obj->hn);
}

/* dead */
void cvFsSetSctLen(CVFS_OBJ *obj, Sint32 sctlen) {
    if (obj == 0) {
        cvfs_Error("cvFsSetSctLen #3:handle error");
        return;
    }
    if (obj->vtbl->SetSctLen == 0) {
        cvfs_Error("cvFsSetSctLen #4:vtbl error");
        return;
    }
    obj->vtbl->SetSctLen(obj->hn, sctlen);
}

/* dead */
Sint32 cvFsGetSctLen(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsGetSctLen #1:handle error");
        return 0;
    }
    if (obj->vtbl->GetSctLen == 0) {
        cvfs_Error("cvFsGetSctLen #2:vtbl error");
        return 0;
    }
    return obj->vtbl->GetSctLen(obj->hn);
}

/* dead */
Sint32 cvFsGetFreeSize(const Char8 *devname) {
    CVFS_IF *vtbl;

    vtbl = cvFsGetDevIf(cvfs_tbl, devname);
    if (vtbl == 0) {
        cvfs_Error("cvFsGetFreeSize #5:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsGetFreeSize #6:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 16, 0, 0);
}

/* dead */
Sint32 cvFsGetFileSizeByHndl(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsGetFileSizeByHndl #1:illegal file handle");
        return 0;
    }
    return cvfs_OptFn(obj->vtbl, obj->hn, 17, 0, 0);
}

/* dead */
Sint32 cvFsGetFileSizeEx(const Char8 *fname, Sint32 *size) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    CVFS_IF *vtbl;

    if (fname == 0) {
        cvfs_Error("cvFsGetFileSizeEx #1:illegal file name");
        return 0;
    }
    cvfs_SplitFname(fname, dev, path);
    if (dev == 0) {
        cvfs_Error("cvFsGetFileSizeEx #2:illegal device name");
        return 0;
    }
    vtbl = cvfs_ResolveDev(fname, dev, path);
    if (vtbl == 0) {
        cvfs_Error("cvFsGetFileSizeEx #3:device not found");
        return 0;
    }
    if (vtbl->x64 == 0) {
        cvfs_Error("cvFsGetFileSizeEx #4:vtbl error");
        return 0;
    }
    return cvfs_OptFn(vtbl, 0, 18, (Sint32)path, (Sint32)size);
}

/* Installs the user error callback (func(obj, msg, hn)); ADXGC_SetupDvdFs installs adxgc_err_dvd. */
void cvFsEntryErrFunc(CVFS_USRERRFN func, void *obj) {
    if (func == 0) {
        cvfs_errfn = 0;
        cvfs_errobj = 0;
    } else {
        cvfs_errfn = func;
        cvfs_errobj = obj;
    }
}

/* Matching constraint: the original allocates `pdev` before the device table; the codeless move
 * through r11 keeps pdev live long enough to get that register order (COMPILER-DIFF M1; no code). */
Sint32 cvFsGetFileSize(const Char8 *fname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    register Char8 *pdev;
    CVFS_IF *vtbl;

    if (fname == 0) {
        cvfs_Error("cvFsGetFileSize #1:illegal file name");
        return 0;
    }
    cvfs_SplitFname(fname, dev, path);
    if (path[0] == '\0') {
        cvfs_Error("cvFsGetFileSize #1:illegal file name");
        return 0;
    }
    pdev = dev;
    asm { mr r11, pdev; mr pdev, r11 } /* COMPILER-DIFF: M1 (level-shifter, no code) */
    vtbl = cvfs_ResolveDev(fname, pdev, path);
    if (dev == 0) {
        cvfs_Error("cvFsGetFileSize #2:illegal device name");
    }
    if (vtbl == 0) {
        cvfs_Error("cvFsGetFileSize #3:device not found");
    }
    if (vtbl->GetFileSize != 0) {
        return vtbl->GetFileSize(path);
    }
    cvfs_Error("cvFsGetFileSize #4:vtbl error");
    return 0;
}

/* Transfer state of a file handle from its device: 1 complete, 2 reading, 3 error (also for a bad
 * handle). */
Sint32 cvFsGetStat(CVFS_OBJ *obj) {
    Sint32 stat = 3;

    if (obj == 0) {
        cvfs_Error("cvFsGetStat #1:handle error");
        return 3;
    }
    if (obj->vtbl->GetStat != 0) {
        stat = obj->vtbl->GetStat(obj->hn);
    } else {
        cvfs_Error("cvFsGetStat #2:vtbl error");
    }
    return stat;
}

/* Runs every registered device's server (gcCiExecServer completes the DVD reads). Called twice per
 * ADXT_ExecFsSvr. */
void cvFsExecServer(void) {
    Sint32 i;
    CVFS_DEV *dev;

    for (i = 0; i < CVFS_MAX_DEV; i++) {
        dev = &cvfs_tbl[i];
        if (dev->vtbl != 0 && dev->vtbl->ExecServer != 0) {
            dev->vtbl->ExecServer();
        }
    }
}

/* Cancels the handle's transfer on its device (DVDCancel for GCD). */
void cvFsStopTr(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsStopTr #1:handle error");
        return;
    }
    if (obj->vtbl->StopTr != 0) {
        obj->vtbl->StopTr(obj->hn);
    } else {
        cvfs_Error("cvFsStopTr #2:vtbl error");
    }
}

/* dead */
Sint32 cvFsReqWr(CVFS_OBJ *obj, Sint32 nsct, void *buf) {
    Sint32 ret;

    if (obj == 0) {
        cvfs_Error("cvFsReqWr #1:handle error");
        return 0;
    }
    if (obj->vtbl->x64 != 0) {
        ret = obj->vtbl->OptFn(obj->hn, 20, nsct, (Sint32)buf);
    } else {
        ret = 0;
        cvfs_Error("cvFsReqWr #2:vtbl error");
    }
    return ret;
}

/* Requests `nsct` sectors into `buf` from the device; returns the sectors accepted (0 = busy/refused). */
Sint32 cvFsReqRd(CVFS_OBJ *obj, Sint32 nsct, void *buf) {
    Sint32 ret;

    if (obj == 0) {
        cvfs_Error("cvFsReqRd #1:handle error");
        return 0;
    }
    if (obj->vtbl->ReqRd != 0) {
        ret = obj->vtbl->ReqRd(obj->hn, nsct, buf);
    } else {
        ret = 0;
        cvfs_Error("cvFsReqRd #2:vtbl error");
    }
    return ret;
}

/* Seeks in sectors (type 0 set, 1 cur, 2 end); returns the new position. */
Sint32 cvFsSeek(CVFS_OBJ *obj, Sint32 pos, Sint32 type) {
    Sint32 ret;

    if (obj == 0) {
        cvfs_Error("cvFsSeek #1:handle error");
        return 0;
    }
    if (obj->vtbl->Seek != 0) {
        ret = obj->vtbl->Seek(obj->hn, pos, type);
    } else {
        ret = 0;
        cvfs_Error("cvFsSeek #2:vtbl error");
    }
    return ret;
}

/* Current position in sectors. */
Sint32 cvFsTell(CVFS_OBJ *obj) {
    Sint32 ret;

    if (obj == 0) {
        cvfs_Error("cvFsTell #1:handle error");
        return 0;
    }
    if (obj->vtbl->Tell != 0) {
        ret = obj->vtbl->Tell(obj->hn);
    } else {
        ret = 0;
        cvfs_Error("cvFsTell #2:vtbl error");
    }
    return ret;
}

/* Closes the device handle and frees the CVFS slot. */
void cvFsClose(CVFS_OBJ *obj) {
    if (obj == 0) {
        cvfs_Error("cvFsClose #1:handle error");
        return;
    }
    if (obj->vtbl->Close != 0) {
        obj->vtbl->Close(obj->hn);
        obj->hn = 0;
        obj->vtbl = 0;
    } else {
        cvfs_Error("cvFsClose #2:vtbl error");
    }
}

/* a free handle slot */
static CVFS_OBJ *cvfs_AllocObj(void) {
    /* declared before i for the original register order (matching constraint) */
    CVFS_OBJ *obj;
    Sint32 i;

    obj = cvfs_obj;
    for (i = 0; i < CVFS_MAX_HN; i++) {
        if (obj->hn == 0) {
            break;
        }
        obj++;
    }
    obj = &cvfs_obj[i];
    if (i == CVFS_MAX_HN) {
        obj = 0;
    }
    return obj;
}

/* Opens `fname` (see cvfs_ResolveDev for the device choice) and returns a handle, 0 on error. */
CVFS_OBJ *cvFsOpen(const Char8 *fname, void *dir, Sint32 rw) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    Char8 *pdev;
    CVFS_OBJ *obj;
    CVFS_IF *vtbl;
    void *hn;

    if (fname == 0) {
        cvfs_Error("cvFsOpen #1:illegal file name");
        return 0;
    }
    cvfs_SplitFname(fname, dev, path);
    if (path[0] == '\0') {
        cvfs_Error("cvFsOpen #1:illegal file name");
        return 0;
    }
    /* allocated through a `void *` and copied: the original keeps the copy (`mr r30, r3`) */
    hn = cvfs_AllocObj();
    obj = hn;
    if (obj == 0) {
        cvfs_Error("cvFsOpen #3:failed handle alloced");
        return 0;
    }
    pdev = dev;
    vtbl = cvfs_ResolveDev(fname, pdev, path);
    obj->vtbl = vtbl;
    if (dev == 0) {
        obj->hn = 0;
        obj->vtbl = 0;
        cvfs_Error("cvFsOpen #2:illegal device name");
        return 0;
    }
    if (obj->vtbl == 0) {
        obj->hn = 0;
        obj->vtbl = 0;
        cvfs_Error("cvFsOpen #4:device not found");
        return 0;
    }
    if (obj->vtbl->Open !=
        0) { /* the target reloads obj->vtbl (r3) for the Open call, not `vtbl` */
        obj->hn = obj->vtbl->Open(path, dir, rw);
    } else {
        obj->hn = 0;
        obj->vtbl = 0;
        cvfs_Error("cvFsOpen #5:vtbl error");
        return 0;
    }
    if (obj->hn == 0) {
        obj->hn = 0;
        obj->vtbl = 0;
        cvfs_Error("cvFsOpen #6:open failed");
        return 0;
    }
    return obj;
}

/* Sets the device used for names without a "DEV:" prefix (upper-cased, must be registered); "GCD"
 * in this game. */
void cvFsSetDefDev(Char8 *devname) {
    Sint32 len;

    if (devname == 0) {
        cvfs_Error("cvFsSetDefDev #1:illegal device name");
        return;
    }
    len = strlen(devname);
    if (len == 0) {
        cvfs_defdev[0] = '\0';
        return;
    }
    cvfs_StrUpr(devname);
    if (cvfs_IsExistDev(devname, len) == 1) {
        memcpy(cvfs_defdev, devname, len + 1);
    } else {
        cvfs_Error("cvFsSetDefDev #2:unknown device name");
    }
}

/* dead */
void cvFsDelDev(const Char8 *devname) {
    Sint32 i;

    if (devname == 0) {
        cvfs_Error("cvFsDelDev #1:illegal device name");
        return;
    }
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (strcmp(devname, cvfs_tbl[i].name) == 0) {
            cvfs_tbl[i].vtbl = 0;
            cvfs_tbl[i].name[0] = '\0';
        }
    }
}

/* Registers the device in the table unless its name is already there; 0 when the table is full.
 * The interface arrives as `void *` and is converted once: the original keeps that copy of getif's
 * result (`mr r28, r3`). */
static CVFS_IF *cvfs_AddDevTbl(Char8 *devname, void *vt) {
    CVFS_DEV *dev;
    Sint32 i;
    CVFS_IF *vtbl = vt;

    if (cvfs_SearchDev(cvfs_tbl, devname) != 0) {
        return vtbl;
    }
    dev = cvfs_tbl;
    for (i = 0; i < CVFS_MAX_DEV; i++) {
        if (dev->name[0] == '\0') {
            break;
        }
        dev++;
    }
    if (i == CVFS_MAX_DEV) {
        return 0;
    }
    dev = &cvfs_tbl[i];
    dev->vtbl = vtbl;
    memcpy(dev->name, devname, strlen(devname) + 1);
    return vtbl;
}

/* Registers a device by name (upper-cased) with its xxCiGetInterface function, and points the
 * device's error callback at cvFsCallUsrErrFn. Called for "MFS" and "GCD" by ADXGC_SetupDvdFs,
 * which passes 0 for the device parameter prm (unused here). */
void cvFsAddDev(Char8 *devname, CVFS_GETIFFN getif, void *prm) {
    Char8 *name;
    CVFS_IF *vtbl;
    CVFS_GETIFFN fn;

    cvfs_build;
    /* the original keeps copies of both parameters (devname above the table entry in register
     * order; the getif copy fixes the order of the two pool loads); the casts make them kept copies */
    name = (Char8 *)(void *)devname;
    fn = (CVFS_GETIFFN)(void *)getif;
    if (name == 0) {
        cvfs_Error("cvFsAddDev #1:illegal device name");
        return;
    }
    if (fn == 0) {
        cvfs_Error("cvFsAddDev #2:illegal I/F func name");
        return;
    }
    cvfs_StrUpr(name);
    vtbl = cvfs_AddDevTbl(name, fn());
    if (vtbl == 0) {
        cvfs_Error("cvFsAddDev #3:failed added a device");
        return;
    }
    if (vtbl->EntryErrFunc != 0) {
        vtbl->EntryErrFunc(cvFsCallUsrErrFn, 0);
    }
}

/* the devices' error callback: forwards to the user's */
void cvFsCallUsrErrFn(void *obj, const Char8 *msg, void *hn) {
    if (cvfs_errfn != 0) {
        cvfs_errfn(cvfs_errobj, msg, hn);
    }
}
