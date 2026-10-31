/* CRI CVFS (virtual file system) front end (cri_cvfs.c, CVFS/GC Ver.2.39): a table of named
 * devices, a default device, file handles pairing a device table with the device's handle,
 * and the "DEV:path" name split.
 *
 * Source from the RE4 GameCube decompilation. Only cvFsEntryErrFunc, cvFsGetFileSize,
 * cvFsGetStat, cvFsExecServer, cvFsStopTr, cvFsReqRd, cvFsSeek, cvFsTell, cvFsClose, cvFsOpen,
 * cvFsSetDefDev, cvFsAddDev and cvFsCallUsrErrFn are kept in the game. The other public
 * functions (initialisation, volumes, directories, option functions, write and the remaining
 * queries) are stripped, so their bodies are unknown; their error messages remain in .rodata,
 * between those of the kept functions. MWCC places literals and .bss objects in order of first
 * use, so never-called stand-ins at the stripped functions' positions refer to the messages in
 * order and, in place of cvFsInit and cvFsFinish, to the .bss objects in storage order (storing
 * the addresses keeps the references).
 *
 * cvFsGetFileSize and cvFsOpen test `dev == 0` after splitting a name into the local array `dev`.
 * The test can never be true, but the game's code performs it (it compares the array's stack
 * address with 0).
 */
#include <string.h>
#include <stdio.h>

#include "CRI/cvfs.h"
#include "layout_check.h"

#define CVFS_MAX_DEV 32
#define CVFS_MAX_HN 40
#define CVFS_NAME_LEN 0x129
#define CVFS_DEVNAME_LEN 12

typedef struct {
    CVFS_IF *vtbl;                /* 0x00 */
    Char8 name[CVFS_DEVNAME_LEN]; /* 0x04 */
} CVFS_DEV;
CHECK_SIZEOF(CVFS_DEV, 0x10);

struct CVFS_OBJ {
    CVFS_IF *vtbl; /* 0x00 */
    void *hn;      /* 0x04 the device's handle */
};
CHECK_SIZEOF(struct CVFS_OBJ, 0x8);

CVFS_USRERRFN cvfs_errfn;
static void *cvfs_errobj;
Sint32 cvfs_init_cnt;
Char8 add_dev_tmp[CVFS_NAME_LEN];
Char8 cvfs_defdev[9];
static CVFS_DEV cvfs_tbl[CVFS_MAX_DEV];
CVFS_OBJ cvfs_obj[CVFS_MAX_HN];

/* Matching workaround: volatile. The original's cvFsAddDev loads the build string pointer and
 * discards it; without volatile MWCC drops the load (cvFsAddDev 0x27C -> 0x278 bytes). Revisit when
 * the original's form of the dead load is known. */
static const Char8 *const volatile cvfs_build = "\nCVFS/GC Ver.2.39 Build:Feb 28 2005 15:48:34\n";

void cvFsCallUsrErrFn(void *obj, const Char8 *msg, void *hn);

/* Matching workaround: order-only stand-in for the stripped cvFsInit and cvFsFinish (their bodies
 * are unknown). MWCC places .bss objects in order of first reference, so this never-called function
 * refers to them in storage order before any other code does. Storing the addresses keeps the
 * references. Revisit when the stripped functions' bodies are known. */
static void cvfs_BssOrder(void **out) {
    out[0] = &cvfs_errfn;
    out[1] = &cvfs_errobj;
    out[2] = &cvfs_init_cnt;
    out[3] = add_dev_tmp;
    out[4] = cvfs_defdev;
    out[5] = cvfs_tbl;
    out[6] = cvfs_obj;
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

/* Device search by name: returns the interface of the global table entry at the index of the first
 * matching name in tbl. */
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

/* Device search with the length computed by the caller (name inferred).
 * Matching workaround: the helper split of cvfs_FindDev, cvfs_WantsDevForm and cvfs_ResolveDev is
 * a reconstruction driven by the original's load order: the original loads the table base after
 * the first strlen, so the name check and strlen stay in cvfs_ResolveDev and `tbl = cvfs_tbl`
 * follows them. Revisit when the original's helper functions are known. */
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

/* Whether the named device asks for the "DEV:path" form (option 100) (name inferred). */
static Sint32 cvfs_WantsDevForm(CVFS_DEV *tbl, const Char8 *name, Sint32 len) {
    return cvfs_OptFn(cvfs_FindDev(tbl, name, len), 0, 100, 0, 0) == 1;
}

/* Finds the device table for a split file name (name inferred): an empty device part takes the
 * default device; a device that asked for it (option 100) gets the "DEV:path" form back in `path`;
 * an unknown device name falls back to the default device with the whole original name as the
 * path. */
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
        sprintf(path, "%s:%s", name, add_dev_tmp);
    }
    vtbl = cvfs_SearchDev(tbl, dev);
    if (vtbl == 0) {
        cvfs_GetDefDev(dev);
        vtbl = cvfs_SearchDev(tbl, dev);
        if (vtbl == 0) {
            return 0;
        }
        strcpy(path, fname);
    }
    return vtbl;
}

/* Matching workaround: as above, order-only stand-in for the stripped cvFsSetDefVol,
 * cvFsGetVolumeInfo, cvFsDelVolume, cvFsAddVolumeEx and cvFsSetCurVolume (see above). */
static void cvfs_RodataOrderVolumes(const Char8 **out) {
    out[0] = "cvFsSetDefVol #1:illegal device name";
    out[1] = "cvFsSetDefVol #2:illegal volume name";
    out[2] = "cvFsSetDefVol #3:device not found";
    out[3] = "cvFsGetVolumeInfo #1:illegal device name";
    out[4] = "cvFsGetVolumeInfo #2:illegal volume name";
    out[5] = "cvFsGetVolumeInfo #3:device not found";
    out[6] = "cvFsDelVolume #1:illegal device name";
    out[7] = "cvFsDelVolume #2:illegal volume name";
    out[8] = "cvFsDelVolume #3:device not found";
    out[9] = "cvFsAddVolumeEx #1:illegal device name";
    out[10] = "cvFsAddVolumeEx #2:illegal volume name";
    out[11] = "cvFsAddVolumeEx #3:illegal image handle";
    out[12] = "cvFsAddVolumeEx #3:device not found";
    out[13] = "cvFsSetCurVolume #1:illegal device name";
    out[14] = "cvFsSetCurVolume #2:illegal image handle";
    out[15] = "cvFsSetCurVolume #3:device not found";
}

/* Matching workaround: as above, order-only stand-in for the stripped cvFsOptFn2, cvFsOptFn1 and
 * cvFsGetDevName (see above). */
static void cvfs_RodataOrderOptions(const Char8 **out) {
    out[0] = "cvFsOptFn2 #1:handle error";
    out[1] = "cvFsOptFn2 #2:vtbl error";
    out[2] = "cvFsOptFn1 #1:handle error";
    out[3] = "cvFsOptFn1 #2:vtbl error";
    out[4] = "cvFsGetDevName #1:vtbl error";
}

/* Matching workaround: as above, order-only stand-in for the stripped cvFsDeleteFile,
 * cvFsRemoveDir, cvFsMakeDir, cvFsGetMaxByteRate, cvFsIsExistFile and cvFsChangeDir (see above). */
static void cvfs_RodataOrderFiles(const Char8 **out) {
    out[0] = "cvFsDeleteFile #1:illegal file name";
    out[1] = "cvFsDeleteFile #2:illegal device name";
    out[2] = "cvFsDeleteFile #3:device not found";
    out[3] = "cvFsDeleteFile #4:vtbl error";
    out[4] = "cvFsRemoveDir #1:illegal directory name";
    out[5] = "cvFsRemoveDir #2:illegal device name";
    out[6] = "cvFsRemoveDir #3:device not found";
    out[7] = "cvFsRemoveDir #4:vtbl error";
    out[8] = "cvFsMakeDir #1:illegal directory name";
    out[9] = "cvFsMakeDir #2:illegal device name";
    out[10] = "cvFsMakeDir #3:device not found";
    out[11] = "cvFsMakeDir #4:vtbl error";
    out[12] = "cvFsGetMaxByteRate #1:handle error";
    out[13] = "cvFsGetMaxByteRate #2:vtbl error";
    out[14] = "cvFsIsExistFile #2:illegal device name";
    out[15] = "cvFsIsExistFile #1:illegal file name";
    out[16] = "cvFsIsExistFile #3:device not found";
    out[17] = "cvFsIsExistFile #4:vtbl error";
    out[18] = "cvFsChangeDir #1:illegal directory name";
    out[19] = "cvFsChangeDir #2:illegal device name";
    out[20] = "cvFsChangeDir #3:device not found";
    out[21] = "cvFsChangeDir #4:vtbl error";
}

/* Matching workaround: as above, order-only stand-in for the stripped cvFsGetNumTr, cvFsSetSctLen,
 * cvFsGetSctLen, cvFsGetFreeSize, cvFsGetFileSizeByHndl and cvFsGetFileSizeEx (see above). */
static void cvfs_RodataOrderQueries(const Char8 **out) {
    out[0] = "cvFsGetNumTr #1:handle error";
    out[1] = "cvFsGetNumTr #2:vtbl error";
    out[2] = "cvFsSetSctLen #3:handle error";
    out[3] = "cvFsSetSctLen #4:vtbl error";
    out[4] = "cvFsGetSctLen #1:handle error";
    out[5] = "cvFsGetSctLen #2:vtbl error";
    out[6] = "cvFsGetFreeSize #5:device not found";
    out[7] = "cvFsGetFreeSize #6:vtbl error";
    out[8] = "cvFsGetFileSizeByHndl #1:illegal file handle";
    out[9] = "cvFsGetFileSizeEx #1:illegal file name";
    out[10] = "cvFsGetFileSizeEx #2:illegal device name";
    out[11] = "cvFsGetFileSizeEx #3:device not found";
    out[12] = "cvFsGetFileSizeEx #4:vtbl error";
}

/* Installs the user error callback (func(obj, msg, hn)); ADXGC_SetupDvdFs installs
 * adxgc_err_dvd. */
void cvFsEntryErrFunc(CVFS_USRERRFN func, void *obj) {
    if (func == 0) {
        cvfs_errfn = 0;
        cvfs_errobj = 0;
    } else {
        cvfs_errfn = func;
        cvfs_errobj = obj;
    }
}

/* NonMatching: the original has pdev in r28 and the device table cursor in r27; here they swap
 * (14 words, register numbers only). */
Sint32 cvFsGetFileSize(const Char8 *fname) {
    Char8 dev[CVFS_NAME_LEN];
    Char8 path[CVFS_NAME_LEN];
    Char8 *pdev;
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
    vtbl = cvfs_ResolveDev(fname, pdev, path);
    /* Original quirk: dev is the local array, so this test is always false (see the header). */
    if (dev == 0) {
        cvfs_Error("cvFsGetFileSize #2:illegal device name");
    }
    if (vtbl == 0) {
        cvfs_Error("cvFsGetFileSize #3:device not found");
    }
    /* Original quirk: no return after the error above, so a null vtbl is dereferenced here. */
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

/* Matching workaround: as above, order-only stand-in for the stripped cvFsReqWr (see above). */
static void cvfs_RodataOrderReqWr(const Char8 **out) {
    out[0] = "cvFsReqWr #1:handle error";
    out[1] = "cvFsReqWr #2:vtbl error";
}

/* Requests `nsct` sectors into `buf` from the device; returns the sectors accepted (0 =
 * busy/refused). */
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

/* a free handle slot
 * Matching workaround: returns void *. With a CVFS_OBJ * return, cvFsOpen is 408 words instead of
 * the original's 409, 228 of them differing. Revisit when the original allocator's signature is
 * known. */
static void *cvfs_AllocObj(void) {
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

    if (fname == 0) {
        cvfs_Error("cvFsOpen #1:illegal file name");
        return 0;
    }
    cvfs_SplitFname(fname, dev, path);
    if (path[0] == '\0') {
        cvfs_Error("cvFsOpen #1:illegal file name");
        return 0;
    }
    obj = cvfs_AllocObj();
    if (obj == 0) {
        cvfs_Error("cvFsOpen #3:failed handle alloced");
        return 0;
    }
    pdev = dev;
    vtbl = cvfs_ResolveDev(fname, pdev, path);
    obj->vtbl = vtbl;
    /* Original quirk: always false, as in cvFsGetFileSize. */
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
    if (obj->vtbl->Open != 0) {
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

/* Matching workaround: as above, order-only stand-in for the stripped cvFsDelDev (see above). */
static void cvfs_RodataOrderDelDev(const Char8 **out) {
    out[0] = "cvFsDelDev #1:illegal device name";
}

/* Registers the device in the table unless its name is already there; 0 when the table is full. */
static CVFS_IF *cvfs_AddDevTbl(Char8 *devname, CVFS_IF *vtbl) {
    CVFS_DEV *dev;
    Sint32 i;

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
    CVFS_IF *vtbl;

    cvfs_build;
    if (devname == 0) {
        cvfs_Error("cvFsAddDev #1:illegal device name");
        return;
    }
    if (getif == 0) {
        cvfs_Error("cvFsAddDev #2:illegal I/F func name");
        return;
    }
    cvfs_StrUpr(devname);
    vtbl = cvfs_AddDevTbl(devname, getif());
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
