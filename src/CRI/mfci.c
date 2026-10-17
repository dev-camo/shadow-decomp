/* CRI CVFS memory-file device (mfci.c, MFCI/GC Ver.1.10): the "MFS" device of the CVFS.
 * A "file" is a memory range named "%08x.%08x" (address.size); reads are memcpy and
 * complete at once.
 *
 * Names, types and layout follow MFCI Ver.1.09 in the RE4 GameCube decompilation (same
 * error codes and function order); Ver.1.10 adds mfCiOptFn for the CVFS OptFn slot.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/cvfs.h"
#include "CRI/svm.h"
#include "layout_check.h"

#define MFCI_MAX_OBJ 40
#define MFCI_DEF_SCTLEN 0x800
#define MFCI_FNAME_LEN 17

/* MFCI_OBJ.stat */
#define MFCI_STAT_STOP 0
#define MFCI_STAT_COMPLETE 1
#define MFCI_STAT_READING 2

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 stat;
    /* 0x02 */ Uint8 pad02[2];
    /* 0x04 */ Sint32 sctlen;
    /* 0x08 */ Sint32 fsize_byte;
    /* 0x0C */ Sint32 fsize_sct;
    /* 0x10 */ Sint32 pos_sct;
    /* 0x14 */ Sint32 numtr; /* bytes transferred by the last request */
    /* 0x18 */ Sint32 rqsct; /* sectors of the last request */
    /* 0x1C */ Char8 fname[20];
    /* 0x30 */ Sint32 rd_ofst;
    /* 0x34 */ Sint32 rd_nbyte;
} MFCI_OBJ;
CHECK_SIZEOF(MFCI_OBJ, 0x38);

typedef MFCI_OBJ *MFCI;

/* volatile: mfCiGetInterface reads the build string pointer without using it. */
const Char8 *const volatile mfci_build = "\nMFCI/GC Ver.1.10 Build:Feb 28 2005 15:49:14\n";

CVFS_USRERRFN mfci_err_func;
void *mfci_err_obj;
Char8 mfci_err_str[300];
MFCI_OBJ mfci_obj[MFCI_MAX_OBJ];
Sint32 mfci_init_flag;

void mfCiExecServer(void);
void mfCiEntryErrFunc(CVFS_USRERRFN func, void *obj);
Sint32 mfCiGetFileSize(const Char8 *fname);
MFCI mfCiOpen(const Char8 *fname, void *dir, Sint32 rw);
void mfCiClose(MFCI mfci);
Sint32 mfCiSeek(MFCI mfci, Sint32 pos, Sint32 type);
Sint32 mfCiTell(MFCI mfci);
Sint32 mfCiReqRd(void *hn, Sint32 nsct, Uint8 *buf);
void mfCiStopTr(MFCI mfci);
Sint32 mfCiGetStat(MFCI mfci);
Sint32 mfCiGetSctLen(MFCI mfci);
void mfCiSetSctLen(MFCI mfci, Sint32 sctlen);
Sint32 mfCiGetNumTr(MFCI mfci);
Sint32 mfCiOptFn(void *hn, Sint32 fnid, Sint32 a, Sint32 b);

CVFS_IF mfci_vtbl = {
    mfCiExecServer,
    mfCiEntryErrFunc,
    mfCiGetFileSize,
    0,
    (void *(*)(const Char8 *, void *, Sint32))mfCiOpen,
    (void (*)(void *))mfCiClose,
    (Sint32 (*)(void *, Sint32, Sint32))mfCiSeek,
    (Sint32 (*)(void *))mfCiTell,
    (Sint32 (*)(void *, Sint32, void *))mfCiReqRd,
    0,
    (void (*)(void *))mfCiStopTr,
    (Sint32 (*)(void *))mfCiGetStat,
    (Sint32 (*)(void *))mfCiGetSctLen,
    (void (*)(void *, Sint32))mfCiSetSctLen,
    (Sint32 (*)(void *))mfCiGetNumTr,
    {0},
    mfCiOptFn,
};

/* Reports an error to the CVFS error callback with the offending handle. */
static inline void mfci_CallErr(const Char8 *msg, void *hn) {
    if (mfci_err_func != 0) {
        mfci_err_func(mfci_err_obj, msg, hn);
    }
}

/* Bytes -> sectors, rounded up. */
static inline Sint32 mfci_ByteToSct(Sint32 sctlen, Sint32 nbyte) {
    Sint32 n;

    n = sctlen;
    n += nbyte;
    return (n - 1) / sctlen;
}

/* First unused slot of the handle table, null when full. */
static inline MFCI mfci_GetFreeHn(void) {
    MFCI mfci = 0;
    Sint32 i;

    for (i = 0; i < MFCI_MAX_OBJ; i++) {
        if (mfci_obj[i].used == 0) {
            mfci = &mfci_obj[i];
            break;
        }
    }
    return mfci;
}

/* "%08x.%08x": memory address and size of the file. */
static inline Uint32 mfci_get_adr_size(const Char8 *fname, Uint32 *size) {
    Char8 *p;
    Uint32 adr;

    if (strlen(fname) != MFCI_FNAME_LEN) {
        sprintf(mfci_err_str, "E01100308:length of '%s' is not 17 bytes.(mfci_get_adr_size)",
                fname);
        mfci_CallErr(mfci_err_str, 0);
    }
    if (fname[8] != '.') {
        sprintf(mfci_err_str, "E01100309:illegal file name format '%s'(mfci_get_adr_size)", fname);
        mfci_CallErr(mfci_err_str, 0);
    }
    p = (Char8 *)fname;
    adr = strtoul(p, &p, 16);
    if (*p != '\0') {
        p++;
    }
    if (size != 0) {
        *size = strtoul(p, &p, 16);
    }
    return adr;
}

/* CVFS OptFn slot (name inferred from the slot and the device naming pattern): 201 returns
 * the handle's transfer count, 203 and 205 the size of a named memory file, other function
 * ids 0. hn is a handle or a file name depending on fnid; a and b are unused.
 */
Sint32 mfCiOptFn(void *hn, Sint32 fnid, Sint32 a, Sint32 b) {
    Sint32 ret = 0;
    const Char8 *fname = hn;
    Uint32 size;
    Uint32 size2;

    if (hn == 0) {
        return 0;
    }
    switch (fnid) {
    case 200:
        ret = 0;
        break;
    case 201:
        /* Same checks as mfCiGetNumTr. */
        if (hn == 0) {
            mfci_CallErr("E0092912:handl is null.", 0);
            ret = 0;
        } else {
            ret = ((MFCI)hn)->numtr;
        }
        break;
    case 202:
        ret = 0;
        break;
    case 203:
        mfci_get_adr_size(fname, &size);
        ret = size;
        break;
    case 204:
        ret = 0;
        break;
    case 205:
        mfci_get_adr_size(fname, &size2);
        ret = size2;
        break;
    case 299:
        ret = 0;
        break;
    }
    return ret;
}

/* Unreferenced, so the linker strips it. Body from the RE4 decompilation (not verified
 * against Shadow); its literals and the mfci_init_flag storage are verified in the object.
 */
MFCI mfCiOpenEntry(Sint32 entry, Sint32 rw) {
    MFCI mfci;

    if (entry < 0 || entry >= MFCI_MAX_OBJ) {
        sprintf(mfci_err_str, "E1041001:invalid entry number.(mfCiOpenEntry)");
        mfci_CallErr(mfci_err_str, 0);
        return 0;
    }
    if (rw != 0) {
        sprintf(mfci_err_str, "E1041002:rw is illigal.(mfCiOpenEntry)");
        mfci_CallErr(mfci_err_str, 0);
        return 0;
    }
    mfci = &mfci_obj[entry];
    if (mfci->used != 0) {
        sprintf(mfci_err_str, "E1041002:not enough handle resource.(mfCiOpenEntry)");
        mfci_CallErr(mfci_err_str, 0);
        return 0;
    }
    mfci_init_flag = 1;
    mfci->used = 1;
    return mfci;
}

/* Bytes transferred by the last read request. */
Sint32 mfCiGetNumTr(MFCI mfci) {
    if (mfci == 0) {
        mfci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    return mfci->numtr;
}

/* Changes the sector length and rescales the file size, position and transfer count. */
void mfCiSetSctLen(MFCI mfci, Sint32 sctlen) {
    Sint32 posbyte;

    if (mfci == 0) {
        mfci_CallErr("E0040302:handl is null.", 0);
        return;
    }
    posbyte = mfci->pos_sct * mfci->sctlen;
    mfci->sctlen = sctlen;
    mfci->fsize_sct = mfci_ByteToSct(mfci->sctlen, mfci->fsize_byte);
    mfci->pos_sct = posbyte / mfci->sctlen;
    mfci->numtr = mfci->rqsct * sctlen;
}

/* Sector length in bytes (0x800 by default). */
Sint32 mfCiGetSctLen(MFCI mfci) {
    if (mfci == 0) {
        mfci_CallErr("E0040301:handl is null.", 0);
        return 0;
    }
    return mfci->sctlen;
}

/* Handle state: 0 stop, 1 complete, 2 reading. */
Sint32 mfCiGetStat(MFCI mfci) {
    if (mfci == 0) {
        mfci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    return mfci->stat;
}

/* Marks the handle stopped (memory reads complete synchronously). */
void mfCiStopTr(MFCI mfci) {
    if (mfci == 0) {
        mfci_CallErr("E0092912:handl is null.", 0);
        return;
    }
    SVM_Lock();
    mfci->stat = MFCI_STAT_STOP;
    SVM_Unlock();
}

/* Copies nsct sectors from the memory range into buf, zero-filling past its end, and
 * completes at once. Returns the sectors copied.
 */
Sint32 mfCiReqRd(void *hn, Sint32 nsct, Uint8 *buf) {
    Uint32 adr;
    Uint32 size;
    Sint32 cpy;
    Sint32 rem;
    Sint32 rd_ofst;
    Sint32 rd_nbyte;
    MFCI p = hn;

    if (p == 0) {
        mfci_CallErr("E01100307:handl is null.", 0);
        return 0;
    }
    if (nsct < 0) {
        mfci_CallErr("E01100308:nsct < 0.(mfCiReqRd)", p);
        return 0;
    }
    if (buf == 0) {
        mfci_CallErr("E01100309:buf is null.(mfCiReqRd)", p);
        return 0;
    }
    if (nsct == 0) {
        p->stat = MFCI_STAT_COMPLETE;
        return 0;
    }
    if (p->stat == MFCI_STAT_READING) {
        return 0;
    }
    SVM_Lock();
    p->numtr = 0;
    rem = p->fsize_sct - p->pos_sct;
    if (nsct < rem) {
        rem = nsct;
    }
    p->rqsct = rem;
    rd_ofst = p->pos_sct * p->sctlen;
    rd_nbyte = p->rqsct * p->sctlen;
    if (rd_nbyte == 0) {
        p->stat = MFCI_STAT_COMPLETE;
        SVM_Unlock();
        return 0;
    }
    p->rd_ofst = rd_ofst;
    p->rd_nbyte = rd_nbyte;
    p->stat = MFCI_STAT_READING;
    adr = mfci_get_adr_size(p->fname, &size);
    cpy = p->rd_nbyte;
    if (cpy > (Sint32)(size - p->rd_ofst)) {
        cpy = size - p->rd_ofst;
    }
    SVM_Unlock();
    memcpy(buf, (void *)(adr + p->rd_ofst), p->rd_nbyte);
    memset(buf + cpy, 0, p->rd_nbyte - cpy);
    SVM_Lock();
    p->numtr = p->rqsct * p->sctlen;
    p->pos_sct += p->rqsct;
    p->stat = MFCI_STAT_COMPLETE;
    SVM_Unlock();
    return p->rqsct;
}

/* Current position in sectors. */
Sint32 mfCiTell(MFCI mfci) {
    if (mfci == 0) {
        mfci_CallErr("E01100306:handl is null.", 0);
        return 0;
    }
    return mfci->pos_sct;
}

/* Seeks in sectors, clamped to 0..size. */
Sint32 mfCiSeek(MFCI mfci, Sint32 pos, Sint32 type) {
    if (mfci == 0) {
        mfci_CallErr("E01100305:handl is null.", 0);
        return 0;
    }
    SVM_Lock();
    if (type == CVFS_SEEK_SET) {
        mfci->pos_sct = pos;
    } else if (type == CVFS_SEEK_END) {
        mfci->pos_sct = mfci->fsize_sct + pos;
    } else if (type == CVFS_SEEK_CUR) {
        mfci->pos_sct += pos;
    }
    mfci->pos_sct = (mfci->pos_sct < mfci->fsize_sct) ? mfci->pos_sct : mfci->fsize_sct;
    mfci->pos_sct = (mfci->pos_sct > 0) ? mfci->pos_sct : 0;
    SVM_Unlock();
    return mfci->pos_sct;
}

/* Stops and clears the handle. */
void mfCiClose(MFCI mfci) {
    if (mfci == 0) {
        return;
    }
    mfCiStopTr(mfci);
    if (mfci->used == 1) {
        mfci->used = 0;
        memset(mfci, 0, sizeof(MFCI_OBJ));
    }
}

/* Opens a memory file: parses address.size from the name; sector length 0x800. */
MFCI mfCiOpen(const Char8 *fname, void *dir, Sint32 rw) {
    MFCI mfci;
    Uint32 size;

    if (fname == 0) {
        mfci_CallErr("E01100301:fname is null.(mfCiOpen)", 0);
        return 0;
    }
    if (rw != 0) {
        mfci_CallErr("E01100302:rw is illigal.(mfCiOpen)", 0);
        return 0;
    }
    mfci = mfci_GetFreeHn();
    if (mfci == 0) {
        mfci_CallErr("E01100303:not enough handle resource.(mfCiOpen)", 0);
        return 0;
    }
    strcpy(mfci->fname, fname);
    mfci->sctlen = MFCI_DEF_SCTLEN;
    mfci_get_adr_size(mfci->fname, &size);
    mfci->fsize_byte = size;
    mfci->fsize_sct = mfci_ByteToSct(mfci->sctlen, mfci->fsize_byte);
    mfci->pos_sct = 0;
    mfci->rqsct = 0;
    mfci->numtr = 0;
    mfci->stat = MFCI_STAT_STOP;
    mfci->used = 1;
    return mfci;
}

/* The size part of the address.size name. */
Sint32 mfCiGetFileSize(const Char8 *fname) {
    Uint32 size;

    mfci_get_adr_size(fname, &size);
    return size;
}

/* Installs the device error callback. */
void mfCiEntryErrFunc(CVFS_USRERRFN func, void *obj) {
    mfci_err_func = func;
    mfci_err_obj = obj;
}

/* Nothing to do per pass (reads complete in mfCiReqRd). */
void mfCiExecServer(void) {
    Sint32 i;

    for (i = 0; i < MFCI_MAX_OBJ; i++) {
    }
}

/* Returns the MFS device's CVFS_IF table. */
CVFS_IF *mfCiGetInterface(void) {
    mfci_build;
    return &mfci_vtbl;
}
