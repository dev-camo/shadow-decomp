/* CRI CVFS GameCube DVD device (gcci.c, GCCI Ver.1.09): the "GCD" device of the CVFS.
 * A handle wraps a DVDFileInfo; reads are asynchronous sector requests polled to completion
 * by gcCiExecServer, one in flight across all handles.
 *
 * Names, types and layout follow the same CRI library version in the RE4 GameCube
 * decompilation (same build string version, error codes, function order and table).
 */
#include <dolphin/dvd.h>
#include <dolphin/os.h>
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/cvfs.h"

#define GCCI_MAX_OBJ 40
#define GCCI_DEF_SCTLEN 0x800
#define GCCI_PATH_LEN 260
#define GCCI_CANCEL_TIMEOUT 2000

/* GCCI_OBJ.stat */
#define GCCI_STAT_STOP 0
#define GCCI_STAT_COMPLETE 1
#define GCCI_STAT_READING 2
#define GCCI_STAT_ERROR 3

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 pad01;
    /* 0x02 */ Sint8 stat;
    /* 0x03 */ Sint8 pad03;
    /* 0x04 */ Sint32 pad04;
    /* 0x08 */ Uint8 *buf;
    /* 0x0C */ Sint32 cbstat; /* last DVDGetCommandBlockStatus */
    /* 0x10 */ Sint32 sctlen;
    /* 0x14 */ Sint32 fsize_byte;
    /* 0x18 */ Sint32 fsize_sct;
    /* 0x1C */ Sint32 pos_sct;
    /* 0x20 */ Sint32 numtr; /* bytes transferred by the last request */
    /* 0x24 */ Sint32 rqsct; /* sectors of the last request */
    /* 0x28 */ DVDFileInfo fi;
} GCCI_OBJ;

typedef GCCI_OBJ *GCCI;

/* Debug information. */
typedef struct {
    /* 0x00 */ Sint32 cbstat; /* last command block status seen */
    /* 0x04 */ Sint8 stat;    /* last handle state set */
    /* 0x05 */ Sint8 pad05[3];
    /* 0x08 */ Sint32 cancel; /* inside DVDCancel */
} GCCI_DEBUG;

/* Defined in gcci_sub.c. */
extern Sint32 gcg_ci_rdmode;
extern Char8 gcg_ci_root_dir[256];

const Char8 gcg_ci_build_str[] = "\nGCCI Ver.1.09 Build:Feb 28 2005 15:48:31\n";

GCCI_DEBUG gcg_ci_debug;
void *gcg_ci_err_obj;
CVFS_USRERRFN gcg_ci_err_func;
GCCI_OBJ gcg_ci_obj[GCCI_MAX_OBJ];
/* volatile: gcCiGetInterface reloads and stores this pointer, which a plain
 * self-assignment would not emit.
 */
const Char8 *volatile gcg_ci_build_ptr;

void gcCiExecServer(void);
void gcCiEntryErrFunc(CVFS_USRERRFN func, void *obj);
Sint32 gcCiGetFileSize(const Char8 *fname);
GCCI gcCiOpen(const Char8 *fname, void *dir, Sint32 rw);
void gcCiClose(void *hn);
Sint32 gcCiSeek(GCCI gcci, Sint32 pos, Sint32 type);
Sint32 gcCiTell(GCCI gcci);
Sint32 gcCiReqRd(void *hn, Sint32 nsct, Uint8 *buf);
void gcCiStopTr(void *hn);
Sint32 gcCiGetStat(GCCI gcci);
Sint32 gcCiGetSctLen(GCCI gcci);
void gcCiSetSctLen(GCCI gcci, Sint32 sctlen);
Sint32 gcCiGetNumTr(GCCI gcci);

CVFS_IF gcg_ci_vtbl = {
    gcCiExecServer,
    gcCiEntryErrFunc,
    gcCiGetFileSize,
    0,
    (void *(*)(const Char8 *, void *, Sint32))gcCiOpen,
    gcCiClose,
    (Sint32 (*)(void *, Sint32, Sint32))gcCiSeek,
    (Sint32 (*)(void *))gcCiTell,
    (Sint32 (*)(void *, Sint32, void *))gcCiReqRd,
    0,
    gcCiStopTr,
    (Sint32 (*)(void *))gcCiGetStat,
    (Sint32 (*)(void *))gcCiGetSctLen,
    (void (*)(void *, Sint32))gcCiSetSctLen,
    (Sint32 (*)(void *))gcCiGetNumTr,
};

/* Unreferenced, so the linker strips it from the executable. Body from RE4's gcci.c (not
 * verified against Shadow). MWCC places .bss objects in order of first reference; this is
 * the first function to touch the debug block, error object and error callback, and the
 * target's .bss order (which the match does verify) follows from it.
 */
void gcCiInit(void) {
    gcg_ci_debug.cbstat = 0;
    gcg_ci_err_obj = 0;
    gcg_ci_err_func = 0;
}

/* Reports an error to the CVFS error callback with the offending handle. */
static inline void gcci_CallErr(const Char8 *msg, void *hn) {
    if (gcg_ci_err_func != 0) {
        gcg_ci_err_func(gcg_ci_err_obj, msg, hn);
    }
}

/* The handle is not reading. */
static inline Sint32 gcci_IsIdle(GCCI gcci) {
    Sint32 idle = 0;

    if (gcci->stat == GCCI_STAT_COMPLETE || gcci->stat == GCCI_STAT_STOP) {
        idle = 1;
    }
    return idle;
}

/* The last DVD command finished (done or cancelled). */
static inline Sint32 gcci_IsCmdDone(GCCI gcci) {
    Sint32 done = 0;

    if (gcci->cbstat == DVD_STATE_END || gcci->cbstat == DVD_STATE_CANCELED) {
        done = 1;
    }
    return done;
}

/* DVDReadAsyncPrio completion callback: the server polls the command block status instead. */
void gcci_rd_cbfn(Sint32 result, DVDFileInfo *fi) {}

/* Bytes transferred by the last read request. */
Sint32 gcCiGetNumTr(GCCI gcci) {
    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    return gcci->numtr;
}

/* Changes the sector length and rescales the file size, position and transfer count. */
void gcCiSetSctLen(GCCI gcci, Sint32 sctlen) {
    Sint32 pos_byte;

    if (gcci == 0) {
        gcci_CallErr("E0040302:handl is null.", 0);
        return;
    }
    if (gcci->sctlen % 32 != 0) {
        gcci_CallErr("E0040303:invalidate size.", 0);
        return;
    }
    pos_byte = gcci->pos_sct * gcci->sctlen;
    gcci->sctlen = sctlen;
    /* Round up to whole sectors; this operand order gives the target's add-then-decrement. */
    gcci->fsize_sct = (gcci->fsize_byte + (gcci->sctlen - 1)) / gcci->sctlen;
    gcci->pos_sct = pos_byte / gcci->sctlen;
    gcci->numtr = gcci->rqsct * sctlen;
}

/* Sector length in bytes (0x800 by default). */
Sint32 gcCiGetSctLen(GCCI gcci) {
    if (gcci == 0) {
        gcci_CallErr("E0040301:handl is null.", 0);
        return 0;
    }
    return gcci->sctlen;
}

/* Handle state: 0 stop, 1 complete, 2 reading, 3 error. */
Sint32 gcCiGetStat(GCCI gcci) {
    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    return gcci->stat;
}

/* Cancels a read in flight and waits (up to 2 s) for the command block to finish. */
void gcCiStopTr(void *hn) {
    GCCI gcci;
    Sint32 ret;
    Uint32 t0;
    Uint32 t;
    Uint32 dt;

    gcci = hn;
    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return;
    }
    if (gcci->stat == GCCI_STAT_COMPLETE || gcci->stat == GCCI_STAT_STOP) {
        return;
    }
    DVDGetCommandBlockStatus(&gcci->fi.cb);
    DVDGetDriveStatus();
    gcg_ci_debug.cancel = 1;
    ret = DVDCancel(&gcci->fi.cb);
    gcg_ci_debug.cancel = 0;
    if (ret < 0) {
        gcci_CallErr("E0092917:DVDCancel failed.", gcci);
        return;
    }
    t0 = OSTicksToMilliseconds(OSGetTick());
    while (!gcci_IsCmdDone(gcci)) {
        gcci->cbstat = DVDGetCommandBlockStatus(&gcci->fi.cb);
        gcg_ci_debug.cbstat = gcci->cbstat;
        t = OSTicksToMilliseconds(OSGetTick());
        /* Elapsed time across a tick counter wrap. */
        dt = (t >= t0) ? (t - t0) : (0xFFFFFFFF - t0 + t);
        if (dt > GCCI_CANCEL_TIMEOUT) {
            gcci_CallErr("E0092918:DVDCancel time out.", gcci);
            break;
        }
    }
    gcci->stat = GCCI_STAT_STOP;
    gcg_ci_debug.stat = GCCI_STAT_STOP;
    DVDGetCommandBlockStatus(&gcci->fi.cb);
    DVDGetDriveStatus();
}

/* First unused slot of the handle table, null when full. */
static inline GCCI gcci_GetFreeHn(void) {
    GCCI gcci = 0;
    Sint32 i;

    for (i = 0; i < GCCI_MAX_OBJ; i++) {
        if (gcg_ci_obj[i].used == 0) {
            gcci = &gcg_ci_obj[i];
            break;
        }
    }
    return gcci;
}

/* A handle of the table has a read in flight. */
static inline Sint32 gcci_IsBusy(GCCI tbl) {
    Sint32 i;

    for (i = 0; i < GCCI_MAX_OBJ; i++) {
        if (tbl[i].used == 1 && tbl[i].stat == GCCI_STAT_READING) {
            return 1;
        }
    }
    return 0;
}

/* Finishes one handle's read when its DVD command completed. */
static inline void gcci_ExecOne(GCCI ci) {
    Sint32 over;
    Sint32 nbyte;
    Uint8 *p;

    if (ci->used == 1 && ci->stat == GCCI_STAT_READING) {
        ci->cbstat = DVDGetCommandBlockStatus(&ci->fi.cb);
        gcg_ci_debug.cbstat = ci->cbstat;
        switch (ci->cbstat) {
        case DVD_STATE_FATAL_ERROR:
            ci->stat = GCCI_STAT_ERROR;
            gcg_ci_debug.stat = GCCI_STAT_ERROR;
            break;
        case DVD_STATE_END:
            nbyte = ci->rqsct * ci->sctlen;
            DCInvalidateRange(ci->buf, nbyte);
            ci->numtr = nbyte;
            ci->pos_sct += ci->rqsct;
            /* Zero the part of the last sector past the end of the file. */
            if (ci->pos_sct * ci->sctlen > ci->fsize_byte) {
                over = ci->pos_sct * ci->sctlen - ci->fsize_byte;
                p = ci->buf + ci->numtr - over;
                memset(p, 0, over);
                DCStoreRange(p, over);
            }
            ci->stat = GCCI_STAT_COMPLETE;
            gcg_ci_debug.stat = GCCI_STAT_COMPLETE;
            break;
        case DVD_STATE_CANCELED:
            over = DVDGetTransferredSize(&ci->fi);
            DCInvalidateRange(ci->buf, over);
            ci->numtr = ci->sctlen * (over / ci->sctlen);
            ci->pos_sct += over / ci->sctlen;
            ci->stat = GCCI_STAT_STOP;
            gcg_ci_debug.stat = GCCI_STAT_STOP;
            break;
        }
    }
}

/* Finishes the reads of the table that completed. */
static inline void gcci_ExecServer(GCCI tbl) {
    Sint32 i;

    for (i = 0; i < GCCI_MAX_OBJ; i++) {
        gcci_ExecOne(&tbl[i]);
    }
}

/* Starts a read of nsct sectors at the current position into buf; refused (0) while any handle
 * is reading. Returns the sectors requested.
 */
Sint32 gcCiReqRd(void *hn, Sint32 nsct, Uint8 *buf) {
    GCCI gcci;
    Sint32 ofst;
    Sint32 nbyte;
    Sint32 ret;

    gcci = hn;
    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    if (nsct < 0) {
        gcci_CallErr("E0092913:nsct < 0.(gcCiReqRd)", gcci);
        return 0;
    }
    if (buf == 0) {
        gcci_CallErr("E0092914:buf is null.(gcCiReqRd)", gcci);
        return 0;
    }
    if (!gcci_IsIdle(gcci)) {
        return 0;
    }
    if (!gcci_IsCmdDone(gcci)) {
        return 0;
    }
    if (gcci_IsBusy(gcg_ci_obj)) {
        return 0;
    }
    if (nsct == 0) {
        gcci->stat = GCCI_STAT_COMPLETE;
        gcg_ci_debug.stat = GCCI_STAT_COMPLETE;
        return 0;
    }
    gcci->numtr = 0;
    gcci->buf = buf;
    gcci->rqsct = nsct;
    gcci_ExecServer(gcg_ci_obj);
    ofst = gcci->pos_sct * gcci->sctlen;
    nbyte = gcci->rqsct * gcci->sctlen;
    if (ofst + nbyte > gcci->fsize_byte) {
        nbyte = gcci->fsize_byte - ofst;
        if (nbyte < 0) {
            gcci->stat = GCCI_STAT_COMPLETE;
            gcg_ci_debug.stat = GCCI_STAT_COMPLETE;
            return nsct;
        }
    }
    /* DVD transfers are whole 32-byte units. */
    nbyte = (nbyte + 31) & ~31;
    DCInvalidateRange(buf, nbyte);
    if (gcg_ci_rdmode == 0) {
        ret = DVDReadAsyncPrio(&gcci->fi, buf, nbyte, ofst, gcci_rd_cbfn, 2);
    } else {
        ret = DVDReadPrio(&gcci->fi, buf, nbyte, ofst, 2);
    }
    if (ret == 0) {
        return 0;
    }
    gcci->stat = GCCI_STAT_READING;
    gcg_ci_debug.stat = GCCI_STAT_READING;
    return gcci->rqsct;
}

/* Current position in sectors. */
Sint32 gcCiTell(GCCI gcci) {
    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    return gcci->pos_sct;
}

/* Seeks in sectors, clamped to 0..file size. */
Sint32 gcCiSeek(GCCI gcci, Sint32 pos, Sint32 type) {
    Sint32 max;

    if (gcci == 0) {
        gcci_CallErr("E0092912:handl is null.", 0);
        return 0;
    }
    if (type == CVFS_SEEK_SET) {
        gcci->pos_sct = pos;
    } else if (type == CVFS_SEEK_END) {
        gcci->pos_sct = gcci->fsize_sct + pos;
    } else if (type == CVFS_SEEK_CUR) {
        gcci->pos_sct += pos;
    }
    max = gcci->fsize_sct;
    if (gcci->pos_sct < max) {
        max = gcci->pos_sct;
    }
    gcci->pos_sct = max;
    gcci->pos_sct = (gcci->pos_sct > 0) ? gcci->pos_sct : 0;
    return gcci->pos_sct;
}

/* The handle arrives as void *; passing hn (not gcci) to the inlined StopTr keeps the
 * target's second copy of r3.
 */
void gcCiClose(void *hn) {
    GCCI gcci;

    gcci = hn;
    if (gcci == 0) {
        return;
    }
    gcCiStopTr(hn);
    DVDClose(&gcci->fi);
    gcci->used = 0;
    memset(gcci, 0, sizeof(GCCI_OBJ));
}

/* Root directory + file name, with '\' converted to '/'. */
static inline void gcci_MakePath(Char8 *path, const Char8 *fname) {
    Uint32 len;
    Uint32 i;

    strcpy(path, gcg_ci_root_dir);
    strcat(path, fname);
    len = strlen(path);
    for (i = 0; i < len; i++) {
        if (path[i] == '\\') {
            path[i] = '/';
        }
    }
}

/* Opens root directory + fname into a free handle; read-only (rw must be 0). */
GCCI gcCiOpen(const Char8 *fname, void *dir, Sint32 rw) {
    Char8 path[GCCI_PATH_LEN];
    GCCI gcci;
    Uint32 len;
    Sint32 n;

    if (fname == 0) {
        gcci_CallErr("E0092908:fname is null.(gcCiOpen)", 0);
        return 0;
    }
    if (rw != 0) {
        gcci_CallErr("E0092909:rw is illigal.(gcCiOpen)", 0);
        return 0;
    }
    gcci = gcci_GetFreeHn();
    if (gcci == 0) {
        gcci_CallErr("E0092910:not enough handle resource.(gcCiOpen)", 0);
        return 0;
    }
    gcci_MakePath(path, fname);
    if (DVDOpen(path, &gcci->fi) == 0) {
        gcci_CallErr("E0092911:DVDOpen fail.(gcCiOpen)", 0);
        memset(gcci, 0, sizeof(GCCI_OBJ));
        return 0;
    }
    gcci->sctlen = GCCI_DEF_SCTLEN;
    /* Sizes are signed; cap the byte length at 0x7FFFFFFF. */
    len = gcci->fi.length;
    if (len & 0x80000000) {
        len = 0x7FFFFFFF;
    }
    gcci->fsize_byte = len;
    n = gcci->sctlen;
    n += gcci->fsize_byte;
    gcci->fsize_sct = (n - 1) / gcci->sctlen;
    gcci->pos_sct = 0;
    gcci->buf = 0;
    gcci->rqsct = 0;
    gcci->numtr = 0;
    gcci->stat = GCCI_STAT_STOP;
    gcci->used = 1;
    return gcci;
}

/* File size in bytes via a temporary open/close (capped at 0x7FFFFFFF). */
Sint32 gcCiGetFileSize(const Char8 *fname) {
    Char8 path[GCCI_PATH_LEN];
    DVDFileInfo fi;
    Uint32 len;

    if (fname == 0) {
        gcci_CallErr("E0092901:fname is null.(gcCiGetFileSize)", 0);
        return 0;
    }
    gcci_MakePath(path, fname);
    if (DVDOpen(path, &fi) == 0) {
        gcci_CallErr("E0040201:can't open a file.(gcCiGetFileSize)", 0);
        return 0;
    }
    len = fi.length;
    if (len & 0x80000000) {
        len = 0x7FFFFFFF;
    }
    if (DVDClose(&fi) == 0) {
        gcci_CallErr("E0040202:can't close a file.(gcCiGetFileSize)", 0);
        return 0;
    }
    return len;
}

/* Installs the device error callback. */
void gcCiEntryErrFunc(CVFS_USRERRFN func, void *obj) {
    gcg_ci_err_func = func;
    gcg_ci_err_obj = obj;
}

/* Device server: completes, cancels or fails every reading handle by its command status. */
void gcCiExecServer(void) {
    Sint32 i;

    for (i = 0; i < GCCI_MAX_OBJ; i++) {
        gcci_ExecOne(&gcg_ci_obj[i]);
    }
}

/* Returns the GCD device's CVFS_IF table and resets the device state. */
CVFS_IF *gcCiGetInterface(void) {
    /* Keeps the build string pointer referenced (see gcg_ci_build_ptr). */
    gcg_ci_build_ptr = gcg_ci_build_ptr;
    memset(gcg_ci_root_dir, 0, sizeof(gcg_ci_root_dir));
    gcg_ci_err_func = 0;
    gcg_ci_err_obj = 0;
    memset(&gcg_ci_debug, 0, sizeof(gcg_ci_debug));
    return &gcg_ci_vtbl;
}
