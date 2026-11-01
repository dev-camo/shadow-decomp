/* CRI Sofdec MW player creation and destruction (mwsfdcre.c): the creation of the SFD handle, the
 * converter and the other parts of a player, the work they are carved from, and their destruction.
 *
 * The functions, their structure and most names follow the Resident Evil 4 GameCube decompilation
 * of the same file. This version differs from it in the player handle layout, in the creation
 * checks (the frame pool limit) and in the order of the functions.
 *
 * The original object also held functions the linker removed. The component work allocator
 * (MWSFD_Malloc) is inlined everywhere and written here as it was; of the others only the error
 * texts and the variables they used remain in the original, so they are reproduced by the marked
 * stand-ins.
 *
 * Status: NonMatching. Exact: mwSfdDestroy, MWSFCRE_ResetSfdHn, mwsfcre_MallocRfb,
 * MWSFCRE_SetSupplySj and mwPlyCalcWorkCprmSfd. mwsfcre_MallocCompoWork and mwPlyCreateSofdec
 * differ in register colouring only; mwPlyCalcWorkSfd in register colouring and in the order of
 * the last two additions. mwsfcre_CreateSfd is 14 words short: the original keeps the component
 * allocator's negative size test in the allocations of the SFD handle work and of the 0x700-byte
 * block (it folds that test for the other constant sizes, so these two probably went through a
 * wrapper whose form is unknown), and each of its two ADXT file type tests has a switch arm for the
 * value 4 with no code of its own; the rest is register colouring. The data, rodata and bss match.
 */
#include <string.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"

/* driver tables of the three file types, in driver order (the unused slots are 0). The creation
 * parameters declare them as pointers to function tables while the entries are the drivers'
 * interface records. */
static const SFD_TR_IF *const mwsfd_mps_trsetup[SFD_TR_NUM] = {
    &SFD_tr_in_mem, &SFD_tr_sd_mps,  &SFD_tr_vd_mpv,    &SFD_tr_ad_adxt, NULL,
    NULL,           &SFD_tr_vo_manu, &SFD_tr_ao_auto_p, &SFD_tr_uo,
};
static const SFD_TR_IF *const mwsfd_mpv_trsetup[SFD_TR_NUM] = {
    &SFD_tr_in_mem, NULL, &SFD_tr_vd_mpv, NULL, NULL, NULL, &SFD_tr_vo_manu, NULL, NULL,
};
static const SFD_TR_IF *const mwsfd_vonlysfd_trsetup[SFD_TR_NUM] = {
    &SFD_tr_in_mem, &SFD_tr_sd_mps,  &SFD_tr_vd_mpv, NULL,       NULL,
    NULL,           &SFD_tr_vo_manu, NULL,           &SFD_tr_uo,
};

#define MWSFD_HNWORK_SIZE 0x4000
#define MWSFD_PICUSR_SIZE 0x800
#define MWSFD_PICUSR_UNIT 0x40
#define MWSFD_FNAME_SIZE 0x100
#define MWSFD_AINFSJ_BSIZE 0x20000

/* Creation parameters of the SFD handle of each file type; mwsfcre_CreateSfd fills in the
 * buffers. */
static SFD_CREPRM mwsfd_sfdmps_crepara = {
    (SFD_TR_FUNC **)mwsfd_mps_trsetup,       /* trif_tbl */
    0,                                       /* adr, set at creation */
    {0x10000, 0x50800, 0x12000, 0, 0, 0, 0}, /* size */
    0,                                       /* x24 */
    0x800,                                   /* unit */
    3,                                       /* x2c */
    0,                                       /* x30 */
    0,                                       /* x34 */
    3,                                       /* x38 */
    NULL,                                    /* hnwork */
    0,                                       /* hnwksiz */
};
static SFD_CREPRM mwsfd_sfdmpv_crepara = {
    (SFD_TR_FUNC **)mwsfd_mpv_trsetup,       /* trif_tbl */
    0,                                       /* adr, set at creation */
    {0x10000, 0x50800, 0x12000, 0, 0, 0, 0}, /* size */
    0,                                       /* x24 */
    0x800,                                   /* unit */
    3,                                       /* x2c */
    0,                                       /* x30 */
    0,                                       /* x34 */
    3,                                       /* x38 */
    NULL,                                    /* hnwork */
    0,                                       /* hnwksiz */
};
static SFD_CREPRM mwsfd_vonlysfd_crepara = {
    (SFD_TR_FUNC **)mwsfd_vonlysfd_trsetup,  /* trif_tbl */
    0,                                       /* adr, set at creation */
    {0x10000, 0x50800, 0x12000, 0, 0, 0, 0}, /* size */
    0,                                       /* x24 */
    0x800,                                   /* unit */
    3,                                       /* x2c */
    0,                                       /* x30 */
    0,                                       /* x34 */
    3,                                       /* x38 */
    NULL,                                    /* hnwork */
    0,                                       /* hnwksiz */
};
static SFMPV_PARA mwsfd_mpvpara = {0xC0, 0xF0, 0x160, 0x1E0, 0, 0x160, 0x1E0, 4, 0};
static SFADXT_PARA mwsfd_adxtpara = {0x5DCC, 0x120, NULL, 2, 0xBB80, 0xC1C0, NULL};
static Sint32 mwsfd_packsize = 0x800;

/* user-supplied frame buffers (set by a stripped function, see mwsfdcre_StrippedSetFrmBuf) */
static Sint32 mwsfdcre_bufnum;
static Sint32 mwsfdcre_bufsize;
static void *mwsfdcre_bufptr[16];

/* component buffer sizes decided by mwsfcre_CreateSfd */
static Sint32 adxibuf;       /* ADXT input ring buffer */
static Sint32 adxwk;         /* ADXT work */
static Sint32 tab;           /* decoded frame buffers */
static Sint32 rfb;           /* reference frame buffers */
static Sint32 aib;           /* audio input buffer */
static Sint32 vib;           /* video input buffer */
static Sint32 sib;           /* system input buffer */
static Sint32 sjb;           /* file stream joint ring buffer */
static Uint32 mwsfd_sisjadr; /* 64-byte aligned start of the stream joint ring buffer */

/* Takes size bytes of component work: from the user work when one was given, else through the
 * library's allocator callback. Every block is remembered so that it can be given back. */
static void *MWSFD_Malloc(MWPLY mwply, Sint32 size) {
    MWSFD_LIBWORK *lw;
    void *ptr;

    if (mwply->cwk_cnt >= MWSFD_CWK_NUM) {
        MWSFSVM_Error("E2053001 MWSFD_Malloc: cnt over.");
        return NULL;
    }
    if (size < 0) {
        return NULL;
    }
    if (mwply->cwk_buf != NULL) {
        if (mwply->cwk_used + size > mwply->cwk_size) {
            ptr = NULL;
        } else {
            ptr = mwply->cwk_ptr;
            mwply->cwk_ptr += size;
            mwply->cwk_used += size;
        }
    } else {
        lw = MWSFLIB_GetLibWorkPtr();
        ptr = lw->malloc_fn(lw->mem_obj, size);
    }
    if (ptr != NULL) {
        mwply->cwk_tbl[mwply->cwk_cnt] = ptr;
        mwply->cwk_cnt++;
    }
    return ptr;
}

/* Gives back every block of component work the player holds. The blocks are carved from the user
 * work (nothing to free) or taken through the library's allocator callbacks. Helper and name
 * inferred, after the same helper in the RE4 decompilation. */
static void mwsfcre_FreeAll(MWPLY mwply) {
    Sint32 i;

    for (i = 0; i < MWSFD_CWK_NUM; i++) {
        void *blk = mwply->cwk_tbl[MWSFD_CWK_NUM - 1 - i];

        if (blk != NULL) {
            if (mwply->cwk_buf == NULL) {
                MWSFD_LIBWORK *lw = MWSFLIB_GetLibWorkPtr();

                lw->free_fn(lw->mem_obj, blk);
            }
            mwply->cwk_cnt--;
            mwply->cwk_tbl[MWSFD_CWK_NUM - 1 - i] = NULL;
        }
    }
}

/* Matching workaround: stand-in for mwPlyGetHdrInf, which the linker removed from the game. Only
 * its two error texts remain in the original, between the component work allocator's text and the
 * destruction text. RE4 has the body, but it needs the header analysis types and callees
 * (SFD_AnalyCreInf, MWSFFRM_AnalyTotalFrmNum), which this game does not contain. Revisit when
 * they are established. */
static void mwsfdcre_StrippedGetHdrInf(void) {
    MWSFSVM_Error("E204161: mwPlyGetHdrInf(): NULL pointer");
    MWSFSVM_Error("E204162: mwPlyGetHdrInf(): bufsize error");
}

/* Destroys the SFD handle. */
static inline void MWSFCRE_DestroySfd(SFD sfd) {
    if (SFD_Destroy(sfd) != 0) {
        MWSFLIB_SetErrCode(-0x132);
        MWSFSVM_Error("E20010703E MWSFCRE_DestroySfd: ");
    }
}

/* Destroys the player: the decoder, the converter, the load scheduler, the stream controller and
 * the stream joints, then returns the component work and clears the handle.
 *
 * Matching workaround: the handle is taken as void *, like the other entries of the player. With
 * a typed parameter the copies of this function inlined into mwPlyCreateSofdec lack a register
 * copy of the handle that the original has (7 differing words in that function). Revisit when the
 * player interface table is sourced. */
void mwSfdDestroy(void *hn) {
    MWPLY mwply = hn;

    if (mwply == NULL) {
        return;
    }
    mwSfdStopDec(mwply);
    mwply->used = 0;
    MWSFTAG_DestroyAinfSj(mwply);
    if (mwply->sfx != NULL) {
        MWSFSFX_Destroy(mwply->sfx);
    }
    if (mwply->lsc != NULL) {
        LSC_Destroy(mwply->lsc);
    }
    if (mwply->stm != NULL) {
        MWSTM_Destroy(mwply->stm);
    }
    if (mwply->file_sj != NULL) {
        SJ_Destroy(mwply->file_sj);
    }
    if (mwply->mem_sj != NULL) {
        SJ_Destroy(mwply->mem_sj);
    }
    if (mwply->sfd != NULL) {
        MWSFCRE_DestroySfd(mwply->sfd);
    }
    MWSST_Destroy(&mwply->sst);
    mwsfcre_FreeAll(mwply);
    if (mwply->cwk_cnt != 0) {
        MWSFSVM_Error("E2053005: forgot free.");
    }
    memset(mwply, 0, sizeof(MWPLY_OBJ));
    mwply->used = 0;
}

/* Takes the work of the frame converter handle and, for a layout that uses it, the buffer of the
 * additional-info stream joint; -1 when the component work is too short. */
Sint32 mwsfcre_MallocCompoWork(MWPLY mwply) {
    MWSFD_CRPRM *prm = &mwply->prm;
    Sint32 size;
    void *wk;

    size = MWSFSFX_CalcHnWorkSiz(prm->max_width, prm->max_height);
    wk = MWSFD_Malloc(mwply, size);
    if (wk == NULL) {
        MWSFSVM_Error("E2053003: not enough work: sfx_wk");
        mwsfcre_FreeAll(mwply);
        return -1;
    }
    mwply->sfx_wk = wk;
    mwply->sfx_wksiz = size;
    if (MWSFTAG_IsUseAinfSj(prm) == TRUE) {
        wk = MWSFD_Malloc(mwply, MWSFD_AINFSJ_BSIZE);
        if (wk == NULL) {
            MWSFSVM_Error("E2053004: not enough work: ainfsj_buf");
            mwsfcre_FreeAll(mwply);
            return -1;
        }
        mwply->ainf_buf = wk;
        mwply->ainf_bsize = MWSFD_AINFSJ_BSIZE;
    } else {
        mwply->ainf_buf = NULL;
        mwply->ainf_bsize = 0;
    }
    return 0;
}

/* Matching workaround: stand-in for the setter of the user-supplied frame buffers, which the
 * linker removed from the game; its body is unknown. The original orders .bss by first reference,
 * and this function stands where the first reference of these variables was. It only references
 * them in the order the original's .bss requires: the buffer count and size, the pointers, then
 * the sizes mwsfcre_CreateSfd decides. Revisit when the stripped code is known. */
static void mwsfdcre_StrippedSetFrmBuf(void) {
    mwsfdcre_bufnum = 0;
    mwsfdcre_bufsize = 0;
    mwsfdcre_bufptr[0] = NULL;
    adxibuf = 0;
    adxwk = 0;
    tab = 0;
    rfb = 0;
    aib = 0;
    vib = 0;
    sib = 0;
    sjb = 0;
    mwsfd_sisjadr = 0;
}

/* Checks the creation parameters: only the frame buffer formats 0 (default) and 3 (planar YCC
 * 4:2:0) are accepted, and the frame pool is limited to 14 frames. Name inferred (RE4's check
 * covers the format only). */
static Bool mwsfcre_IsValidPrm(MWSFD_CRPRM *cprm) {
    Bool ret = TRUE;

    if (cprm->buffmt != 0 && cprm->buffmt != 3) {
        MWSFSVM_Error("E3012102: Buffer format is invalid.");
        ret = FALSE;
    }
    if (cprm->nfrm_pool_wk > 14) {
        MWSFSVM_Error("E4110901: Number of frame pool exceeds 14.");
        ret = FALSE;
    }
    return ret;
}

/* -1 when the component work has to come from the library callbacks and none were registered */
static Sint32 mwsfcre_ChkMallocFn(MWSFD_CRPRM *cprm) {
    MWSFD_LIBWORK *lw = MWSFLIB_GetLibWorkPtr();
    Sint32 ret = 0;

    if (cprm->work == NULL) {
        if (lw->malloc_fn == NULL) {
            ret = -1;
        }
        if (lw->free_fn == NULL) {
            ret = -1;
        }
    }
    return ret;
}

/* Starts the component allocator on the caller's work (MWSFD_CRPRM.work / wksize): nothing used, no
 * blocks recorded. */
static void mwsfcre_InitCompoWork(MWPLY mwply, MWSFD_CRPRM *cprm) {
    Sint32 i;

    mwply->cwk_buf = cprm->work;
    mwply->cwk_size = cprm->wksize;
    mwply->cwk_ptr = cprm->work;
    mwply->cwk_used = 0;
    mwply->cwk_cnt = 0;
    for (i = 0; i < MWSFD_CWK_NUM; i++) {
        mwply->cwk_tbl[i] = NULL;
    }
}

/* Gives the SFD handle the player's picture user data buffer (at creation, and again in
 * MWSFCRE_ResetSfdHn). */
static inline void mwsfcre_AttachPicUsrBuf(MWPLY mwply) {
    MWSFD_PICUSR *picusr;
    Uint8 *buf;
    Sint32 usize;
    Sint32 nfrm;

    picusr = mwply->picusr_ptr;
    if (picusr == NULL) {
        MWSFSVM_Error("E02120501: Internal Error: mwsfcre_AttachPicUsrBuf().");
        return;
    }
    nfrm = mwply->prm.nfrm_pool_wk;
    usize = picusr->usize;
    buf = picusr->buf;
    if (picusr->bsize < (nfrm + 3) * usize) {
        MWSFSVM_Error("E02120502: mwsfcre_AttachPicUsrBuf(): usrdatbuf is short.");
        return;
    }
    if (MWSFD_GetUsePicUsr() == 1) {
        SFD_SetPicUsrBuf(mwply->sfd, buf, nfrm + 3, usize);
    }
}

/* Sets the conditions of a new SFD handle. Three of them (0x2D, 0x2C and 0x2A) take the library's
 * frame pool size scaled by its x08 setting and by 1000, rounded to the nearest integer. */
static inline void mwsfcre_SetSfdCond(MWPLY mwply, MWSFD_LIBWORK *lw) {
    Float32 ftime;
    Sint32 cond;
    Sint32 npool;
    Sint32 x08;
    SFD sfd;

    sfd = mwply->sfd;
    npool = lw->nfrm_pool;
    x08 = lw->x08;
    SFD_SetCond(sfd, 8, 0);
    SFD_SetCond(sfd, 1, 1);
    SFD_SetCond(sfd, 0, 0);
    SFD_SetCond(sfd, 0x17, 4);
    ftime = 0.5f + (Float32)(x08 * npool * 1000);
    cond = (Sint32)ftime;
    if ((Float32)cond > ftime) {
        cond--;
    }
    SFD_SetCond(sfd, 0x2D, cond);
    SFD_SetCond(sfd, 0x2C, cond);
    SFD_SetCond(sfd, 0x2A, cond);
    SFD_SetCond(sfd, 0xF, 2);
    SFD_SetCond(sfd, 0x33, 0);
    SFD_SetCond(sfd, 0xE, 0);
    SFD_SetCond(sfd, 0x1C, 0);
    SFD_SetMpvCond(sfd, 5, 0);
}

/* Component buffer sizes of a file type, from the creation parameters: the system, video and audio
 * input buffers, the file stream joint ring buffer (nsec seconds of the maximum bit rate, in whole
 * 2048-byte sectors) and the ADXT buffers. */
static inline void mwsfcre_CalcBufSize(MWSFD_CRPRM *cprm, Sint32 *sibsiz, Sint32 *vibsiz,
                                       Sint32 *aibsiz, Sint32 *sjbsiz, Sint32 *adxibsiz,
                                       Sint32 *adxwksiz) {
    Sint32 nsec = cprm->nsec;
    Sint32 mode = cprm->mode;
    Sint32 bps = cprm->max_bps;

    if (nsec <= 0) {
        nsec = 1;
    }
    if (mode == MWSFD_FTYPE_MPV) {
        *sibsiz = 0;
        *vibsiz = 0;
        *aibsiz = 0;
        *adxibsiz = 0;
        *adxwksiz = 0;
        *sjbsiz = nsec * (bps / 8 / 0x800 * 0x800);
    } else if (mode == MWSFD_FTYPE_VONLYSFD) {
        *sibsiz = 0;
        *aibsiz = 0;
        *adxibsiz = 0;
        *adxwksiz = 0;
        *sjbsiz = nsec * (bps / 8 / 0x800 * 0x800);
        *vibsiz = bps / 8 / 0x800 * 0x800 / 2 + 0x800;
    } else {
        *sibsiz = 0;
        *aibsiz = 0x5DCC;
        *sjbsiz = nsec * (bps / 8 / 0x800 * 0x800);
        *vibsiz = bps / 8 / 0x800 * 0x800 / 2 + 0x800;
        *adxibsiz = 0x5F0C;
        *adxwksiz = 0xC1C0;
    }
}

/* One YCC 4:2:0 frame: 16-aligned dimensions, 32-byte rows, 32 bytes of slack. */
static inline Sint32 mwsfcre_CalcYccSize(Sint32 width, Sint32 height) {
    Sint32 h16;
    Sint32 w16;
    Sint32 ysize;
    Sint32 csize;

    w16 = (width + 15) / 16 * 16;
    h16 = (height + 15) / 16 * 16;
    ysize = h16 * ((w16 + 31) / 32 * 32);
    csize = (h16 / 2) * ((w16 / 2 + 31) / 32 * 32);

    return ysize + csize * 2 + 0x20;
}

/* Frame buffer size of the creation parameters; the format is checked again at every use. */
static inline Sint32 mwsfcre_CalcFrmSize(MWSFD_CRPRM *cprm) {
    Sint32 width;
    Sint32 height;

    width = cprm->max_width;
    height = cprm->max_height;
    if (cprm->buffmt >= 4 || cprm->buffmt < 0) {
        MWSFSVM_Error("E206011: MwsfdCrePrm: illigal buffmt.");
    }
    return mwsfcre_CalcYccSize(width, height);
}

/* Sizes of the two reference frame buffers and of the decoded frame buffers that come from the
 * component work: none when the user supplied frame buffers. */
static inline void mwsfcre_CalcFrmBufSize(MWSFD_CRPRM *cprm, Sint32 *rfbsiz, Sint32 *tabsiz) {
    Sint32 nfrm;
    Sint32 fsize;

    if (mwsfdcre_bufnum != 0) {
        *rfbsiz = 0;
        *tabsiz = 0;
    } else {
        nfrm = cprm->nfrm_pool_wk;
        fsize = mwsfcre_CalcFrmSize(cprm);
        *rfbsiz = fsize * 2;
        *tabsiz = nfrm * fsize;
    }
}

/* Whether the file type has an ADXT audio decoder. */
static Bool mwsfcre_IsUseAdxt(Sint32 mode) {
    switch (mode) {
    case MWSFD_FTYPE_MPV:
    case MWSFD_FTYPE_VONLYSFD:
        return FALSE;
    default:
        return TRUE;
    }
}

static SFD mwsfcre_CreateSfd(MWPLY mwply, MWSFD_CRPRM *cprm);

/* Creates a player handle from the creation parameters: takes one of the eight MWPLY slots, builds
 * the SFD decoder (mwsfcre_CreateSfd) with the buffers of the file type, sets its conditions,
 * creates the file and memory stream joints, the stream controller and load scheduler that feed the
 * file joint, the frame converter and the additional-info joint, and installs the Sofdec header
 * callback. Returns NULL with an error message on any failure. */
MWPLY mwPlyCreateSofdec(MWSFD_CRPRM *cprm) {
    MWSFD_LIBWORK *lw;
    MWPLY mwply;
    SFD sfd;
    Sint32 sibsiz;
    Sint32 vibsiz;
    Sint32 aibsiz;
    Sint32 sjbsiz;
    Sint32 adxibsiz;
    Sint32 adxwksiz;
    Sint32 i;
    SFX_OBJ *sfx;

    if (cprm == NULL) {
        MWSFSVM_Error("E1122612 mwPlyCreateSofdec : cprm is NULL.");
        return NULL;
    }
    if (mwsfcre_IsValidPrm(cprm) != TRUE) {
        return NULL;
    }
    lw = MWSFLIB_GetLibWorkPtr();
    for (i = 0; i < MWSFD_MAX_HN; i++) {
        mwply = &lw->hn[i];
        if (mwply->used != 1) {
            break;
        }
    }
    if (i == MWSFD_MAX_HN) {
        MWSFLIB_SetErrCode(-0xB);
        MWSFSVM_Error(
            "E4061801 mwPlyCreateSofdec: Number of MWPLY handles exceeds its maximum number.");
        return NULL;
    }
    if (mwsfcre_ChkMallocFn(cprm) == -1) {
        MWSFSVM_Error("E2053006 mwPlyCreateSofdec: Didn't set malloc/free func.");
        return NULL;
    }
    /* the original tests the slot again here although the loop above always set it */
    if (mwply != NULL) {
        memset(mwply, 0, sizeof(MWPLY_OBJ));
    }
    mwsfcre_InitCompoWork(mwply, cprm);
    mwply->prm = *cprm;
    sfd = mwsfcre_CreateSfd(mwply, cprm);
    mwply->sfd = sfd;
    if (mwply->sfd == NULL) {
        MWSFSVM_Error("E2012 mwPlyCreate:can't create SFD");
        mwSfdDestroy(mwply);
        return NULL;
    }
    mwsfcre_AttachPicUsrBuf(mwply);
    mwsfcre_CalcBufSize(cprm, &sibsiz, &vibsiz, &aibsiz, &sjbsiz, &adxibsiz, &adxwksiz);
    mwsfcre_SetSfdCond(mwply, lw);
    mwply->file_sj = SJRBF_Create((void *)mwply->x1c4, mwply->flow_nsct, mwply->x1cc);
    if (mwply->file_sj == NULL) {
        MWSFSVM_Error("E2013 mwPlyCreate:can't create SJ");
        mwSfdDestroy(mwply);
        return NULL;
    }
    mwply->mem_sj = SJMEM_Create(NULL, 0);
    if (mwply->mem_sj == NULL) {
        MWSFSVM_Error("E2020 mwPlyCreate:can't create SJ");
        mwSfdDestroy(mwply);
        return NULL;
    }
    mwply->x58 = sjbsiz;
    mwply->x38 = 1;
    mwply->dec_svr_flg = 0;
    mwply->stat = MWSFD_STAT_STOP;
    mwply->compo_fix = cprm->compo;
    mwply->compo = cprm->compo;
    if (SFD_GetTrHn(sfd, SFADXT_TR, &mwply->x44) != 0) {
        mwply->x44 = NULL;
    }
    mwply->x6c = 1;
    mwply->linkstm = 0;
    mwply->linkstm_req = 0;
    mwply->pause_flg = 0;
    mwply->x73 = 0;
    mwply->sleep_bdr = 0;
    MWSFSVR_SetHnMwplySvrFlg(mwply, 0);
    MWSFSVR_SetHnSfdSvrFlg(mwply, 0);
    mwply->noskip = 1;
    mwply->nskipdisp = 0;
    mwply->stm = MWSTM_Create(mwply->file_sj);
    if (mwply->stm == NULL) {
        mwSfdDestroy(mwply);
        return NULL;
    }
    MWSFPLY_SetFlowLimit(mwply);
    mwply->lsc = LSC_Create(mwply->file_sj);
    mwply->x74 = 0;
    LSC_SetStmHndl(mwply->lsc, mwply->stm);
    if (mwsfcre_MallocCompoWork(mwply) == -1) {
        mwSfdDestroy(mwply);
        return NULL;
    }
    sfx = MWSFSFX_Create(mwply->sfx_wk, mwply->sfx_wksiz, cprm->max_width, cprm->max_height);
    if (sfx == NULL) {
        MWSFSVM_Error("E201185: can't create SfxHn");
        mwSfdDestroy(mwply);
        return NULL;
    }
    mwply->sfx = sfx;
    MWSFSFX_SetCompoMode(mwply, mwply->prm.compo);
    mwply->ainf_sj = MWSFTAG_CreateAinfSj(mwply);
    if (MWSFTAG_SetAinfSj(mwply) != 0) {
        MWSFSVM_Error("E201212 mwPlyCreate: can't set AddInfSJ");
        mwSfdDestroy(mwply);
        return NULL;
    }
    MWSFTAG_InitTagInf(mwply);
    MWSFFRM_InitSfhInfTable(mwply);
    MWSFFRM_SetShfCbFn(mwply);
    mwply->used = 1;
    return mwply;
}

/* Stops the SFD handle, installs the player's error function in it again and gives it the
 * picture user data buffer again, for a new play; 0, or -1 when that fails. */
Sint32 MWSFCRE_ResetSfdHn(MWPLY mwply) {
    SFD sfd;

    sfd = mwply->sfd;
    mwply->sfd = NULL;
    if (SFD_Stop(sfd) != 0) {
        MWSFSVM_Error("E0203261: MWSFCRE_ResetSfdHn: SFD_Stop() failed.");
        return -1;
    }
    mwply->sfd = sfd;
    if (SFD_SetErrFn(sfd, MWSFLIB_SfdErrFunc, mwply) != 0) {
        MWSFLIB_SetErrCode(-0x12F);
        MWSFSVM_Error("E0203262: MWSFCRE_ResetSfdHn: SFD_SetErrFn() failed.");
        return -1;
    }
    mwsfcre_AttachPicUsrBuf(mwply);
    return 0;
}

/* Matching workaround: stand-in for mwPlyAttachPicUsrBuf, which the linker removed from the game.
 * Only its error text remains in the original, after the texts of MWSFCRE_ResetSfdHn. Revisit when
 * the stripped code is known. */
static void mwsfdcre_StrippedAttachPicUsrBuf(void) {
    MWSFSVM_Error("E02120503: mwPlyAttachPicUsrBuf(): bufsize is short.");
}

/* Converts the creation buffer format to the SFD frame buffer format (0 and 3 are planar). */
static Sint32 mwsfcre_CnvBufFmt(Sint32 buffmt) {
    Sint32 fmt;

    switch (buffmt) {
    case 0:
        fmt = 3;
        break;
    case 1:
        fmt = 1;
        break;
    case 2:
        fmt = 2;
        break;
    case 3:
        fmt = 3;
        break;
    default:
        MWSFSVM_Error("E206011: MwsfdCrePrm: illigal buffmt.");
        fmt = 3;
        break;
    }
    return fmt;
}

/* Takes the decoded frame buffers: user frame buffers 2.. or one component allocation per frame. */
static inline Sint32 mwsfcre_MallocFrmTbl(MWPLY mwply, MWSFD_CRPRM *cprm, void **frmtbl) {
    Sint32 ret = 0;
    Sint32 nfrm = cprm->nfrm_pool_wk;
    Sint32 fsize;
    Sint32 i;

    fsize = mwsfcre_CalcFrmSize(cprm);
    if (mwsfdcre_bufnum != 0) {
        if (mwsfdcre_bufnum < nfrm + 2 || mwsfdcre_bufsize < fsize) {
            ret = -1;
        } else {
            for (i = 0; i < nfrm; i++) {
                frmtbl[i] = mwsfdcre_bufptr[i + 2];
                if (frmtbl[i] == NULL) {
                    ret = -1;
                }
            }
        }
    } else {
        for (i = 0; i < nfrm; i++) {
            frmtbl[i] = MWSFD_Malloc(mwply, fsize);
            if (frmtbl[i] == NULL) {
                ret = -1;
            }
        }
    }
    return ret;
}

/* Sets the MPEG video decoder parameters for the maximum picture size and the frame pool; helper
 * and name inferred. The original reads the picture size from the creation parameters again here
 * instead of using the values mwsfcre_CreateSfd already holds, which points to code of its own. */
static inline void mwsfcre_SetMpvPara(MWSFD_CRPRM *cprm, Sint32 nfrm) {
    Sint32 w;
    Sint32 h;

    w = cprm->max_width;
    h = cprm->max_height;
    mwsfd_mpvpara.cwidth = (w / 2 + 31) / 32 * 32;
    mwsfd_mpvpara.cheight = h / 2;
    mwsfd_mpvpara.width = w;
    mwsfd_mpvpara.height = h;
    mwsfd_mpvpara.x10 = 0;
    mwsfd_mpvpara.max_width = w;
    mwsfd_mpvpara.max_height = h;
    mwsfd_mpvpara.nfrm = nfrm;
    mwsfd_mpvpara.x20 = 0;
}

/* Creates the SFD decoder handle of a player: decides the component buffer sizes, takes them from
 * the component work and fills in the creation parameters of the file type. NULL on failure. */
static SFD mwsfcre_CreateSfd(MWPLY mwply, MWSFD_CRPRM *cprm) {
    Sint32 mode = cprm->mode;
    void *adxibuf_p;
    void *adxwk_p;
    Sint32 nfrm = cprm->nfrm_pool_wk;
    Sint32 width = cprm->max_width;
    Sint32 height = cprm->max_height;
    void *rfbbuf[2];
    void *frmtbl[16];
    SFD_CREPRM crepara;
    void *cwk1;
    void *cwk2;
    Sint32 rfbret;
    Sint32 frmret;
    void *picusr_p;
    void *hnwork_p;
    void *buf700_p;
    void *fname_p;
    Sint32 fmt;
    MWSFD_PICUSR *picusr;
    SFD sfd;

    mwsfcre_CalcBufSize(cprm, &sib, &vib, &aib, &sjb, &adxibuf, &adxwk);
    mwsfcre_CalcFrmBufSize(cprm, &rfb, &tab);
    cwk1 = MWSFD_Malloc(mwply, sib + vib + aib + 0x20);
    cwk2 = MWSFD_Malloc(mwply, sjb + 0x40);
    rfbret = mwsfcre_MallocRfb(mwply, cprm, rfbbuf);
    frmret = mwsfcre_MallocFrmTbl(mwply, cprm, frmtbl);
    if (mwsfcre_IsUseAdxt(mode) == TRUE) {
        adxibuf_p = MWSFD_Malloc(mwply, adxibuf);
        adxwk_p = MWSFD_Malloc(mwply, adxwk);
    } else {
        adxibuf_p = NULL;
        adxwk_p = NULL;
    }
    picusr_p = MWSFD_Malloc(mwply, MWSFD_PICUSR_SIZE);
    hnwork_p = MWSFD_Malloc(mwply, MWSFD_HNWORK_SIZE);
    /* the 0x700-byte block is checked below but never stored */
    buf700_p = MWSFD_Malloc(mwply, 0x700);
    fname_p = MWSFD_Malloc(mwply, MWSFD_FNAME_SIZE);
    if (cwk1 == NULL || cwk2 == NULL || rfbret != 0 || frmret != 0 || picusr_p == NULL ||
        hnwork_p == NULL || fname_p == NULL || buf700_p == NULL) {
        MWSFSVM_Error("E2053002: not enough work");
        mwsfcre_FreeAll(mwply);
        return NULL;
    }
    if (mwsfcre_IsUseAdxt(mode) == TRUE) {
        if (adxibuf_p == NULL || adxwk_p == NULL) {
            MWSFSVM_Error("E4041301: not enough work");
            mwsfcre_FreeAll(mwply);
            return NULL;
        }
    }
    mwsfd_sisjadr = ((Uint32)cwk2 + 0x3F) & ~0x3F;
    mwsfcre_SetMpvPara(cprm, nfrm);
    mwsfd_adxtpara.buf = adxibuf_p;
    mwsfd_adxtpara.work = adxwk_p;
    switch (mode) {
    case MWSFD_FTYPE_SFD:
        crepara = mwsfd_sfdmps_crepara;
        mwply->x1c4 = mwsfd_sisjadr;
        mwply->flow_nsct = sjb - mwsfd_packsize;
        mwply->x1cc = mwsfd_packsize;
        break;
    case MWSFD_FTYPE_MPV:
        crepara = mwsfd_sfdmpv_crepara;
        mwply->x1c4 = mwsfd_sisjadr;
        mwply->flow_nsct = sjb - 0x800;
        mwply->x1cc = 0x800;
        break;
    case MWSFD_FTYPE_VONLYSFD:
        crepara = mwsfd_vonlysfd_crepara;
        mwply->x1c4 = mwsfd_sisjadr;
        mwply->flow_nsct = sjb - mwsfd_packsize;
        mwply->x1cc = mwsfd_packsize;
        break;
    default:
        break;
    }
    crepara.unit = mwsfd_packsize;
    if (sib != 0) {
        sib = sib - sib % mwsfd_packsize;
    }
    fmt = mwsfcre_CnvBufFmt(cprm->buffmt);
    crepara.adr = (Uint32)cwk1;
    crepara.size[0] = sib;
    crepara.size[1] = vib;
    crepara.size[2] = aib;
    crepara.x2c = nfrm;
    crepara.x30 = width;
    crepara.x34 = height;
    crepara.x38 = fmt;
    crepara.hnwork = hnwork_p;
    crepara.hnwksiz = MWSFD_HNWORK_SIZE;
    /* SFD_SetMpvParaTbl declares the buffer address tables as arrays of words. */
    SFD_SetMpvParaTbl(&mwsfd_mpvpara, (Uint32 *)rfbbuf, (Uint32 *)frmtbl);
    switch (mode) {
    case MWSFD_FTYPE_SFD:
        SFD_SetAdxtPara(&mwsfd_adxtpara);
        break;
    case MWSFD_FTYPE_MPV:
    case MWSFD_FTYPE_VONLYSFD:
        break;
    }
    sfd = SFD_Create(&crepara, 0);
    if (sfd == NULL) {
        MWSFLIB_SetErrCode(-0x131);
        MWSFSVM_Error("E20010703C mwPlyCreateSofdec: create error");
        return NULL;
    }
    if (SFD_SetErrFn(sfd, MWSFLIB_SfdErrFunc, mwply) != 0) {
        MWSFLIB_SetErrCode(-0x12F);
        MWSFSVM_Error("E20010703D mwPlyCreateSofdec: set errcb");
        return NULL;
    }
    mwply->fname = fname_p;
    mwply->fname_siz = MWSFD_FNAME_SIZE;
    picusr = &mwply->picusr;
    picusr->buf = picusr_p;
    picusr->bsize = MWSFD_PICUSR_SIZE;
    picusr->usize = MWSFD_PICUSR_UNIT;
    mwply->picusr_ptr = picusr;
    mwply->frmapi = 0;
    return sfd;
}

/* Takes the two reference frame buffers: user frame buffers 0 and 1 or two component allocations.
 * Returns 0, or -1 when a buffer is missing. */
Sint32 mwsfcre_MallocRfb(MWPLY mwply, MWSFD_CRPRM *cprm, void **rfbbuf) {
    Sint32 ret = 0;
    Sint32 fsize;

    fsize = mwsfcre_CalcYccSize(cprm->max_width, cprm->max_height);
    if (mwsfdcre_bufnum != 0) {
        /* the original tests the buffer size twice */
        if (mwsfdcre_bufnum < 2 || mwsfdcre_bufsize < fsize || mwsfdcre_bufsize < fsize) {
            rfbbuf[0] = NULL;
            rfbbuf[1] = NULL;
            ret = -1;
        } else {
            rfbbuf[0] = mwsfdcre_bufptr[0];
            rfbbuf[1] = mwsfdcre_bufptr[1];
        }
    } else {
        rfbbuf[0] = MWSFD_Malloc(mwply, fsize);
        rfbbuf[1] = MWSFD_Malloc(mwply, fsize);
    }
    if (rfbbuf[0] == NULL || rfbbuf[1] == NULL) {
        ret = -1;
    }
    return ret;
}

/* Tells the SFD handle where the stream joint of the play gets its data: the memory buffer of a
 * play from memory (kind 1), the ring buffer of the file stream joint (kind 0), or the supply
 * given with the joint. */
void MWSFCRE_SetSupplySj(MWPLY mwply) {
    SFBUF_SUP sup;
    SJ sj = mwply->sji;
    SFD sfd = mwply->sfd;

    if (sj == NULL) {
        return;
    }
    if (sj == mwply->mem_sj) {
        sup.kind = 1;
        sup.sj = sj;
        sup.ofst = (Uint32)mwply->mem_buf;
        sup.size = mwply->mem_size;
        sup.xsize = 0;
        sup.x14 = 0;
    } else if (sj == mwply->file_sj) {
        sup.kind = 0;
        sup.sj = sj;
        sup.ofst = mwply->x1c4;
        sup.size = mwply->flow_nsct;
        sup.xsize = mwply->x1cc;
        sup.x14 = 0;
    } else {
        sup.kind = mwply->sup_kind;
        sup.sj = sj;
        sup.ofst = mwply->sup_ofst;
        sup.size = mwply->sup_size;
        sup.xsize = mwply->sup_xsize;
        sup.x14 = 0;
    }
    if (SFD_SetSupplySj(sfd, &sup) != 0) {
        MWSFLIB_SetErrCode(-0x138);
        MWSFSVM_Error("E20010703B MWSFCRE_SetSupplySj: ");
    }
}

/* Work of the frame converter handle plus, for a layout that uses it, the additional-info ring. */
static inline Sint32 mwsfcre_CalcWorkSfx(MWSFD_CRPRM *cprm) {
    Sint32 size = MWSFSFX_CalcHnWorkSiz(cprm->max_width, cprm->max_height);

    if (MWSFTAG_IsUseAinfSj(cprm) == TRUE) {
        size += MWSFD_AINFSJ_BSIZE;
    }
    return size;
}

/* Total component work a player of the creation parameters needs: the SFD buffers plus the frame
 * converter. The caller allocates this much and passes it back as cprm->work. */
Sint32 mwPlyCalcWorkCprmSfd(MWSFD_CRPRM *cprm) {
    Sint32 sfdsiz;
    Sint32 sfxsiz;

    if (cprm == NULL) {
        MWSFSVM_Error("E1122613 mwPlyCalcWorkCprmSfd: cprm is NULL.");
        return 0;
    }
    sfdsiz = mwPlyCalcWorkSfd(cprm);
    sfxsiz = mwsfcre_CalcWorkSfx(cprm);
    return sfdsiz + sfxsiz;
}

/* Component work mwsfcre_CreateSfd takes. */
Sint32 mwPlyCalcWorkSfd(MWSFD_CRPRM *cprm) {
    Sint32 sjbsiz;
    Sint32 sibsiz;
    Sint32 vibsiz;
    Sint32 aibsiz;
    Sint32 adxibsiz;
    Sint32 adxwksiz;
    Sint32 size;
    Sint32 rfbsiz;
    Sint32 tabsiz;

    mwsfcre_CalcBufSize(cprm, &sibsiz, &vibsiz, &aibsiz, &sjbsiz, &adxibsiz, &adxwksiz);
    mwsfcre_CalcFrmBufSize(cprm, &rfbsiz, &tabsiz);
    /* The original adds the system input buffer size last, after the constant sizes. */
    size = vibsiz + aibsiz;
    size += 0x20;
    size += sjbsiz;
    size += 0x40;
    size += MWSFD_PICUSR_SIZE;
    size += rfbsiz;
    size += tabsiz;
    size += adxibsiz;
    size += adxwksiz;
    size += MWSFD_HNWORK_SIZE;
    size += 0x700;
    size += MWSFD_FNAME_SIZE;
    return sibsiz + size;
}
