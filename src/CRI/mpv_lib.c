/* CRI Sofdec MPV library and handle management (mpv_lib.c): library init in the caller's work
 * area (the handles followed by the shared table area), handle creation and destruction, and the
 * decoder conditions. With the locked cache enabled in HID2 the handle and table areas are
 * invalidated (and zeroed in the locked cache) a cache line at a time.
 * Bodies from the RE4 GameCube decompilation of the same file. This version (1.958) has larger
 * handles, installs the IDCT functions in each handle and no longer moves the work area into the
 * locked cache.
 */
#include <stddef.h>
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/dct.h"
#include "CRI/mpv.h"
#include "CRI/uty.h"
#include "layout_check.h"

#define MPVLIB_COND_NUM 16
#define MPVLIB_COND_GUARD 0x5A5A5A5A /* last entry of the default condition table */

/* handle states */
#define MPVLIB_STAT_STOP 1
#define MPVLIB_STAT_READY 2

/* HID2 locked cache enable */
#define MPVLIB_HID2_LCE 0x10000000

/* Handles are laid out a whole number of 32-byte cache lines apart. */
#define MPVLIB_HN_SIZE ((sizeof(MPV_OBJ) + 0x1F) & ~0x1F)
#define MPVLIB_NEXT_HN(hn) ((MPV)((Uint8 *)(hn) + MPVLIB_HN_SIZE))

/* space between the last handle and the shared table area */
#define MPVLIB_IXA_GAP 0x3A0

#define MPVLIB_IXA_SIZE sizeof(MPV_IXA)

/* library work */
typedef struct {
    Sint32 cond[MPVLIB_COND_NUM + 1]; /* 0x00 default conditions (copy of mpvlib_cond_dfl) */
    Sint32 use_lc_work;               /* 0x44 (not read by this version) */
    Uint32 hid2;                      /* 0x48 HID2 at init (MPVLIB_HID2_LCE cleared if unused) */
    void *hn_end;                     /* 0x4C */
    MPV_IXA *ixa;                     /* 0x50 */
    Sint32 nhn;                       /* 0x54 */
    MPV_OBJ *hn;                      /* 0x58 */
} MPVLIB_WORK;
CHECK_SIZEOF(MPVLIB_WORK, 0x5C);

const Char8 MPVLIB_version_str[] =
    "\nCRI MPV/GC Ver.1.958 Build:Feb 28 2005 19:36:10\n\0Append: MW2407 GC20Apr2004Patch2\n";

const Sint32 mpvlib_cond_dfl[MPVLIB_COND_NUM + 1] = {
    0, 1, 1, 0, 0, 0, 3, 0x7FFFFFFF, (Sint32)MPV_MbCbFn, 0, 0, 0, 0, 0, 0, 0, MPVLIB_COND_GUARD,
};
const Sint32 mpvlib_siz_mpvwork = sizeof(MPVLIB_WORK);
const Sint32 mpvlib_siz_mpvobj = sizeof(MPV_OBJ);
const Sint32 mpvlib_siz_mpvixa = MPVLIB_IXA_SIZE;

static Sint32 mpvlib_use_lc;
Uint8 mpv_clip_0_255_tbl[0x400];
static Uint8 *mpv_clip_0_255_base;
static MPV mpvlib_mpvobj;
static Sint32 mpvlib_oix;
Sint32 mpvlib_iix;
static MPVLIB_WORK mpvlib_libwork;
static const Char8 *cri_verstr_ptr;

/* Stand-ins for functions the linker stripped from the game; their bodies are unknown. MWCC lays
 * out .bss by first reference, so these never-called functions refer to the variables whose first
 * use in the remaining functions comes too late for their storage order. Each refers to one
 * variable: one function for the first three makes MWCC pool the .bss addresses earlier, which
 * changes MPV_Init's register assignment (18 words); the last two follow the same form.
 */
static void *mpvlib_BssOrder1(void) {
    return &mpvlib_use_lc;
}

static void *mpvlib_BssOrder2(void) {
    return &mpv_clip_0_255_tbl;
}

static void *mpvlib_BssOrder3(void) {
    return &mpv_clip_0_255_base;
}

/* -1 for a NULL or not ready handle; the handle is recorded in mpvlib_mpvobj. */
Sint32 MPVLIB_CheckHn(MPV mpv) {
    mpvlib_mpvobj = mpv;
    if (mpv == NULL) {
        return -1;
    }
    if (mpv->stat != MPVLIB_STAT_READY) {
        return -1;
    }
    return 0;
}

/* stand-ins as above, for the two words after mpvlib_mpvobj (no surviving function uses them) */
static void *mpvlib_BssOrder4(void) {
    return &mpvlib_oix;
}

static void *mpvlib_BssOrder5(void) {
    return &mpvlib_iix;
}

/* Decoder condition `id` of the handle, or the library default when mpv is NULL. */
Sint32 MPV_GetCond(MPV mpv, Sint32 id, Sint32 *val) {
    Sint32 *cond;

    if (mpv == NULL) {
        cond = mpvlib_libwork.cond;
    } else {
        if (MPVLIB_CheckHn(mpv) != 0) {
            return MPVERR_SetCode(NULL, 0xFF030210);
        }
        cond = mpv->cond;
    }
    *val = cond[id];
    return 0;
}

/* Sets decoder condition `id` (8 with 0 installs the empty macroblock callback); with mpv NULL the
 * library default and the condition of every ready handle.
 */
Sint32 MPV_SetCond(MPV mpv, Sint32 id, Sint32 val) {
    Sint32 *cond;
    Sint32 i;
    Sint32 nhn;
    MPV hn;

    if (id == 8 && val == 0) {
        val = (Sint32)MPV_MbCbFn;
    }
    if (mpv == NULL) {
        nhn = mpvlib_libwork.nhn;
        hn = mpvlib_libwork.hn;
        for (i = 0; i < nhn; i++) {
            if (hn->stat == MPVLIB_STAT_READY) {
                hn->cond[id] = val;
            }
            hn = MPVLIB_NEXT_HN(hn);
        }
        cond = mpvlib_libwork.cond;
    } else {
        if (MPVLIB_CheckHn(mpv) != 0) {
            return MPVERR_SetCode(NULL, 0xFF030202);
        }
        cond = mpv->cond;
    }
    cond[id] = val;
    MPVM2V_SetCond(mpv, id, val);
    return 0;
}

/* Default (empty) macroblock callback. */
void MPV_MbCbFn(void) {}

/* Frees the handle, invalidating its cache lines when the locked cache is enabled. */
Sint32 MPV_Destroy(MPV mpv) {
    register MPV p = mpv;
    register Sint32 i;

    if (MPVLIB_CheckHn(p) != 0) {
        return MPVERR_SetCode(NULL, 0xFF030201);
    }
    MPVM2V_Destroy(p);
    if (mpvlib_libwork.hid2 & MPVLIB_HID2_LCE) {
        for (i = 0; i < MPVLIB_HN_SIZE; i += 0x20) {
            asm { dcbi p, i }
        }
    }
    p->stat = MPVLIB_STAT_STOP;
    return 0;
}

/* The two block counters of the handle's IDCT parameter block. */
void MPV_GetDctCnt(MPV mpv, Sint32 *cnt0, Sint32 *cnt1) {
    *cnt0 = mpv->dctpa.cnt0;
    *cnt1 = mpv->dctpa.cnt1;
}

static void mpvlib_InitPicAtr(MPV_PICATR *atr);

/* Builds a handle: table pointers into the shared table area, motion compensation buffers, the
 * IDCT functions and parameter block, default conditions, cleared picture attributes and error
 * record, no user data joints.
 */
MPV mpvlib_InitHn(MPV mpv) {
    MPV_IXA *ixa = mpvlib_libwork.ixa;
    Sint32 i;
    DCT_PA *pa;

    mpv->rl_8 = mpvvlc_run_level_8;
    mpv->rl_4 = mpvvlc_run_level_4 - 4;
    mpv->rl_2 = mpvvlc_run_level_2 - 8;
    mpv->rl_1 = mpvvlc_run_level_1 - 8;
    mpv->rl_0a = mpvvlc_run_level_0a;
    mpv->rl_0b = mpvvlc_run_level_0b;
    mpv->rl_0c = mpvvlc_run_level_0c;
    mpv->group_tbl_p = ixa->group_tbl;
    mpv->rl_p = ixa->rl;
    mpv->zigzag_tbl = ixa->zigzag;
    mpv->bitmsk_tbl = ixa->bitmsk;
    mpv->scale_tbl = ixa->scale;
    mpv->clip_base = mpv_clip_0_255_base;
    mpv->mc.wk.clip = mpv->clip_base;
    mpv->mc.wk.buf0 = mpv->mcbuf[0];
    mpv->mc.wk.work = mpv->work;
    mpv->mc.wk.work2 = mpv->mc.wk.work + 0x180;
    mpv->tbl36c[0] = mpv->mcbuf[2];
    mpv->tbl36c[1] = mpv->mcbuf[3];
    mpv->tbl36c[2] = mpv->mcbuf[4];
    mpv->tbl36c[3] = mpv->mcbuf[5];
    mpv->tbl36c[4] = mpv->mcbuf[0];
    mpv->tbl36c[5] = mpv->mcbuf[1];
    mpv->x2e4 = DCT_FsriTrans6Blk;
    mpv->x2e8 = DCT_FsriTransCbp;
    mpv->x18c = 0;
    UTY_MemcpyDword((Uint32 *)mpv->cond, (Uint32 *)mpvlib_libwork.cond, MPVLIB_COND_NUM);
    MPVERR_InitErrInf(&mpv->errinf);
    MPVCMC_InitObj(mpv);
    pa = &mpv->dctpa;
    DCT_FsriInitPa(pa);
    pa->work = mpv->work;
    pa->blk = mpv->blk[0];
    pa->tbl = mpv->tbl36c;
    mpvlib_InitPicAtr(&mpv->picatr);
    mpv->pic_done = 0;
    mpv->npic = 0;
    mpv->dctbl_y = mpvvlc_y_dcsiz;
    mpv->dctbl_c = mpvvlc_c_dcsiz;
    mpv->m2v_mode = 0;
    mpv->stc[0] = 0;
    mpv->stc[1] = 0;
    mpv->stc[2] = 0;
    mpv->dcprec = 0;
    mpv->intra_func = MPVABDEC_IntraBlock;
    mpv->nintra_func = MPVABDEC_NintraBlock;
    mpv->x1344 = mpv->cond[7];
    for (i = 0; i < 4; i++) {
        MPV_SetUsrSj(mpv, i, NULL, NULL, NULL);
    }
    MPV_SetPicUsrBuf(mpv, NULL, 0);
    mpv->stat = MPVLIB_STAT_READY;
    return mpv;
}

/* first unused handle */
static inline MPV mpvlib_SearchFreeHn(void) {
    Sint32 nhn;
    MPV hn;
    Sint32 i;

    nhn = mpvlib_libwork.nhn;
    hn = mpvlib_libwork.hn;
    for (i = 0; i < nhn; i++) {
        if (hn->stat == MPVLIB_STAT_STOP) {
            return hn;
        }
        hn = MPVLIB_NEXT_HN(hn);
    }
    return NULL;
}

/* Takes a free handle (its cache lines zeroed in the locked cache when enabled), initialises it and
 * creates the MPEG-2 sub-object (none in this library).
 */
MPV MPV_Create(void) {
    register MPV mpv;
    register Sint32 i;
    register MPV hn;

    hn = mpvlib_SearchFreeHn();
    mpv = hn;
    if (mpv == NULL) {
        return NULL;
    }
    if (mpvlib_libwork.hid2 & MPVLIB_HID2_LCE) {
        for (i = 0; i < MPVLIB_HN_SIZE; i += 0x20) {
            asm {
                dcbi hn, i
                dcbz_l hn, i
            }
        }
    }
    mpv = mpvlib_InitHn(mpv);
    mpv->m2v = MPVM2V_Create(mpv);
    return mpv;
}

/* Library finish: invalidates the table area's cache lines when the locked cache is enabled. */
void MPV_Finish(void) {
    register Sint32 i;
    register MPV_IXA *ixa;

    MPVUMC_Finish();
    MPVM2V_Finish();
    ixa = mpvlib_libwork.ixa;
    if (mpvlib_libwork.hid2 & MPVLIB_HID2_LCE) {
        for (i = 0; i < MPVLIB_IXA_SIZE; i += 0x20) {
            asm { dcbi ixa, i }
        }
    }
}

/* Cleared picture attributes. */
static void mpvlib_InitPicAtr(MPV_PICATR *atr) {
    memset(atr, 0, 4);
    atr->width = 0;
    atr->height = 0;
    atr->mb_width = 0;
    atr->mb_height = 0;
    atr->frame_rate = 0;
    atr->temp_ref = 0;
    atr->pic_type = 0;
    atr->tc_drop = 0;
    atr->tc_hour = 0;
    atr->tc_min = 0;
    atr->tc_sec = 0;
    atr->tc_pic = 0;
    atr->ngop = 0;
    atr->nseq = 0;
    atr->x38 = 3;
    atr->x3c = 1;
    atr->x40 = 1;
    atr->x44 = 1;
    atr->bitrate = 0;
    atr->vbv_size = 0;
    atr->x50 = -1;
    atr->x52 = -1;
    atr->x54 = 0;
    atr->x55 = -1;
    atr->x56 = -1;
    atr->x57 = -1;
    atr->x58 = 0;
    atr->aspect = 1;
    atr->constrained = 0;
    atr->x5b = 0;
    atr->x5c = 0;
    atr->x5d = 0xFF;
    atr->x5e = -1;
    atr->x5f = -1;
    atr->x60 = -1;
    atr->x61 = 0;
    atr->x62 = 0xFF;
    atr->x63 = 0xFF;
    atr->x64 = 0xFF;
    atr->x68 = 0;
}

/* Library init in the caller's work area: checks the build (motion compensation buffer alignment,
 * VLC area size, handle size, the default condition table's guard word, the library version and
 * big-endian byte order), reads HID2 for the locked cache, lays out `nhn` handles followed by the
 * shared table area, builds the clip table and initialises the sub-modules.
 */
Sint32 MPV_Init(Sint32 nhn, void *work) {
    Sint32 ofs_mcbuf = offsetof(MPV_OBJ, mcbuf);
    Sint32 siz_obj = MPVLIB_HN_SIZE;
    Uint8 *hn_end;
    register MPV_OBJ *hn;
    register MPV_IXA *ixa;
    register Sint32 i;
    static const Uint32 test_wrok = 0x01020304;
    Uint32 hid2;
    Sint32 ret;
    Uint8 *p;

    cri_verstr_ptr = MPVLIB_version_str;
    /* the IDCT output buffer must start on a cache line */
    if ((ofs_mcbuf & 0x1F) != 0) {
        ret = MPVERR_SetCode(NULL, 0xFF03FF06);
    } else if (MPVVLC_IsVlcSizErr() != 0) {
        ret = MPVERR_SetCode(NULL, 0xFF03FF03);
    } else if ((Uint32)siz_obj > 0x2000) {
        ret = MPVERR_SetCode(NULL, 0xFF03FF01);
    } else if (mpvlib_cond_dfl[MPVLIB_COND_NUM] != MPVLIB_COND_GUARD) {
        ret = MPVERR_SetCode(NULL, 0xFF03FF02);
    } else if (MPVDEC_CheckVersion("1.958", sizeof(MPV_OBJ), sizeof(MPV_PICATR)) != 0) {
        ret = MPVERR_SetCode(NULL, 0xFF03FF07);
    } else {
        /* a little-endian build calls address -1 forever (original behaviour) */
        if (*(const Uint8 *)&test_wrok != 1) {
            for (;;) {
                ((void (*)(void))-1)();
            }
        }
        ret = 0;
    }
    if (ret != 0) {
        if (ret == 0xFF03FF05) {
            return ret;
        }
        /* every other build check failure hangs here (original behaviour) */
        for (;;) {
        }
    }

    asm {
        mfspr r0, 920
        stw r0, hid2
    }
    if (mpvlib_use_lc == 0) {
        hid2 &= ~MPVLIB_HID2_LCE;
    }
    mpvlib_libwork.hid2 = hid2;
    hn = (MPV_OBJ *)(((Uint32)work + 0x1F) & ~0x1F);
    UTY_MemsetDword(hn, 0, (Uint32)(nhn * 0x2000 + 0x2000) / 4);
    hn_end = (Uint8 *)hn + nhn * MPVLIB_HN_SIZE;
    ixa = (MPV_IXA *)(hn_end + MPVLIB_IXA_GAP);
    if (mpvlib_libwork.hid2 & MPVLIB_HID2_LCE) {
        for (i = 0; i < MPVLIB_IXA_SIZE; i += 0x20) {
            asm {
                dcbi ixa, i
                dcbz_l ixa, i
            }
        }
    }
    UTY_MemcpyDword((Uint32 *)mpvlib_libwork.cond, (Uint32 *)mpvlib_cond_dfl, MPVLIB_COND_NUM);
    mpvlib_libwork.hn_end = hn_end;
    mpvlib_libwork.ixa = ixa;
    mpvlib_libwork.nhn = nhn;
    mpvlib_libwork.hn = hn;
    MPVERR_Init();
    MPVHDEC_Init();
    MPVFRM_Init();
    MPVVLC_Init(ixa->vlc, ixa);
    MPVBDEC_Init(ixa);
    MPVUMC_Init();
    MPVCMC_Init();
    p = mpv_clip_0_255_tbl;
    for (i = 0; i < 0x180; i++) {
        *p++ = 0;
    }
    for (i = 0; i < 0x100; i++) {
        *p++ = i;
    }
    for (i = 0; i < 0x180; i++) {
        *p++ = 0xFF;
    }
    mpv_clip_0_255_base = &mpv_clip_0_255_tbl[0x180];
    if (ixa->clip != NULL) {
        UTY_MemcpyDword((Uint32 *)ixa->clip, (Uint32 *)mpv_clip_0_255_tbl, 0x100);
        mpv_clip_0_255_base = &ixa->clip[0x180];
    }
    {
        Sint32 n = mpvlib_libwork.nhn;
        MPV_OBJ *hn2 = mpvlib_libwork.hn;

        for (i = 0; i < n; i++) {
            hn2->stat = MPVLIB_STAT_STOP;
            hn2 = MPVLIB_NEXT_HN(hn2);
        }
    }
    DCT_FsriInit();
    DCT_FsriInitScaleTbl(ixa->scale);
    MPVM2V_Init();
    return 0;
}
