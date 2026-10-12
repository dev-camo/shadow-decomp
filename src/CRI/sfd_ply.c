/* CRI Sofdec: player front end (sfd_ply.c). Handle creation / destruction, the state machine run by
 * SFD_ExecOne (stop -> prep -> standby -> play), the buffering pause and the reset that re-creates
 * a handle in place while keeping the user's settings. */
#include "CRI/sfd.h"

#define SFD_HN_MAX 8
/* stat is one of STOP..PLAYING */
#define SFD_STAT_IS_ACTIVE(stat)                                                                   \
    ((Uint32)((stat) - SFD_STAT_STOP) <= SFD_STAT_PLAYING - SFD_STAT_STOP)
#define SFD_HNWORK_MIN 0x3660
#define SFD_HNCTRL_WKSIZ_MAX 0x6CC0

/* player information block as sfply_InitHn clears it (SFD_OBJ + 0x950): 12 counters, 12 64-bit
 * counters, reserve */
typedef struct {
    Sint32 ndecpic;   /* 0x00 decoded pictures (SFPLY_AddDecPic) */
    Sint32 nskippic;  /* 0x04 skipped pictures (SFPLY_AddSkipPic) */
    Sint32 x08;       /* 0x08 */
    Sint32 x0c;       /* 0x0C */
    Sint32 x10;       /* 0x10 */
    Sint32 x14;       /* 0x14 */
    Sint32 getcnt;    /* 0x18 frames taken by SFD_GetFrm (this game only) */
    Sint32 relcnt;    /* 0x1C frames returned by SFD_RelFrm (RE4: a "frame held" flag) */
    Sint32 bpa;       /* 0x20 buffering pause active */
    Sint32 nbpa;      /* 0x24 buffering pauses so far */
    Sint32 x28;       /* 0x28 */
    Sint32 x2c;       /* 0x2C */
    Sint32 x30;       /* 0x30 */
    Sint64 cnt64[12]; /* 0x38 */
    Sint32 rsv[4];    /* 0x98 */
} SFPLY_PLYINF;

#define PLYINF(sfd) ((SFPLY_PLYINF *)&(sfd)->plyinf)

/* playing and not paused by the user or the buffering pause */
#define SFPLY_IS_PLAYING(sfd)                                                                      \
    ((sfd)->stat == SFD_STAT_PLAYING && (sfd)->pause_sw != 1 && PLYINF(sfd)->bpa != 1)

/* this game's Sofdec clock (sfd_ply time base; not RE4's UTY_GetTmr) */
extern Sint64 fn_803FEA5C(void);
/* this game only: clock units per second for fn_803FEA5C (inferred) */
extern Sint64 fn_803FE96C(void);
/* the timer is unusable (RE4 calls UTY_IsTmrVoid here; this game's version differs) */
extern Bool fn_80402510(void);

void (*SFPLY_recordgetfrm)(SFD sfd, void *frm);
Sint32 sfply_last_hnctrl_wksiz;
void (*SFPLY_SetPtsInfo)(SFPLY_PTSM *ptsm);
void (*SFPLY_ResetPtsm)(SFPLY_PTSM *ptsm);

/* default conditions (100) + the guard word checked by SFPLY_Init */
const Sint32 SFPLY_cond_dfl[101] = {
    1,      1,      1,        1,     1,      1,      1,          0,  0, 0, 0,  0,     1,
    1,      0,      1,        0,     1,      -3,     1,          -4, 1, 0, 3,  4096,  0,
    1,      60,     1,        -1,    -1,     0,      0,          0,  0, 0, 0,  0,     1,
    0,      -30000, -14000,   8000,  59940,  4000,   4000,       41, 0, 0, 0,  0,     5,
    0,      5,      35820000, 0,     0,      0,      0x7FFFFFFF, 0,  0, 1, 10, 16683, 200000,
    -16683, -16683, 1,        66732, 133464, 512000, 1,          1,  0, 0, 0,  0,     0,
    0,      1,      1,        -1,    -1,     0,      0,          0,  0, 0, 0,  0,     0,
    0,      0,      0,        0,     0,      0,      0x5A5A5A5A, 0,  0, 0,
};

/* Tells the input driver where its data comes from: a memory range or a stream joint (the player's
 * file ring fed by ADXSTM). MWSFCRE_SetSupplySj calls it at every start. */
/* This game only: returns the pause switch (inferred) */
Sint32 fn_803F94D8(SFD sfd) {
    if (SFLIB_CheckHn(sfd) != 0) {
        SFLIB_SetErr(0, 0xFF000141);
        return 0;
    }
    return sfd->pause_sw;
}

Sint32 SFD_SetSupplySj(SFD sfd, SFBUF_SUP *sup) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000139);
    }
    return SFBUF_SetSupplySj(sfd, sup);
}

/* dead (inlined into sfply_ResetHn): writable region of the input buffer */
Sint32 SFD_GetSupplyInf(SFD sfd, SFBUF_RINF *inf) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000134);
    }
    return SFTRN_CallTrtTrif(sfd, 0, 9, (Sint32)inf, 0);
}

/* dead (inlined into sfply_ResetHn) */
Sint32 SFD_AddSupply(SFD sfd, Sint32 adr, Sint32 nbyte) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000135);
    }
    return SFTRN_CallTrtTrif(sfd, 0, 10, adr, nbyte);
}

/* This game only: the first frame access claims sfd->x58 (0 -> 1); any other value is error
 * 0xFF000207 (inferred) */
static inline Sint32 sfply_LockFrmAccess(SFD sfd) {
    if (sfd->x58 == 0) {
        sfd->x58 = 1;
        return 0;
    }
    if (sfd->x58 == 1) {
        return 0;
    }
    return SFLIB_SetErr(sfd, 0xFF000207);
}

/* Returns a decoded frame to the video output driver (tr 6, AddRead) and counts the release while
 * a frame is out (relcnt catches up with getcnt); mwPlyRelCurFrm. */
Sint32 SFD_RelFrm(SFD sfd, void *frm) {
    Sint32 ret;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000137);
    }
    ret = sfply_LockFrmAccess(sfd);
    if (ret != 0) {
        return ret;
    }
    if (PLYINF(sfd)->relcnt < PLYINF(sfd)->getcnt) {
        PLYINF(sfd)->relcnt++;
    }
    return SFTRN_CallTrtTrif(sfd, 6, 12, (Sint32)frm, 0);
}

/* Asks the video output driver (tr 6, GetRead) for the next frame whose display time has come;
 * *frm = 0 when none. Counts the frame when none is out (recording the time of the first) and calls
 * the optional record hook. */
Sint32 SFD_GetFrm(SFD sfd, void **frm) {
    Sint32 ret;

    *frm = 0;
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000136);
    }
    ret = sfply_LockFrmAccess(sfd);
    if (ret != 0) {
        return ret;
    }
    ret = SFTRN_CallTrtTrif(sfd, 6, 11, (Sint32)frm, 0);
    if (*frm != 0 && PLYINF(sfd)->getcnt == PLYINF(sfd)->relcnt) {
        if (PLYINF(sfd)->getcnt == 0) {
            sfd->frmstat.t0 = fn_803FEA5C();
        }
        PLYINF(sfd)->getcnt++;
    }
    if (SFPLY_recordgetfrm != 0) {
        SFPLY_recordgetfrm(sfd, *frm);
    }
    return ret;
}

/* dead (placed after SFD_GetFrm: .bss is in first-reference order, SFPLY_recordgetfrm comes
 * first): the handle control work size the last handle was created with */
Sint32 SFPLY_GetLastHnCtrlWkSiz(void) {
    return sfply_last_hnctrl_wksiz;
}

/* dead: PTS manager hooks */
void SFPLY_SetPtsmFn(void (*setfn)(SFPLY_PTSM *ptsm), void (*resetfn)(SFPLY_PTSM *ptsm)) {
    SFPLY_SetPtsInfo = setfn;
    SFPLY_ResetPtsm = resetfn;
}

/* Declares the input finished: sets the terminate flag on the input driver's output buffer so the
 * demuxer/decoders drain and the play ends (linked play, stream-joint starts). */
Sint32 SFD_TermSupply(SFD sfd) {
    Sint32 buf;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000135);
    }
    buf = sfd->tr[0].bufout;
    if (SFBUF_GetTermFlg(sfd, buf) == 1) {
        return 0;
    }
    SFBUF_SetTermFlg(sfd, buf, 1);
    sfd->chg_flg = 1;
    return 0;
}

static SFD sfply_InitHn(SFD_CREPRM *prm, Sint32 x);

/* re-create the handle in place, keeping the user settings (callbacks, speed, seek work, PTS
 * work) */
Sint32 sfply_ResetHn(SFD sfd) {
    Sint32 nmpv;
    SFD_CREPRM prm;
    Sint32 cond[SFD_COND_NUM];
    Uint8 mpvcond[0x40];
    SFBUF_RINF inf;
    SFPLY_PTSM ptsm;
    Sint32 keepsup;
    Sint32 supadr = 0;
    void (*errfn)(void *obj, Sint32 code);
    void *errobj;
    SFTIM_FN usrtimefn;
    SFTIM_FN extfn;
    Sint32 extwrap;
    void *extobj;
    Bool (*isskipfn)();
    Sint32 speed;
    SFSEE_WORK *see;
    Sint32 byterate;
    Sint32 fsize;
    Sint32 tottime;
    Sint32 tunit;
    Sint32 seekpos;
    SFPTS_ENT *ptsent;
    Sint32 ptssiz;
    Sint32 ret;

    prm = sfd->prm;
    keepsup = sfd->cond[8];
    if (keepsup) {
        SFD_GetSupplyInf(sfd, &inf);
        supadr = inf.rsv[1];
    }
    SFHDS_FinishFhd(&sfd->fhd);
    SFBUF_DestroySj(sfd);
    errobj = sfd->err.obj;
    errfn = sfd->err.fn;
    usrtimefn = SFD_TIM(sfd)->timefn[4];
    extfn = SFD_TIM(sfd)->extfn;
    extwrap = SFD_TIM(sfd)->ext_wrap;
    extobj = SFD_TIM(sfd)->ext_obj;
    isskipfn = SFD_TIM(sfd)->isskipfn;
    speed = SFD_TIM(sfd)->speed;
    ptsm = sfd->ptsm;
    see = sfd->see.wk;
    if (see != 0) {
        byterate = see->byterate;
        fsize = see->fsize;
        tottime = see->tottime;
        tunit = see->tunit;
        seekpos = see->seekpos;
    } else {
        seekpos = 0;
        tunit = 0;
        tottime = 0;
        fsize = 0;
        byterate = 0;
    }
    ptssiz = sfd->buf[1].u.ring.ptsque.num * sizeof(SFPTS_ENT);
    ptsent = sfd->buf[1].u.ring.ptsque.ent;
    nmpv = SFMPV_SaveCond(sfd, mpvcond, sizeof(mpvcond));
    sfd->stat = SFD_STAT_NONE;
    sfd->req = SFD_REQ_NONE;
    ret = SFTRN_CallTrSetup(sfd, 4);
    if (ret != 0) {
        return ret;
    }
    MEM_Copy(cond, sfd->cond_def, sizeof(cond));
    sfd = sfply_InitHn(&prm, 0);
    if (sfd == 0) {
        return SFLIB_SetErr(0, 0xFF000202);
    }
    MEM_Copy(sfd->cond, cond, sizeof(cond));
    MEM_Copy(sfd->cond_def, cond, sizeof(cond));
    SFMPV_RestoreCond(sfd, mpvcond, nmpv);
    if (keepsup) {
        ret = SFD_GetSupplyInf(sfd, &inf);
        if (ret != 0) {
            return ret;
        }
        ret = SFD_AddSupply(sfd, supadr, inf.rsv[1]);
        if (ret != 0) {
            return ret;
        }
        SFD_TermSupply(sfd);
    }
    if (errfn != 0) {
        SFD_SetErrFn(sfd, errfn, errobj);
    }
    if (usrtimefn != 0) {
        SFD_SetUsrTimeFn(sfd, usrtimefn);
    }
    if (extfn != 0) {
        SFD_SetExtClockFn(sfd, extfn, extwrap, extobj);
    }
    if (isskipfn != 0) {
        SFD_SetUsrIsSkipFn(sfd, isskipfn);
    }
    if (speed != 1000) {
        SFD_SetSpeed(sfd, speed);
    }
    if (ptsm.x00 != 0) {
        sfd->ptsm = ptsm;
        if (SFPLY_ResetPtsm != 0) {
            SFPLY_ResetPtsm(&ptsm);
        }
    }
    if (see != 0) {
        SFD_EntrySeek(sfd, see);
        SFD_SetByteRate(sfd, byterate);
        SFD_SetFileSize(sfd, fsize);
        SFD_SetTotTime(sfd, tottime, tunit);
        SFD_SetSeekPos(sfd, seekpos);
    }
    if (ptsent != 0) {
        SFD_SetVideoPts(sfd, ptsent, ptssiz);
    }
    return 0;
}

/* 1 while sfply_StopHn is re-creating a handle (the drivers' Destroy/Create see it). */
Sint32 SFPLY_GetResetFlg(void) {
    return SFLIB_libwork.x1fc;
}

/* stop the output drivers of a playing handle */
static Sint32 sfply_StopTr(SFD sfd) {
    Sint32 ret;

    if (sfd->stat == SFD_STAT_PLAYING) {
        ret = SFTRN_CallTrtTrif(sfd, 7, 7, 0, 0);
        if (ret != 0) {
            return ret;
        }
    }
    sfd->stat = SFD_STAT_STOP;
    sfd->req = SFD_REQ_STOP;
    return 0;
}

/* stop and reset the handle */
static Sint32 sfply_StopHn(SFD sfd) {
    Sint32 ret;

    if (sfd->stat == SFD_STAT_STOP) {
        return 0;
    }
    ret = sfply_StopTr(sfd);
    if (ret != 0) {
        return ret;
    }
    sfd->req = SFD_REQ_NONE;
    sfd->stat = SFD_STAT_NONE;
    SFLIB_libwork.x1fc = 1;
    ret = sfply_ResetHn(sfd);
    SFLIB_libwork.x1fc = 0;
    if (ret != 0) {
        return ret;
    }
    return 0;
}

/* Stops the handle: stops the output drivers, then re-creates the handle in place with its settings
 * kept (sfply_ResetHn) so it is ready for the next start. Requests a state update. */
Sint32 SFD_Stop(SFD sfd) {
    Sint32 ret;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000133);
    }
    ret = sfply_StopHn(sfd);
    sfd->chg_flg = 1;
    return ret;
}

/* Requests playback (req START): the state machine goes PREP -> STBY -> PLAYING as the streams are
 * prepared. With cond 0x2F the Pro Logic II standby path is used instead. */
Sint32 SFD_Start(SFD sfd) {
    Sint32 ret;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000132);
    }
    if (SFSET_GetCond(sfd, 0x2F) == 1) {
        ret = SFPL2_Standby(sfd);
    } else {
        sfd->req = SFD_REQ_START;
        ret = 0;
    }
    sfd->chg_flg = 1;
    return ret;
}

/* Returns SFTRN_CallTrSetup's result (mwsfdcre checks it). Keeping that result live across the
 * table clear also gives the loop its original registers. */
Sint32 SFD_Destroy(SFD sfd) {
    SFD *tbl;
    Sint32 i;
    Sint32 ret;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000131);
    }
    sfply_StopHn(sfd);
    SFHDS_FinishFhd(&sfd->fhd);
    SFBUF_DestroySj(sfd);
    sfd->stat = SFD_STAT_NONE;
    sfd->req = SFD_REQ_NONE;
    ret = SFTRN_CallTrSetup(sfd, 4);
    tbl = SFLIB_libwork.hn;
    for (i = 0; i < SFD_HN_MAX; i++, tbl++) {
        if (*tbl == sfd) {
            *tbl = 0;
        }
    }
    return ret;
}

/* Counts `n` skipped pictures in the player information and calls the optional hook (cond 0x25). */
void SFPLY_AddSkipPic(SFD sfd, Sint32 n, void *arg) {
    void (*fn)(SFD sfd, void *arg, SFD_PLYINF *inf);
    SFD_PLYINF *inf = &sfd->plyinf;

    PLYINF(sfd)->nskippic += n;
    fn = (void (*)(SFD, void *, SFD_PLYINF *))SFSET_GetCond(sfd, 0x25);
    if (fn != 0) {
        fn(sfd, arg, inf);
    }
}

/* Counts `n` decoded pictures and calls the optional hook (cond 0x24). */
void SFPLY_AddDecPic(SFD sfd, Sint32 n, void *arg) {
    void (*fn)(SFD sfd, void *arg, SFD_PLYINF *inf);
    SFD_PLYINF *inf = &sfd->plyinf;

    PLYINF(sfd)->ndecpic += n;
    fn = (void (*)(SFD, void *, SFD_PLYINF *))SFSET_GetCond(sfd, 0x24);
    if (fn != 0) {
        fn(sfd, arg, inf);
    }
}

/* Builds a handle in the caller's 32-byte-aligned work: copies the creation parameters, resets the
 * header analysis, stream info, player information, timers and error info, copies the library
 * default conditions, initialises the timing work, the 8 buffers (SFBUF_InitHn), the 9
 * transfer-driver slots and the seek work, then runs every driver's Create (fn 3). State STOP. 0 on
 * a bad work size (all handles must use the same size). */
static SFD sfply_InitHn(SFD_CREPRM *prm, Sint32 x) {
    SFD hn;
    Sint32 i;
    Sint32 wksiz;
    Uint32 ndw;
    SFTMR_TSUM *ts;
    void *work;

    work = prm->hnwork;
    wksiz = prm->hnwksiz;
    ndw = (Uint32)wksiz >> 2;
    if (work == 0) {
        return 0;
    }
    if (wksiz <= 0 || (Uint32)wksiz > SFD_HNCTRL_WKSIZ_MAX) {
        return 0;
    }
    if (sfply_last_hnctrl_wksiz != 0 && sfply_last_hnctrl_wksiz != wksiz) {
        return 0;
    }
    sfply_last_hnctrl_wksiz = wksiz;
    UTY_MemsetDword(work, 0, ndw);
    hn = (SFD)(((Uint32)work + 31) & ~31);
    hn->stat = hn->req = SFD_STAT_NONE; /* SFD_REQ_NONE is also 0 */
    prm->adr = (prm->adr + 31) & ~31;
    hn->prm = *prm;
    hn->chg_flg = 1;
    hn->pause_sw = 0;
    hn->pause_cnt = 0;
    hn->x58 = 0;
    hn->x5c = 0;
    SFHDS_InitFhd(&hn->fhd);
    UTY_MemsetDword(&hn->x90c, 0, 0x10);
    hn->x90c = 0;
    hn->x910 = 0;
    hn->x914 = 0;
    hn->x918 = 0;
    hn->x91c = 0;
    hn->picrate = 0;
    hn->x924 = 0;
    hn->x928 = 1;
    hn->x92c = 0;
    hn->numelem_aud = -1;
    hn->numelem_vid = -1;
    hn->numelem_prv = -1;
    UTY_MemsetDword(&hn->plyinf, 0, sizeof(SFD_PLYINF) / 4);
    PLYINF(hn)->ndecpic = 0;
    PLYINF(hn)->nskippic = 0;
    PLYINF(hn)->x08 = 0;
    PLYINF(hn)->x0c = 0;
    PLYINF(hn)->x10 = 0;
    PLYINF(hn)->getcnt = 0;
    PLYINF(hn)->relcnt = 0;
    PLYINF(hn)->bpa = 0;
    PLYINF(hn)->nbpa = 0;
    PLYINF(hn)->x28 = 0;
    PLYINF(hn)->x2c = 0;
    PLYINF(hn)->x30 = 0;
    PLYINF(hn)->cnt64[0] = 0;
    PLYINF(hn)->cnt64[1] = 0;
    PLYINF(hn)->cnt64[2] = 0;
    PLYINF(hn)->cnt64[3] = 0;
    PLYINF(hn)->cnt64[4] = 0;
    PLYINF(hn)->cnt64[5] = 0;
    PLYINF(hn)->cnt64[6] = 0;
    PLYINF(hn)->cnt64[7] = 0;
    PLYINF(hn)->cnt64[8] = 0;
    PLYINF(hn)->cnt64[9] = 0;
    PLYINF(hn)->cnt64[10] = 0;
    PLYINF(hn)->cnt64[11] = 0;
    /* clears tsum[] and the frame statistics behind it in one call */
    UTY_MemsetDword(hn->tsum, 0, (sizeof(hn->tsum) + sizeof(hn->frmstat)) / 4);
    for (i = 0, ts = hn->tsum; i < 5; i++, ts++) {
        SFTMR_InitTsum(ts);
    }
    SFTMR_InitTsum(&hn->tsum[5]);
    hn->frmstat.t0 = 0;
    hn->frmstat.t1 = 0;
    hn->frmstat.tunit = 0;
    hn->frmstat.nfrm = 0;
    hn->frmstat.rate = 0.0f;
    SFLIB_InitErrInf(&hn->err);
    MEM_Copy(hn->cond, SFLIB_libwork.cond, sizeof(hn->cond));
    MEM_Copy(hn->cond_def, SFLIB_libwork.cond, sizeof(hn->cond_def));
    SFTIM_InitHn(hn, SFD_TIM(hn));
    if (SFBUF_InitHn(hn, hn->buf, (SFBUF_PRM *)prm) != 0) {
        return 0;
    }
    SFTRN_InitHn(hn, hn->tr, (SFTRN_PRM *)prm, x);
    SFSEE_InitHn(&hn->see);
    if (SFTRN_CallTrSetup(hn, 3) != 0) {
        return 0;
    }
    hn->req = SFD_REQ_STOP;
    hn->stat = SFD_STAT_STOP;
    return hn;
}

/* First free slot of the 8 handle pointers, -1 when full. */
static Sint32 sfply_SearchFreeHn(SFD *tbl) {
    Sint32 i;

    for (i = 0; i < SFD_HN_MAX; i++) {
        if (tbl[i] == 0) {
            return i;
        }
    }
    return -1;
}

/* Public creation: validates the buffer base and handle work size (>= 0x3660), takes a slot and
 * builds the handle (sfply_InitHn). The MW player calls it from mwsfcre_CreateSfd. */
SFD SFD_Create(SFD_CREPRM *prm, Sint32 x) {
    Sint32 id;
    SFD *tbl = SFLIB_libwork.hn;
    Sint32 ret;

    if (prm->adr == 0) {
        ret = SFLIB_SetErr(0, 0xFF000204);
    } else if ((Uint32)prm->hnwksiz < SFD_HNWORK_MIN) {
        ret = SFLIB_SetErr(0, 0xFF000205);
    } else {
        ret = 0;
    }
    if (ret != 0) {
        return 0;
    }
    id = sfply_SearchFreeHn(tbl);
    if (id == -1) {
        SFLIB_SetErr(0, 0xFF000206);
        return 0;
    }
    return tbl[id] = sfply_InitHn(prm, x);
}

/* a stream or buffer reached its end */
static Bool sfply_IsTermAny(SFD sfd) {
    Sint32 i;

    if (SFSET_GetCond(sfd, 5) && SFTRN_GetTermFlg(sfd, 6)) {
        return 1;
    }
    if (SFSET_GetCond(sfd, 6) && SFTRN_GetTermFlg(sfd, 7)) {
        return 1;
    }
    for (i = 0; i < SFD_BUF_NUM; i++) {
        if (SFBUF_GetTermFlg(sfd, i)) {
            return 1;
        }
    }
    return 0;
}

/* the video input ring holds 80% of its size or the configured amount */
static Bool sfply_IsVidBufFull(SFD sfd) {
    SFBUF_RING *ring = &sfd->buf[sfd->tr[2].bufin].u.ring;
    Sint32 ndata;

    ndata = SJ_GetNumData(ring->sup.sj, 1);
    if (ndata >= ring->sup.size * 80 / 100 || ndata >= SFSET_GetCond(sfd, 0x46)) {
        return 1;
    }
    return 0;
}

/* The audio input ring holds >= 80% of its size. */
static Bool sfply_IsAudBufFull(SFD sfd) {
    SFBUF_RING *ring = &sfd->buf[sfd->tr[3].bufin].u.ring;
    Sint32 ndata;

    ndata = SJ_GetNumData(ring->sup.sj, 1);
    if (ndata >= ring->sup.size * 80 / 100) {
        return 1;
    }
    return 0;
}

/* start a buffering pause? */
static Bool sfply_IsBpaOn(SFD sfd) {
    Sint32 t;
    Sint32 u;
    Sint32 a;
    Sint32 b;
    Sint32 i;
    Bool term;

    if (SFSET_GetCond(sfd, 0x43) == 0) {
        return 0;
    }
    if (SFSET_GetCond(sfd, 0xF) == 0) {
        return 0;
    }
    if (sfd->pause_sw != 0) {
        return 0;
    }
    if (sfd->stat != SFD_STAT_PLAYING) {
        return 0;
    }
    /* sfply_IsTermAny written out: as a helper call the function would be small enough to be
     * inlined into sfply_StatPlay */
    if (SFSET_GetCond(sfd, 5) && SFTRN_GetTermFlg(sfd, 6)) {
        term = 1;
    } else if (SFSET_GetCond(sfd, 6) && SFTRN_GetTermFlg(sfd, 7)) {
        term = 1;
    } else {
        for (i = 0; i < SFD_BUF_NUM; i++) {
            if (SFBUF_GetTermFlg(sfd, i)) {
                term = 1;
                goto term_end;
            }
        }
        term = 0;
    }
term_end:
    if (term) {
        return 0;
    }
    if (SFSET_GetCond(sfd, 5) == 1 && PLYINF(sfd)->x28 == 0) {
        return 0;
    }
    if (SFSET_GetCond(sfd, 6) == 1 && SFBUF_RingGetDataSiz(sfd, 2) > 0) {
        return 0;
    }
    if (SFTRN_IsSetup(sfd, 1) && SFBUF_RingGetDataSiz(sfd, 0) > 0) {
        return 0;
    }
    if (SFSET_GetCond(sfd, 5) == 1) {
        if (sfply_IsVidBufFull(sfd)) {
            return 0;
        }
    }
    SFTIM_GetTime(sfd, &t, &u);
    a = SFD_TIM(sfd)->x284;
    b = SFD_TIM(sfd)->x288;
    a -= UTY_MulDiv(SFSET_GetCond(sfd, 0x44), b, 1000000);
    if (t <= 0 || a <= 0) {
        return 0;
    }
    return SFD_CmpTime(t, u, a, b) == 0;
}

/* end a buffering pause? */
static Bool sfply_IsBpaOff(SFD sfd) {
    Sint32 t;
    Sint32 u;
    Sint32 a;
    Sint32 b;

    if (sfply_IsTermAny(sfd)) {
        return 1;
    }
    if (SFSET_GetCond(sfd, 5) == 1) {
        if (sfply_IsVidBufFull(sfd)) {
            return 1;
        }
    }
    if (SFSET_GetCond(sfd, 6) == 1) {
        if (sfply_IsAudBufFull(sfd)) {
            return 1;
        }
    }
    SFTIM_GetTime(sfd, &t, &u);
    a = SFD_TIM(sfd)->x284;
    b = SFD_TIM(sfd)->x288;
    a -= UTY_MulDiv(SFSET_GetCond(sfd, 0x45), b, 1000000);
    if (SFD_CmpTime(t, u, a, b)) {
        return 1;
    }
    return 0;
}

/* the user's end time was reached */
static Bool sfply_IsEndTime(SFD sfd) {
    Sint32 t;
    Sint32 u;
    Sint32 ec;
    Sint32 eu;

    ec = sfd->cond[20];
    eu = sfd->cond[21];
    if (ec == -4) {
        return 0;
    }
    SFTIM_GetTime(sfd, &t, &u);
    if (t < 0) {
        return 0;
    }
    if (UTY_CmpTime(t, u, ec, eu) != 0) {
        return 0;
    }
    return 1;
}

/* all the streams selected by the termination mode (cond 0x19) ended */
static Bool sfply_IsTermAll(SFD sfd) {
    Sint32 vterm;
    Sint32 aterm;
    Bool term;

    if (sfd->cond[6] == 0 && sfd->cond[5] == 0) {
        return 1;
    }
    term = 0;
    vterm = SFTRN_GetTermFlg(sfd, 6);
    aterm = SFTRN_GetTermFlg(sfd, 7);
    switch (SFSET_GetCond(sfd, 0x19)) {
    case 1:
        term = aterm;
        break;
    case 2:
        term = vterm;
        break;
    case 3:
        term = aterm | vterm;
        break;
    case 0:
        term = aterm & vterm;
        break;
    }
    return term;
}

/* Playing and the clock has not advanced for too long (SFTIM_IsStagnant) -> the play is ended. */
static Bool sfply_IsStagnant(SFD sfd) {
    if (!SFPLY_IS_PLAYING(sfd)) {
        return 0;
    }
    if (SFTIM_IsStagnant(sfd)) {
        return 1;
    }
    return 0;
}

/* the clock ran past the configured stop time */
static Bool sfply_IsOverTime(SFD sfd) {
    Sint32 t;
    Sint32 u;

    if (!SFPLY_IS_PLAYING(sfd)) {
        return 0;
    }
    if (SFTIM_GetTimeSub(sfd, &t, &u) != 0) {
        return 0;
    }
    if (t < 0) {
        return 0;
    }
    if (SFD_CmpTime(SFSET_GetCond(sfd, 0x36), 1000, t, u)) {
        return 1;
    }
    return 0;
}

/* the standby state may start playing */
static Bool sfply_IsStartable(SFD sfd) {
    if (sfd->cond[14] == 0) {
        return 1;
    }
    if (sfd->cond[5] == 0) {
        return 1;
    }
    if (SFD_TIM(sfd)->x2b0 != 0) {
        return 1;
    }
    if (SFD_TIM(sfd)->vcnt >= sfd->cond[45]) {
        return 1;
    }
    if (sfply_IsTermAll(sfd)) {
        return 1;
    }
    return 0;
}

/* playback ended: stop the outputs and request the end state */
static Sint32 sfply_StopPlay(SFD sfd) {
    Sint32 ret;

    ret = sfply_StopTr(sfd);
    if (ret != 0) {
        return ret;
    }
    sfd->req = SFD_REQ_END;
    sfd->frmstat.t1 = fn_803FEA5C();
    sfd->frmstat.tunit = fn_803FE96C();
    sfd->frmstat.nfrm = PLYINF(sfd)->getcnt;
    if (sfd->frmstat.t1 - sfd->frmstat.t0 != 0) {
        sfd->frmstat.rate = (Float32)(sfd->frmstat.nfrm * sfd->frmstat.tunit) /
                            (Float32)(sfd->frmstat.t1 - sfd->frmstat.t0);
    }
    return 0;
}

/* PLAYING state step: ends the play at the user end time, when all selected streams terminated, on
 * a stagnant clock or past the stop time; toggles the buffering pause (bpa) when the input runs dry
 * /refills (pausing audio and clock through SFPL2_Pause); req END -> PLAYEND. */
Sint32 sfply_StatPlay(SFD sfd) {
    Sint32 cs;
    Sint32 ret;
    Sint32 stat;

    if (sfply_IsEndTime(sfd) || sfply_IsTermAll(sfd) || sfply_IsStagnant(sfd) ||
        sfply_IsOverTime(sfd)) {
        ret = sfply_StopPlay(sfd);
    } else {
        ret = 0;
    }
    if (ret != 0) {
        return sfd->stat;
    }
    SFLIB_LockCs(&cs);
    ret = 0;
    if (PLYINF(sfd)->bpa == 0) {
        if (sfply_IsBpaOn(sfd)) {
            PLYINF(sfd)->bpa = 1;
            PLYINF(sfd)->nbpa++;
            ret = SFPL2_Pause(sfd, 1);
        }
    } else if (sfply_IsBpaOff(sfd)) {
        PLYINF(sfd)->bpa = 0;
        ret = SFPL2_Pause(sfd, 0);
    }
    SFLIB_UnlockCs(&cs);
    if (ret != 0) {
        return sfd->stat;
    }
    stat = sfd->stat;
    switch (sfd->req) {
    case SFD_REQ_START:
        break;
    case SFD_REQ_END:
        stat = SFD_STAT_PLAYEND;
        break;
    }
    return stat;
}

/* both selected streams are prepared (or ended) */
static Bool sfply_IsPrepared(SFD sfd) {
    Bool vprep;
    Bool aprep;
    Bool prep;
    Bool term;

    if (SFSET_GetCond(sfd, 5) == 0) {
        vprep = 1;
    } else {
        prep = SFTRN_GetPrepFlg(sfd, 6);
        term = SFTRN_GetTermFlg(sfd, 6);
        vprep = prep | term;
    }
    if (SFSET_GetCond(sfd, 6) == 0) {
        aprep = 1;
    } else {
        prep = SFTRN_GetPrepFlg(sfd, 7);
        term = SFTRN_GetTermFlg(sfd, 7);
        aprep = prep | term;
    }
    if (vprep == 0 || aprep == 0) {
        return 0;
    }
    return 1;
}

/* PREP state step: once both selected streams are prepared, drops a stream that produced no data at
 * all (cond 5 video / 6 audio), fixes the clock source (cond 15: 1 video, 2 audio) and the
 * termination mode (cond 0x19) from what is present, then STBY or, if startable, starts the output
 * drivers (fn 6) and goes PLAYING. */
static Sint32 sfply_StatPrep(SFD sfd) {
    Sint32 req;
    Sint32 stat;
    Sint32 mode;
    Sint32 sel;

    stat = sfd->stat;
    req = sfd->req;
    if (!sfply_IsPrepared(sfd)) {
        return stat;
    }
    if (sfd->cond[5] == 1 && SFBUF_GetWTot(sfd, 1) == 0 && SFBUF_GetRTot(sfd, 1) == 0) {
        sfd->cond[5] = 0;
    }
    if (sfd->cond[6] == 1 && SFBUF_GetWTot(sfd, 2) == 0 && SFBUF_GetRTot(sfd, 2) == 0) {
        sfd->cond[6] = 0;
    }
    SFSEE_FixAvPlay(sfd, sfd->cond[5], sfd->cond[6]);
    if (sfd->cond[6] == 0 && sfd->cond[15] == 2) {
        SFSET_SetCond(sfd, 15, 1);
    }
    if (sfd->cond[5] == 0 && sfd->cond[15] == 1) {
        SFSET_SetCond(sfd, 15, 2);
    }
    sel = 0;
    if (sfd->cond[6] == 1) {
        sel |= 1;
    }
    if (sfd->cond[5] == 1) {
        sel |= 2;
    }
    switch (sel) {
    case 1:
        mode = 1;
        break;
    case 2:
        mode = 2;
        break;
    case 3:
        mode = SFSET_GetCond(sfd, 0x19);
        if (mode == 0) {
            if (fn_80402510() || SFSET_GetCond(sfd, 0x48) == 0) {
                mode = 3;
            }
        }
        break;
    default:
        mode = 3;
        break;
    }
    SFSET_SetCond(sfd, 0x19, mode);
    switch (req) {
    case SFD_REQ_PREP:
        stat = SFD_STAT_PREP;
        break;
    case SFD_REQ_STBY:
        stat = SFD_STAT_STBY;
        break;
    case SFD_REQ_START:
    case SFD_REQ_STARTEND:
        if (sfply_IsStartable(sfd)) {
            SFTRN_CallTrtTrif(sfd, 7, 6, 0, 0);
            stat = SFD_STAT_PLAYING;
        } else {
            stat = SFD_STAT_STBY;
        }
        break;
    }
    return stat;
}

/* standby: wait for the start request and the start condition */
static Sint32 sfply_StatStby(SFD sfd, Sint32 stat) {
    switch (sfd->req) {
    case SFD_REQ_PREP:
        stat = SFD_STAT_PREP;
        break;
    case SFD_REQ_STBY:
        stat = SFD_STAT_STBY;
        break;
    case SFD_REQ_START:
    case SFD_REQ_STARTEND:
        if (sfply_IsStartable(sfd)) {
            SFTRN_CallTrtTrif(sfd, 7, 6, 0, 0);
            stat = SFD_STAT_PLAYING;
        }
        break;
    }
    return stat;
}

/* One server step of a handle (only when a control change or driver flagged chg_flg): runs every
 * driver's ExecServer (fn 2) and the seek server in PREP/STBY/PLAYING, then the state transition
 * (STOP -> PREP on any request, PREP, STBY, PLAYING steps); times the step in tsum[5]. */
void sfply_ExecOne(SFD sfd) {
    Sint64 t0;
    Sint64 t1;
    Sint32 stat;

    stat = sfd->stat;
    if (!SFD_STAT_IS_ACTIVE(stat)) {
        return;
    }
    if (sfd->chg_flg == 0) {
        return;
    }
    sfd->chg_flg = 0;
    t0 = fn_803FEA5C();
    if (stat == SFD_STAT_PREP || stat == SFD_STAT_STBY || stat == SFD_STAT_PLAYING) {
        SFTRN_CallTrSetup(sfd, 2);
        SFSEE_ExecServer(sfd);
    }
    /* the three idle states re-read the state: MWCC only builds the jump table for 7 separate
     * case bodies, and the re-read folds into the loaded value */
    stat = sfd->stat;
    switch (stat) {
    case SFD_STAT_STOP:
        switch (sfd->req) {
        case SFD_REQ_PREP:
        case SFD_REQ_STBY:
        case SFD_REQ_START:
        case SFD_REQ_STARTEND:
            stat = SFD_STAT_PREP;
            break;
        }
        break;
    case SFD_STAT_PREP:
        stat = sfply_StatPrep(sfd);
        break;
    case SFD_STAT_STBY:
        stat = sfply_StatStby(sfd, stat);
        break;
    case SFD_STAT_PLAYING:
        stat = sfply_StatPlay(sfd);
        break;
    case 5:
        stat = sfd->stat;
        break;
    case SFD_STAT_PLAYEND:
        stat = sfd->stat;
        break;
    case SFD_STAT_NONE:
        stat = sfd->stat;
        break;
    }
    sfd->stat = stat;
    t1 = fn_803FEA5C();
    SFTMR_AddTsum(&sfd->tsum[5], t1 - t0);
}

/* Public server entry: validates the handle and runs sfply_ExecOne. Called per handle from the MW
 * player's decode server each frame. */
Sint32 SFD_ExecOne(SFD sfd) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000138);
    }
    sfply_ExecOne(sfd);
    return 0;
}

/* the handle has nothing to do until the next control change */
static Bool sfply_IsHnSvrWait(SFD sfd) {
    Sint32 stat = sfd->stat;

    if (!SFD_STAT_IS_ACTIVE(stat)) {
        return 1;
    }
    if (sfd->chg_flg != 0) {
        return 0;
    }
    return 1;
}

/* 1 when no handle has pending work (drives the MW player's idle hook). */
Bool SFD_IsSvrWait(void) {
    SFD *tbl = SFLIB_libwork.hn;
    Sint32 i;
    SFD sfd;

    for (i = 0; i < SFD_HN_MAX; i++, tbl++) {
        sfd = *tbl;
        if (SFLIB_CheckHn(sfd) != 0) {
            continue;
        }
        if (!sfply_IsHnSvrWait(sfd)) {
            return 0;
        }
    }
    return 1;
}

/* 1 when this handle has nothing to do (inactive state or no change flagged). */
Bool SFD_IsHnSvrWait(SFD sfd) {
    /* matching constraint: the inactive test is written out rather than as !SFD_STAT_IS_ACTIVE,
     * which orders the branch and the `return 1` differently */
    if ((Uint32)(sfd->stat - SFD_STAT_STOP) > SFD_STAT_PLAYING - SFD_STAT_STOP) {
        return 1;
    }
    return sfd->chg_flg == 0;
}

/* Vsync tick for the SFD clock (SFTIM_VbIn); the MW player calls it once per ADXM_ExecMain. */
void SFD_VbIn(void) {
    SFTIM_VbIn();
}

/* Library init check: the default condition table's guard word (0x5A5A5A5A at index 96) must be
 * intact; clears the get-frame record hook. */
void SFPLY_Init(void) {
    if (SFPLY_cond_dfl[96] != 0x5A5A5A5A) {
        SFLIB_SetErr(0, 0xFF000201);
    }
    SFPLY_recordgetfrm = 0;
}
