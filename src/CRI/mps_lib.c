/* CRI Sofdec MPEG system stream demultiplexer: handle management (mps_lib.c).
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mps.h"
#include "CRI/uty.h"

const Char8 MPSLIB_version_str[] =
    "\nCRI MPS/GC Ver.1.924 Build:Feb 28 2005 19:36:08\n\0Append: MW2407 GC20Apr2004Patch2\n";

static MPS mpslib_hn_last;
static const Char8 *cri_verstr_ptr;
MPSLIB_WORK *MPSLIB_libwork;
/* unreferenced: MWCC still emits it, after the referenced statics (.bss 0x10, the target's fourth word) */
static Sint32 mpslib_init_cnt;

/* Static handle check used inside this file. The public MPSLIB_CheckHn repeats the check instead of
 * calling this copy; MPSLIB_SetErr wraps mpslib_SetErr. */
static Sint32 mpslib_CheckHn(MPS mps) {
    mpslib_hn_last = mps;
    if (mps == 0) {
        return -1;
    }
    if (mps->used == MPS_HN_FREE) {
        return -1;
    }
    return 0;
}

/* Matching workaround: stand-in for a function the linker stripped from the game. The body is not
 * the original's. It refers to cri_verstr_ptr before the error functions below refer to
 * MPSLIB_libwork; storing the address keeps the reference. Without it the .bss objects land in the
 * first-reference order of the surviving functions and the code that pools their addresses changes
 * (measured by compiling without it). Revisit when the stripped code is known. */
static void mpslib_BssOrder(void **out) {
    out[0] = &cri_verstr_ptr;
}

/* Library-level error: records the code and calls the library error callback with it. */
static Sint32 mpslib_SetLibErr(Sint32 code) {
    MPSLIB_WORK *lw;

    lw = MPSLIB_libwork;
    lw->errcode = code;
    if (code != 0 && lw->errfn != 0) {
        lw->errfn(lw->errobj, code);
    }
    return code;
}

/* Records an error code on the handle (or the library) and calls the matching error callback; the
 * SFD demux driver's callback turns it into an SFD error. */
static Sint32 mpslib_SetErr(MPS mps, Sint32 code) {
    MPSLIB_WORK *lw;

    if (mps == 0) {
        lw = MPSLIB_libwork;
        lw->errcode = code;
        if (code != 0 && lw->errfn != 0) {
            lw->errfn(lw->errobj, code);
        }
    } else {
        mps->errcode = code;
        if (code != 0 && mps->errfn != 0) {
            mps->errfn(mps->errobj, code);
        }
    }
    return code;
}

/* Frees the parser handle. */
Sint32 MPS_Destroy(MPS mps) {
    if (mpslib_CheckHn(mps) != 0) {
        return mpslib_SetLibErr(0xFF020103);
    }
    mps->used = MPS_HN_FREE;
    return 0;
}

/* First free handle of the library work, 0 when all are used. */
static MPS mpslib_GetFreeHn(void) {
    MPSLIB_WORK *lw;
    MPS mps;
    Sint32 i;

    lw = MPSLIB_libwork;
    mps = lw->hn;
    for (i = 0; i < lw->num_hn; i++) {
        if (mps->used == MPS_HN_FREE) {
            return mps;
        }
        mps++;
    }
    return 0;
}

/* Takes a free handle: cleared, headers marked "not seen" (-1), MPEG-1 header decoder installed. */
MPS MPS_Create(void) {
    MPS mps;
    Sint32 i;

    mps = mpslib_GetFreeHn();
    if (mps == 0) {
        return 0;
    }
    UTY_MemsetDword(mps, 0, sizeof(MPS_OBJ) / 4);
    mps->used = MPS_HN_USED;
    mps->errfn = 0;
    mps->errobj = 0;
    mps->errcode = 0;
    mps->x10 = 2;
    mps->packhd.scr = -1;
    mps->packhd.rsv = -1;
    mps->packhd.mux_rate = -1;
    for (i = 0; i < 8; i++) {
        mps->last_syshd.raw[i] = -1;
    }
    for (i = 0; i < 8; i++) {
        mps->syshd[0].raw[i] = -1;
    }
    for (i = 0; i < 8; i++) {
        mps->syshd[1].raw[i] = -1;
    }
    for (i = 0; i < 8; i++) {
        mps->syshd[2].raw[i] = -1;
    }
    mps->pkethd.pts = -1;
    mps->pkethd.dts = -1;
    for (i = 0; i < 6; i++) {
        mps->pkethd.raw[i] = -1;
    }
    mps->xd0 = 0;
    mps->dechd_func = MPSDEC_DecHdMpeg1;
    mps->xd8 = 0;
    mps->xdc = 0;
    mps->xe0 = 0;
    mps->sysfn = 0;
    mps->sysobj = 0;
    return mps;
}

/* -1 for a 0 or free handle; records it in mpslib_hn_last. */
Sint32 MPSLIB_CheckHn(MPS mps) {
    mpslib_hn_last = mps;
    if (mps == 0) {
        return -1;
    }
    if (mps->used == MPS_HN_FREE) {
        return -1;
    }
    return 0;
}

/* Installs the error callback library-wide (mps 0) or per handle. */
Sint32 MPS_SetErrFn(MPS mps, void (*fn)(void *obj, Sint32 code), void *obj) {
    MPSLIB_WORK *lw;

    if (mps == 0) {
        lw = MPSLIB_libwork;
        lw->errfn = fn;
        lw->errobj = obj;
    } else {
        if (mpslib_CheckHn(mps) != 0) {
            return mpslib_SetLibErr(0xFF020101);
        }
        mps->errfn = fn;
        mps->errobj = obj;
    }
    return 0;
}

/* Public error report (mpslib_SetErr). */
Sint32 MPSLIB_SetErr(MPS mps, Sint32 code) {
    return mpslib_SetErr(mps, code);
}

/* Destroys every used handle and finishes the sub-modules. */
void MPS_Finish(void) {
    MPSLIB_WORK *lw;
    MPS mps;
    Sint32 i;
    Sint32 num;

    lw = MPSLIB_libwork;
    num = lw->num_hn;
    mps = lw->hn;
    for (i = 0; i < num; i++) {
        if (mps->used != MPS_HN_FREE) {
            MPS_Destroy(mps);
        }
        mps++;
    }
    MPSDEC_Finish();
    MPSGET_Finish();
}

/* Clears the handle table. Inlined into MPS_Init; its always-zero status and the caller's check
 * account for the dead branch pair the original keeps after the loop. */
static Sint32 mpslib_ClrHn(MPS hn, Sint32 num) {
    Sint32 i;

    for (i = 0; i < num; i++) {
        hn[i].used = MPS_HN_FREE;
    }
    return 0;
}

/* Library init in the caller's work: endianness self-check (spins on little-endian), header +
 * `num_hn` handles all free. */
Sint32 MPS_Init(Sint32 num_hn, void *work) {
    static const Uint32 test_wrok = 0x01020304;
    MPSLIB_WORK *lw;
    Sint32 ret;

    cri_verstr_ptr = MPSLIB_version_str;
    if (*(const Uint8 *)&test_wrok != 1) {
        for (;;) {
            ((void (*)(void))-1)();
        }
    }
    MPSLIB_libwork = (MPSLIB_WORK *)work;
    UTY_MemsetDword(work, 0, (sizeof(MPSLIB_WORK) + (num_hn - 1) * sizeof(MPS_OBJ)) / 4);
    lw = MPSLIB_libwork;
    lw->errfn = 0;
    lw->errobj = 0;
    lw->errcode = 0;
    MPSLIB_libwork->num_hn = num_hn;
    ret = mpslib_ClrHn(MPSLIB_libwork->hn, num_hn);
    if (ret != 0) {
        return ret;
    }
    MPSDEC_Init();
    MPSGET_Init();
    return 0;
}
