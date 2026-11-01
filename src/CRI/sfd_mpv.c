/* CRI Sofdec: MPEG video decoder driver (sfd_mpv.c, SFD_tr_vd_mpv, transfer driver slot 2). It
 * feeds the video elementary stream from ring buffer 1 into an MPV handle, collects the decoded
 * frames in the frame pool managed by sfd_mpvf.c and hands them to the video output driver.
 *
 * Function and variable names are the official symbols of the same unit in the Resident Evil 4
 * build, where the functions appear in the same order; sfmpv_InitInf, sfmpv_ExecServerSub,
 * SFMPV_GetRead and SFMPV_AddRead differ in size there but sit at the same positions.
 * fn_803F5134 (the sfmpv_ChkBufSiz position there, but a different size) and fn_803F894C keep
 * placeholder names. The names of the static inline helpers, which exist only as inline
 * expansions, are inferred from what they do.
 *
 * The unit is compiled with deferred inlining, so the functions are defined in the reverse of
 * their order in .text and the variables in the reverse of their order in .bss. */
#include <stddef.h>
#include <string.h>

#include "CRI/mpv.h"
#include "CRI/sfd.h"
#include "CRI/uty.h"

#define SFMPV_WK(sfd) ((SFMPV_WORK *)(sfd)->tr[SFMPV_TR].hn)

/* application callbacks held in the conditions: the check of a sequence header (picture size;
 * nonzero rejects it) and the receiver of the header bytes */
typedef Sint32 (*SFMPV_SIZECB)(void *obj, Sint32 width, Sint32 height);
typedef void (*SFMPV_HDRCB)(void *obj, Uint8 *data, Sint32 size);

/* MPV conditions SFMPV_SaveCond saves at most */
#define SFMPV_COND_NUM 16

/* error codes */
#define SFMPV_ERR_INIT 0xFF000F01
#define SFMPV_ERR_DECODE 0xFF000F06 /* MPV_DecodeFrmSj failed */
#define SFMPV_ERR_CREATE 0xFF000F0A
#define SFMPV_ERR_SETERRFN 0xFF000F0B
#define SFMPV_ERR_DESTROY 0xFF000F0C
#define SFMPV_ERR_NOTSUPPORTED 0xFF000F0D
#define SFMPV_ERR_FRM_UNUSED 0xFF000F0E      /* the frame information was not handed out */
#define SFMPV_ERR_FRM_NOT_CURRENT 0xFF000F0F /* not the frame handed out last */
#define SFMPV_ERR_SETCOND 0xFF000F12
#define SFMPV_ERR_INIT_FF05 0xFF000F13 /* MPV_Init returned 0xFF03FF05 */
#define SFMPV_ERR_PARA 0xFF000F15
#define SFMPV_ERR_PICATR 0xFF000F1B
#define SFMPV_ERR_PICUSR 0xFF000F1D
#define SFMPV_ERR_FRM_ID 0xFF000F1F      /* no frame has this id (released twice) */
#define SFMPV_ERR_PICATR_DEC 0xFF000F04  /* MPV_DecodePicAtrSj failed */
#define SFMPV_ERR_PICATR_GET 0xFF000F05  /* MPV_GetPicAtr failed */
#define SFMPV_ERR_SKIP 0xFF000F07        /* MPV_SkipFrmSj failed */
#define SFMPV_ERR_BITRATE 0xFF000F16     /* MPV_GetBitRate failed */
#define SFMPV_ERR_FRMSIZE 0xFF000F17     /* the frame buffers do not hold the picture size */
#define SFMPV_ERR_PICATR_SIZE 0xFF000F19 /* MPV_PICATR is not 0x80 bytes */
#define SFMPV_ERR_VHDR_SIZE 0xFF000F1A   /* the video header analysis is too large */
#define SFMPV_ERR_RING_FULL 0xFF000F1C   /* the input ring is too full for another unit */
#define SFMPV_ERR_INT_SIZE 0xFF000F1E    /* Sint32 is not 4 bytes */
/* start code classes that begin a unit of the driver: the first slice and the picture bounds */
#define SFMPV_DLM_UNIT (MPV_DLM_PIC_BOUND | MPV_DLM_SLICE1)
/* PTS clock (90 kHz) ticks in a thousand seconds, for the frame rates of SFTIM_prate, which are in
 * frames per thousand seconds */
#define SFMPV_PTS_TICKS_PER_KSEC 90000000
/* bit_rate field of the sequence header for a variable bit rate stream */
#define SFMPV_BITRATE_VARIABLE 0x3FFFF
#define SFD_ERR_SETPICUSR 0xFF000185
#define SFD_ERR_SETMPVCOND 0xFF000181

Uint8 sfmpv_work[0x12024]; /* MPV library work */
/* Decoder resources the application supplies before the handle exists (sfmpv_para to
 * sfmpv_picusr_buf1siz: the parameters, the frame buffers, the picture user data area);
 * SFMPV_Destroy saves them from the work and SFMPV_Create restores them, so a handle re-created
 * by the player reset keeps its memory. */
SFMPV_PARA sfmpv_para;
void *sfmpv_rfb_adr_tbl[2];
void *sfmpv_ta_adr_tbl[SFMPV_FRM_NUM];
void *sfmpv_picusr_pbuf;
Sint32 sfmpv_picusr_bufnum;
Sint32 sfmpv_picusr_buf1siz;

Sint32 sfmpv_discard_wsiz; /* use unknown: the written functions only clear it */

static Sint32 sfmpv_ChkFatal(void);

Sint32 sfmpv_GetActiveSize(SFD sfd, Sint32 *size, Sint32 *cls, Sint32 *partial);
Bool sfmpv_NeedSafeDlmRefresh(SFBUF_RINF *inf, Sint32 head_cls, Uint8 *delim);
Sint32 sfmpv_DecodeOneUnit(SFD sfd, Sint32 size, Sint32 mask, Sint32 partial, Sint32 *more);
Sint32 sfmpv_Concat(SFD sfd, SJ sj);
Sint32 sfmpv_DecodePicAtr(SFD sfd, SJCK *ck, SJ sj, Sint32 mask, Sint32 *result);
static void sfmpv_CalcRepeatField(SFD sfd, MPV_PICATR *picatr, Sint32 newgop);
void sfmpv_DoReformTc(SFD sfd, MPV_PICATR *picatr, Sint64 pts, Sint32 newgop);
void sfmpv_Pts2Tc(Sint64 pts, Sint32 type, Sint32 drop, Sint32 tmpref, SFTIM_TC *tc);
Sint32 fn_803F5134(SFD sfd, SFD_VIDINF *vid);
Bool sfmpv_IsSkip(SFD sfd, SJCK *ck);

Sint32 SFMPV_Init(void);
Sint32 SFMPV_Finish(void);
Sint32 SFMPV_ExecServer(SFD sfd);
static Sint32 SFMPV_Create(SFD sfd);
Sint32 SFMPV_Destroy(SFD sfd);
Sint32 SFMPV_Standby(SFD sfd);
Sint32 SFMPV_Start(SFD sfd);
Sint32 SFMPV_Stop(SFD sfd);
Sint32 SFMPV_Pause(SFD sfd);
static Sint32 SFMPV_GetWrite(SFD sfd);
Sint32 SFMPV_AddWrite(SFD sfd);
/* Slots 0xB and 0xC: SFBUF_VfrmGetRead and SFBUF_VfrmAddRead pass the frame information and their
 * argument. When frames are handed out with an id (sfd->x58 == 2), GetRead returns the frame id
 * through it and AddRead receives that id. */
Sint32 SFMPV_GetRead(SFD sfd, SFD_VFRM_INF **inf, Sint32 *id);
Sint32 SFMPV_AddRead(SFD sfd, SFD_VFRM_INF *inf, Sint32 id);
void sfmpv_SetFrmInf(SFD sfd, SFMPV_FRM *frm, SFD_VFRM_INF **out);
Sint32 sfmpv_SetFrmPara(SFD sfd, MPV_PICATR *picatr, MPV_FRM *para, SFMPV_FRM **frm);
Sint32 sfmpv_GoDdelim(SFD sfd, SJ sj, Sint32 mask);
Sint32 sfmpv_DecodeFrm(SFD sfd, SJ sj);
Sint32 sfmpv_ExecServerSub(SFD sfd);
Sint32 SFMPV_Seek(SFD sfd);
void sfmpv_ErrFn(void *obj, Sint32 code);
Sint32 sfmpv_InitInf(SFD sfd, SFMPV_WORK *wk);

const SFD_TR_IF SFD_tr_vd_mpv = {
    SFMPV_Init,     SFMPV_Finish,  SFMPV_ExecServer, SFMPV_Create, SFMPV_Destroy,
    SFMPV_Standby,  SFMPV_Start,   SFMPV_Stop,       SFMPV_Pause,  SFMPV_GetWrite,
    SFMPV_AddWrite, SFMPV_GetRead, SFMPV_AddRead,    SFMPV_Seek,
};

/* Rounded frame rate for each MPEG frame_rate_code (1 23.976 ... 8 60 Hz; inferred from the
 * values), the frames per second of sfmpv_Pts2Tc and sfmpv_NextTc. */
const Sint32 sfmpv_fps_round[9] = {0, 24, 24, 25, 30, 30, 50, 60, 60};
/* Drop-frame time code constants for 29.97 and 59.94 Hz, as sfmpv_Pts2Tc uses them: frames per
 * hour, per ten minutes, in the first minute of the ten and in each of the other nine, then the
 * frames of the first second of those nine minutes that are counted (the others are dropped), the
 * nominal frames per second, the minutes per ten and the frames dropped at the start of a
 * minute. */
const Sint32 sfmpv_conv_29_97[8] = {107892, 17982, 1800, 1798, 28, 30, 10, 2};
const Sint32 sfmpv_conv_59_94[8] = {215784, 35964, 3600, 3596, 56, 60, 10, 4};

/* Matching workaround: #pragma dont_inline around fn_803F894C. The original calls it out of line
 * from SFMPV_GetRead, its only caller; with deferred inlining it is expanded there instead.
 * Revisit when the original unit's inline settings are known. */
#pragma dont_inline on
/* Next frame id: returns sfd->x5c and counts it up, back to 0 after an overflow. The frame gets the
 * id when frames are handed out with an id (inferred from SFMPV_GetRead). */
Sint32 fn_803F894C(SFD sfd) {
    Sint32 id = sfd->x5c++;

    if (sfd->x5c < 0) {
        sfd->x5c = 0;
    }
    return id;
}
#pragma dont_inline off

/* Saves the decoder parameters and the buffers for the reference frames (rfb) and the decoded
 * frames (ta) the application supplies, each aligned up to 32 bytes; SFMPV_Create picks them up. */
void SFD_SetMpvParaTbl(SFMPV_PARA *para, void **rfb, void **ta) {
    Sint32 i;

    sfmpv_para = *para;
    sfmpv_para.rfb_base = 0;
    sfmpv_para.ta_base = 0;
    sfmpv_rfb_adr_tbl[0] = (void *)(((Uint32)rfb[0] + 31) & ~31);
    sfmpv_rfb_adr_tbl[1] = (void *)(((Uint32)rfb[1] + 31) & ~31);
    for (i = 0; i < SFMPV_FRM_NUM; i++) {
        if (i < para->nfrm) {
            sfmpv_ta_adr_tbl[i] = (void *)(((Uint32)ta[i] + 31) & ~31);
        } else {
            sfmpv_ta_adr_tbl[i] = 0;
        }
    }
}

/* Places the three planes of a frame buffer: luma first, then the two chroma planes. h16 is the
 * picture height rounded up to whole macroblocks. pln[2] is the luma plane, pln[0] and pln[1] the
 * chroma planes. */
static inline void sfmpv_LayoutPlanes(MPV_RFB *rfb, void *buf, Sint32 ypitch, Sint32 cpitch,
                                      Sint32 h16) {
    rfb->ypitch = ypitch;
    rfb->cpitch = cpitch;
    rfb->pln[2] = buf;
    rfb->pln[0] = rfb->pln[2] + h16 * ypitch;
    rfb->pln[1] = rfb->pln[0] + h16 / 2 * cpitch;
}

/* Rounds a picture size up to the layout of a frame buffer: the width to whole macroblocks, the
 * pitches of the luma and chroma (half width) planes to 32 bytes, and the height to whole
 * macroblocks. */
static inline void sfmpv_CalcPitch(Sint32 width, Sint32 height, Sint32 *ypitch, Sint32 *cpitch,
                                   Sint32 *h16) {
    Sint32 w16 = (width + 15) / 16 * 16;

    *ypitch = (w16 + 31) / 32 * 32;
    *cpitch = (w16 / 2 + 31) / 32 * 32;
    *h16 = (height + 15) / 16 * 16;
}

/* Lays out a frame buffer at buf as the decoder wants it: the luma plane of the picture size
 * rounded up to whole macroblocks, then the two chroma planes at half width and height. The rows
 * are padded to 32 bytes. */
void SFD_CalcYccPlane(void *buf, Sint32 width, Sint32 height, MPV_RFB *rfb) {
    Sint32 ypitch;
    Sint32 cpitch;
    Sint32 h16;

    sfmpv_CalcPitch(width, height, &ypitch, &cpitch, &h16);
    sfmpv_LayoutPlanes(rfb, buf, ypitch, cpitch, h16);
}

/* Sets a condition of the decoder; with sfd 0, MPV_SetCond sets it as the library default and in
 * every ready MPV handle. */
Sint32 SFD_SetMpvCond(SFD sfd, Sint32 id, Sint32 val) {
    MPV hn;

    if (sfd == 0) {
        hn = 0;
    } else {
        if (SFLIB_CheckHn(sfd) != 0) {
            return SFLIB_SetErr(0, SFD_ERR_SETMPVCOND);
        }
        hn = SFMPV_WK(sfd)->mpv;
    }
    if (id == 5) {
        val = 0;
    }
    if (MPV_SetCond(hn, id, val) != 0) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_SETCOND);
    }
    return 0;
}

/* Saves the MPV conditions (at most 16, as many as fit in size bytes); returns how many. */
Sint32 SFMPV_SaveCond(SFD sfd, void *buf, Uint32 size) {
    MPV hn = SFMPV_WK(sfd)->mpv;
    Sint32 *cond = buf;
    Sint32 i;
    Sint32 n;

    if (hn == 0) {
        return 0;
    }
    n = size / 4 > SFMPV_COND_NUM ? SFMPV_COND_NUM : size / 4;
    for (i = 0; i < n; i++) {
        MPV_GetCond(hn, i, &cond[i]);
    }
    return n;
}

/* Restores the MPV conditions saved by SFMPV_SaveCond. */
void SFMPV_RestoreCond(SFD sfd, void *buf, Sint32 n) {
    MPV hn = SFMPV_WK(sfd)->mpv;
    Sint32 *cond = buf;
    Sint32 i;

    if (hn == 0) {
        return;
    }
    for (i = 0; i < n; i++) {
        MPV_SetCond(hn, i, cond[i]);
    }
}

/* Clears the picture user data bookkeeping. */
static inline void sfmpv_ClearPicUsr(SFMPV_PICUSRINF *pu) {
    int i; /* int: a Sint32 counter adds a guard before the loop that the original lacks */

    pu->buf = 0;
    pu->num = 0;
    pu->siz = 0;
    pu->dat = 0;
    pu->len = 0;
    for (i = 0; i < SFMPV_FRM_NUM; i++) {
        pu->slot[i].buf = 0;
        pu->slot[i].len = 0;
    }
}

/* Hands the decoder the picture user data area: num slots of siz bytes at buf. Slot 0 collects
 * the data of the picture being decoded, each decoded frame gets one of the others. */
static inline Sint32 sfmpv_SetPicUsrBuf(SFD sfd, Uint8 *buf, Sint32 num, Sint32 siz) {
    SFMPV_PICUSRINF *pu = &SFMPV_WK(sfd)->picusr;
    Sint32 n;
    Sint32 i;
    Uint8 *p;

    p = buf;
    if (p == 0 || num == 0 || siz == 0) {
        sfmpv_ClearPicUsr(pu);
        return 0;
    }
    if (num < sfd->prm.x2c + 3) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_PICUSR);
    }
    pu->buf = p;
    pu->num = num;
    pu->siz = siz;
    pu->dat = p;
    pu->len = 0;
    p += siz;
    n = num - 1;
    for (i = 0; i < (n < SFMPV_FRM_NUM ? n : SFMPV_FRM_NUM); i++) {
        pu->slot[i].buf = p;
        pu->slot[i].len = 0;
        p += siz;
    }
    return 0;
}

/* Gives the application's picture user data area (num slots of siz bytes) to the decoder. */
Sint32 SFD_SetPicUsrBuf(SFD sfd, Uint8 *buf, Sint32 num, Sint32 siz) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, SFD_ERR_SETPICUSR);
    }
    return sfmpv_SetPicUsrBuf(sfd, buf, num, siz);
}

/* Brings up the MPV library on its work area and clears the saved resources. A failed
 * check in sfmpv_ChkFatal hangs the system. */
Sint32 SFMPV_Init(void) {
    Sint32 err;
    Sint32 code;

    if (sfmpv_ChkFatal() != 0) {
        for (;;) {
        }
    }
    err = MPV_Init(8, sfmpv_work);
    if (err != 0) {
        if (err == 0xFF03FF05) {
            code = SFMPV_ERR_INIT_FF05;
        } else {
            code = SFMPV_ERR_INIT;
        }
        return SFLIB_SetErr(0, code);
    }
    memset(&sfmpv_para, 0, sizeof(sfmpv_para));
    memset(sfmpv_rfb_adr_tbl, 0, sizeof(sfmpv_rfb_adr_tbl));
    memset(sfmpv_ta_adr_tbl, 0, sizeof(sfmpv_ta_adr_tbl));
    sfmpv_discard_wsiz = 0;
    return 0;
}

/* Layout checks of the driver, made at run time: the picture attributes, the part of the video
 * header analysis after its first 0x38 bytes and Sint32 must have the sizes the code assumes. All
 * three sizes are compile-time constants, so the compiler keeps only the compares. The quantities
 * and error codes follow the Resident Evil 4 decompilation of this file; the first check's code is
 * dropped from the bytes with its dead branch. Returns 0 when all hold.
 *
 * Matching workaround: #pragma dont_inline around sfmpv_ChkFatal. The original calls it out of
 * line from SFMPV_Init, its only caller; with deferred inlining it is expanded there instead
 * (SFMPV_Init 66 words against the original 45). Revisit when the original unit's inline settings
 * are known. */
#pragma dont_inline on
static Sint32 sfmpv_ChkFatal(void) {
    Sint32 picatr_size = sizeof(MPV_PICATR);
    Sint32 vhdr_size = sizeof(SFSEE_VHDR) - 0x38;
    Sint32 int_size = sizeof(Sint32);

    if (picatr_size != 0x80) {
        return SFLIB_SetErr(0, SFMPV_ERR_PICATR_SIZE);
    }
    if (vhdr_size > 0x204) {
        return SFLIB_SetErr(0, SFMPV_ERR_VHDR_SIZE);
    }
    if (int_size != 4) {
        return SFLIB_SetErr(0, SFMPV_ERR_INT_SIZE);
    }
    return 0;
}
#pragma dont_inline off

Sint32 SFMPV_Finish(void) {
    MPV_Finish();
    return 0;
}

/* Runs the decoder step of the driver server. */
Sint32 SFMPV_ExecServer(SFD sfd) {
    return sfmpv_ExecServerSub(sfd);
}

/* Whether the input side is ready: the input ring is finished, the file header is valid with
 * ftr_fixflg 0, the bit rate is variable, the decoder's VBV buffer worth of data has been supplied
 * or a ring is full. */
static inline Bool sfmpv_IsInputEnough(SFD sfd) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    MPV mpv = wk->mpv;
    Sint32 bitrate;
    Sint32 ring;

    if (SFBUF_GetTermFlg(sfd, sfd->tr[SFMPV_TR].bufin) == 1) {
        return TRUE;
    }
    if (sfd->fhd.valid != 0 && sfd->fhd.vid.ftr_fixflg == 0) {
        return TRUE;
    }
    MPV_GetBitRate(mpv, &bitrate);
    if (bitrate == SFMPV_BITRATE_VARIABLE) {
        return TRUE;
    }
    if (SFBUF_GetWTot(sfd, 1) >= wk->vbvsiz) {
        return TRUE;
    }
    ring = SFTRN_IsSetup(sfd, SFMPS_TR) == 0;
    if (SFBUF_GetWTot(sfd, ring) >= SFBUF_GetRingBufSiz(sfd, ring)) {
        return TRUE;
    }
    return FALSE;
}

/* Whether the video input is filled enough to start playing: the frame pool holds as many frames
 * as the smaller of cond 0x17 and prm.x2c (prm.x2c alone when cond 0x17 is -1; frames handed out
 * count as well with frame ids) and the input side is ready, or the decoder is finished. */
static inline Bool sfmpv_IsPrepared(SFD sfd) {
    Sint32 limit;
    Sint32 x2c;
    Sint32 nfrm;

    if (SFMPVF_IsTermDec(sfd) != 0) {
        return TRUE;
    }
    limit = sfd->cond[0x17];
    x2c = sfd->prm.x2c;
    if (limit == -1) {
        limit = x2c;
    }
    if (x2c < limit) {
        limit = x2c;
    }
    nfrm = SFMPVF_GetNumFrm(sfd);
    if (sfd->x58 == 2) {
        nfrm += sfd->plyinf.getcnt;
    }
    if (nfrm >= limit && sfmpv_IsInputEnough(sfd)) {
        return TRUE;
    }
    return FALSE;
}

/* Whether the video output has nothing more to hand out: the decoder is finished and no frame is
 * left, or (with frame access through SFD_GetFrm, x58 == 1, and one frame left) more frames were
 * handed out than released. */
static inline Bool sfmpv_IsOutputDone(SFD sfd, Sint32 nfrm) {
    if (SFMPVF_IsTermDec(sfd) != 0) {
        if (nfrm == 0) {
            return TRUE;
        }
        if (sfd->x58 == 1 && nfrm == 1 && sfd->plyinf.getcnt > sfd->plyinf.relcnt) {
            return TRUE;
        }
    }
    return FALSE;
}

/* One step of the video driver server: with cond 0x1C set and a colour type in the file header,
 * sets MPV condition 5 to 0; decodes the picture attributes the application supplied before the
 * stream, runs the decoder over the input ring until it needs more, keeps the flow statistics,
 * marks the output prepared once enough is decoded and terminates the output when the decoder has
 * finished. Returns an error code, 0 when there is none. */
Sint32 sfmpv_ExecServerSub(SFD sfd) {
    SFMPV_WORK *wk;
    MPV mpv;
    Sint32 bufin;
    Sint32 bufout;
    Sint32 nfrm;
    Sint32 err;
    SJCK ck;
    Sint32 used;
    Sint32 c;
    Sint32 b;
    Sint32 more;
    Sint32 a;
    Sint32 rcnt;
    Sint32 wcnt;
    SJ sj;

    if (SFSET_GetCond(sfd, SFD_COND_VIDEO_ON) == 0) {
        return 0;
    }
    if (SFBUF_GetTermFlg(sfd, sfd->tr[SFMPV_TR].bufout) == 1) {
        return 0;
    }
    if (SFSET_GetCond(sfd, 0x1C) != 0 && SFHDS_GetColType(sfd) != -1) {
        SFD_SetMpvCond(sfd, 5, 0);
    }
    if (sfd->stat == SFD_STAT_PREP) {
        wk = SFMPV_WK(sfd);
        mpv = wk->mpv;
        ck.data = (Uint8 *)SFSET_GetCond(sfd, SFD_COND_HDRDAT_ADR);
        ck.len = SFSET_GetCond(sfd, SFD_COND_HDRDAT_SIZE);
        if (ck.data != 0 && ck.len != 0 && wk->dlmmask == (MPV_DLM_END | MPV_DLM_SEQ)) {
            if (MPV_DecodePicAtr(mpv, &ck, &used) == 0) {
                wk->picstat = 2;
                wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ | MPV_DLM_GOP;
            }
        }
    }
    for (;;) {
        err = sfmpv_GetActiveSize(sfd, &a, &b, &c);
        if (err != 0) {
            break;
        }
        err = sfmpv_DecodeOneUnit(sfd, a, b, c, &more);
        if (err != 0) {
            break;
        }
        if (more == 0) {
            break;
        }
    }
    SFBUF_RingGetSj(sfd, sfd->tr[SFMPV_TR].bufin, &sj);
    if (sj != 0) {
        SFBUF_GetFlowCnt(sj, &wcnt, &rcnt);
        sfd->plyinf.v_flow = SFBUF_UpdateFlowCnt(sfd->plyinf.v_flow, wcnt);
    }
    bufout = sfd->tr[SFMPV_TR].bufout;
    bufin = sfd->tr[SFMPV_TR].bufin;
    if (SFBUF_GetPrepFlg(sfd, bufout) != 1 && SFBUF_GetPrepFlg(sfd, bufin) == 1) {
        if (sfmpv_IsPrepared(sfd)) {
            SFBUF_SetPrepFlg(sfd, bufout, 1);
            if (SFD_TIM(sfd)->vofst.val != 0x7FFFFFFF) {
                SFD_TIM(sfd)->vofst.valid = 1;
            }
        }
    }
    nfrm = SFMPVF_GetNumFrm(sfd);
    if (nfrm == -1 || sfmpv_IsOutputDone(sfd, nfrm)) {
        SFBUF_SetTermFlg(sfd, sfd->tr[SFMPV_TR].bufout, 1);
        if (sfd->plyinf.ndecpic == 0) {
            SFSET_SetCond(sfd, SFD_COND_VIDEO_ON, 0);
        }
    }
    return err;
}

/* Finds the first start code of the classes in mask in the data of a ring buffer, in the first
 * chunk, across the join of the chunks (a start code can straddle it: the last bytes of the first
 * chunk and the first bytes of the second are searched together), then in the second chunk. Returns
 * its address, 0 when there is none; *cls gets its class.
 *
 * Matching workaround: the join search is written out here and in sfmpv_FindLastDelim instead of
 * in a helper of its own. With a shared helper, sfmpv_GetActiveSize becomes 490 words against the
 * original's 474 (136 differing instead of 95), and sfmpv_NeedSafeDlmRefresh and sfmpv_GoDdelim
 * also move further from the original. Revisit when the original's source form of the search is
 * known. */
static inline Uint8 *sfmpv_FindDelimRing(SFBUF_RINF *inf, Sint32 mask, Sint32 *cls) {
    Uint8 edge[6];
    Uint8 *delim;
    Uint8 *q;
    Sint32 head;
    Sint32 tail;
    Sint32 i;
    Sint32 found;

    delim = (Uint8 *)MPV_SearchDelim((Sint8 *)inf->ck1.data, inf->ck1.len, mask);
    if (delim != 0) {
        *cls = MPV_CheckDelim(delim);
        return delim;
    }
    if (inf->ck2.len == 0) {
        return 0;
    }
    head = inf->ck1.len < 3 ? inf->ck1.len : 3;
    tail = inf->ck2.len < 3 ? inf->ck2.len : 3;
    memcpy(edge, inf->ck1.data + inf->ck1.len - head, head);
    memcpy(edge + head, inf->ck2.data, tail);
    q = edge;
    for (i = 0; i < head + tail - 3; i++, q++) {
        found = MPV_CheckDelim(q);
        if (found & mask) {
            *cls = found;
            return inf->ck1.data + inf->ck1.len - head + i;
        }
    }
    delim = (Uint8 *)MPV_SearchDelim((Sint8 *)inf->ck2.data, inf->ck2.len, mask);
    if (delim != 0) {
        *cls = MPV_CheckDelim(delim);
        return delim;
    }
    return 0;
}

/* Offset of an address in the data of a ring buffer (the first chunk, then the second); 0 when it
 * is in neither. */
static inline Sint32 sfmpv_RingOffset(SFBUF_RINF *inf, Uint8 *p) {
    if (inf->ck1.data <= p && p < inf->ck1.data + inf->ck1.len) {
        return p - inf->ck1.data;
    }
    if (inf->ck2.data <= p && p < inf->ck2.data + inf->ck2.len) {
        return inf->ck1.len + (p - inf->ck2.data);
    }
    return 0;
}

/* Checks that the input ring buffer is not so full that no further unit fits: the free room must
 * be at least the buffer's unit size. Returns 0, or the error code. */
static inline Sint32 sfmpv_CheckRingRoom(SFD sfd) {
    Sint32 bufin = sfd->tr[SFMPV_TR].bufin;
    Sint32 size = SFBUF_GetRingBufSiz(sfd, bufin);

    if (size - SFBUF_RingGetDataSiz(sfd, bufin) < sfd->prm.unit) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_RING_FULL);
    }
    return 0;
}

/* Finds a start code of the picture bounds classes in the data of a ring buffer: the last one in
 * the second chunk, else the first across the join of the chunks, else the last one in the first
 * chunk. Returns its address, 0 when there is none. */
static inline Uint8 *sfmpv_FindLastDelim(SFBUF_RINF *inf) {
    Uint8 edge[6];
    Uint8 *delim;
    Uint8 *q;
    Sint32 head;
    Sint32 tail;
    Sint32 i;

    if (inf->ck2.len != 0) {
        delim = (Uint8 *)MPV_BsearchDelim((Sint8 *)inf->ck2.data + inf->ck2.len, inf->ck2.len,
                                          MPV_DLM_PIC_BOUND);
        if (delim != 0) {
            /* the class is not used, but the original reads it */
            MPV_CheckDelim(delim);
            return delim;
        }
        head = inf->ck1.len < 3 ? inf->ck1.len : 3;
        tail = inf->ck2.len < 3 ? inf->ck2.len : 3;
        memcpy(edge, inf->ck1.data + inf->ck1.len - head, head);
        memcpy(edge + head, inf->ck2.data, tail);
        q = edge;
        for (i = 0; i < head + tail - 3; i++, q++) {
            if (MPV_CheckDelim(q) & MPV_DLM_PIC_BOUND) {
                return inf->ck1.data + inf->ck1.len - head + i;
            }
        }
    }
    delim = (Uint8 *)MPV_BsearchDelim((Sint8 *)inf->ck1.data + inf->ck1.len, inf->ck1.len,
                                      MPV_DLM_PIC_BOUND);
    if (delim != 0) {
        /* the class is not used, but the original reads it */
        MPV_CheckDelim(delim);
        return delim;
    }
    return 0;
}

/* Looks at the start of the video input ring. *cls gets the class of the start code the data
 * begins with, 0 when it does not begin with one. *size gets the length of the unit at the head:
 * 4 (the start code alone) while no later start code of the picture bounds is known, otherwise the
 * offset of that start code; with no start code at the head, the offset of the first start code in
 * the data, or all but the last three bytes when there is none (a start code may begin there).
 * *partial is set when those bytes are data without a start code. The later start code is kept in
 * the ring (SFBUF_RingSetDlm, with the end of the data searched) so that it is not searched for
 * again. Fails when that start code is not known and the free room of the ring is less than its
 * unit size (prm.unit). Returns an error code, 0 when there is none. */
Sint32 sfmpv_GetActiveSize(SFD sfd, Sint32 *size, Sint32 *cls, Sint32 *partial) {
    SFBUF_RINF inf;
    Sint32 bufin = sfd->tr[SFMPV_TR].bufin;
    Uint8 *delim;
    Sint32 head_cls;
    Uint8 *dlm;
    Uint8 *dlm_end;
    Uint8 *end;
    Uint8 *found;
    Sint32 next_cls;
    Sint32 err;
    Sint32 n;

    *size = 0;
    *cls = 0;
    *partial = 0;
    err = SFBUF_RingGetRead(sfd, bufin, &inf);
    if (err != 0) {
        return err;
    }
    if (inf.ck1.len == 0) {
        return 0;
    }

    head_cls = 0;
    delim = sfmpv_FindDelimRing(&inf, SFMPV_DLM_UNIT, &head_cls);
    if (delim != inf.ck1.data) {
        if (delim != 0) {
            *size = sfmpv_RingOffset(&inf, delim);
        } else {
            n = inf.ck1.len + inf.ck2.len - 3;
            *size = n < 0 ? 0 : n;
        }
        if (*size > 0) {
            *partial = 1;
        }
        return 0;
    }

    *cls = head_cls;
    *size = 4;
    if (head_cls & MPV_DLM_END) {
        return 0;
    }

    SFBUF_RingGetDlm(sfd, bufin, &dlm, &dlm_end);
    if (sfmpv_NeedSafeDlmRefresh(&inf, head_cls, dlm)) {
        dlm = 0;
        if (inf.ck2.len == 0) {
            end = inf.ck1.data + inf.ck1.len;
        } else {
            end = inf.ck2.data + inf.ck2.len;
        }
        if (dlm_end != end) {
            dlm_end = end;
            dlm = sfmpv_FindLastDelim(&inf);
            SFBUF_RingSetDlm(sfd, bufin, dlm, dlm_end);
        }
    }

    if (dlm == 0) {
        return sfmpv_CheckRingRoom(sfd);
    }
    switch (MPV_CheckDelim(dlm)) {
    case MPV_DLM_GOP:
        if (head_cls & MPV_DLM_SEQ) {
            found = sfmpv_FindDelimRing(&inf, MPV_DLM_GOP, &next_cls);
            if (found == 0 || found == dlm) {
                return sfmpv_CheckRingRoom(sfd);
            }
        }
        break;
    case MPV_DLM_PIC:
        if (head_cls & (MPV_DLM_GOP | MPV_DLM_SEQ)) {
            found = sfmpv_FindDelimRing(&inf, MPV_DLM_PIC, &next_cls);
            if (found == 0 || found == dlm) {
                return sfmpv_CheckRingRoom(sfd);
            }
        }
        break;
    case MPV_DLM_SEQ:
    case MPV_DLM_END:
    default:
        break;
    }
    *size = sfmpv_RingOffset(&inf, dlm);
    return 0;
}

/* Whether the start code recorded for the ring buffer (SFBUF_RingGetDlm, delim) has to be searched
 * for again. That is so when it is missing, at the start of the data or within 3 bytes of it, not
 * wholly inside the data, not a start code of the picture bounds, or a GOP or picture start code
 * for which a search of the data finds none or only that one. head_cls, the class of the start
 * code at the head of the ring, decides whether that search is made. */
Bool sfmpv_NeedSafeDlmRefresh(SFBUF_RINF *inf, Sint32 head_cls, Uint8 *delim) {
    Uint8 code[4];
    Uint8 *next;
    Sint32 cls;
    Sint32 over;

    if (delim == 0) {
        return TRUE;
    }
    if (delim == inf->ck1.data) {
        return TRUE;
    }
    if (delim > inf->ck1.data && delim - inf->ck1.data <= 3) {
        return TRUE;
    }

    /* the four bytes of the start code, which may be split between the chunks */
    if (delim >= inf->ck1.data && delim < inf->ck1.data + inf->ck1.len) {
        over = delim + 4 - (inf->ck1.data + inf->ck1.len);
        if (over > 0) {
            if (over > inf->ck2.len) {
                return TRUE;
            }
            memcpy(code, delim, 4 - over);
            memcpy(code + 4 - over, inf->ck2.data, over);
        } else {
            memcpy(code, delim, 4);
        }
    } else if (delim >= inf->ck2.data && delim < inf->ck2.data + inf->ck2.len) {
        if (delim + 4 - (inf->ck2.data + inf->ck2.len) > 0) {
            return TRUE;
        }
        memcpy(code, delim, 4);
    } else {
        return TRUE;
    }

    switch (MPV_CheckDelim(code)) {
    case MPV_DLM_GOP:
        if (head_cls & MPV_DLM_SEQ) {
            next = sfmpv_FindDelimRing(inf, MPV_DLM_GOP, &cls);
            if (next == 0 || next == delim) {
                return TRUE;
            }
        }
        break;
    case MPV_DLM_PIC:
        if (head_cls & (MPV_DLM_GOP | MPV_DLM_SEQ)) {
            next = sfmpv_FindDelimRing(inf, MPV_DLM_PIC, &cls);
            if (next == 0 || next == delim) {
                return TRUE;
            }
        }
        break;
    case MPV_DLM_SEQ:
    case MPV_DLM_END:
        break;
    default:
        return TRUE;
    }
    return FALSE;
}

/* Discards the sequence end codes at the head of the stream joint's data, counting their bytes
 * as consumed. */
static inline void sfmpv_SkipEndCodes(SFD sfd, SJ sj) {
    SJCK ck;

    for (;;) {
        SJ_GetChunk(sj, SJ_CK_DATA, 4, &ck);
        if (ck.len != 4 || MPV_CheckDelim(ck.data) != MPV_DLM_END) {
            break;
        }
        SJ_PutChunk(sj, SJ_CK_FREE, &ck);
        SFBUF_AddRtotSj(sfd, sfd->tr[SFMPV_TR].bufin, 4);
        sfd->plyinf.v_byte += 4;
    }
    SJ_UngetChunk(sj, SJ_CK_DATA, &ck);
}

/* Whether the unit at the head of the input ends the stream: a sequence end code, or only the
 * start code (4 bytes) is there and the input has terminated. */
static inline Bool sfmpv_IsStreamEnd(SFD sfd, Sint32 mask, Sint32 size) {
    if (mask == MPV_DLM_END) {
        return TRUE;
    }
    if (size <= 4 && SFBUF_GetTermFlg(sfd, sfd->tr[SFMPV_TR].bufin) == 1) {
        return TRUE;
    }
    return FALSE;
}

/* Gets the first chunk of the readable data of the video input ring in *ck; an empty chunk when
 * the ring cannot be read. */
static inline void sfmpv_GetHeadChunk(SFD sfd, SJCK *ck) {
    SFBUF_RINF inf;

    if (SFBUF_RingGetRead(sfd, sfd->tr[SFMPV_TR].bufin, &inf) != 0) {
        ck->data = 0;
        ck->len = 0;
    } else {
        *ck = inf.ck1;
    }
}

/* Decodes one unit at the head of the video input ring. size, mask and partial are what
 * sfmpv_GetActiveSize reports: the length of the unit, the class of its start code and whether it
 * is data without a start code. A sequence end code ends the decoder or, in concatenated play,
 * starts the next file; sequence, GOP and picture headers are decoded as picture attributes; the
 * first slice decodes the picture or, when sfmpv_IsSkip says so, skips it; other data is discarded
 * up to the next start code. *more is set when the server should look at the ring again. Returns an
 * error code, 0 when there is none. */
Sint32 sfmpv_DecodeOneUnit(SFD sfd, Sint32 size, Sint32 mask, Sint32 partial, Sint32 *more) {
    SFMPV_WORK *wk;
    SJCK ck;
    SJ sj;
    Sint32 ret;
    Sint32 result;
    Sint32 before;
    Sint32 flow;
    Sint32 err;

    *more = 0;
    wk = SFMPV_WK(sfd);
    sfd->plyinf.x28 = 0;
    if (wk->dlmmask != MPV_DLM_PIC_BOUND || wk->dlmwait == 0) {
        mask &= MPV_DLM_PIC_BOUND;
    }
    ret = SFBUF_RingGetSj(sfd, sfd->tr[SFMPV_TR].bufin, &sj);
    if (ret != 0) {
        return 0;
    }
    if (sj == 0) {
        return 0;
    }
    if (mask & (MPV_DLM_END | MPV_DLM_SEQ | MPV_DLM_GOP)) {
        SFMPVF_SetGopStat(sfd, 1);
    }

    if (mask == MPV_DLM_END) {
        SFTIM_TTU *tot = &SFD_TIM(sfd)->tot;

        if (sfd->tr[SFMPV_TR].x20 < 0) {
            sfd->tr[SFMPV_TR].x20 = SFBUF_GetRTot(sfd, sfd->tr[SFMPV_TR].bufin) + 4;
        }
        if (tot->val < 0) {
            *tot = SFD_TIM(sfd)->ttu1;
        }
    }
    if (mask == MPV_DLM_END && SFCON_IsEndcodeSkip(sfd)) {
        if (sfmpv_Concat(sfd, sj) == 0) {
            *more = 1;
        }
    } else if (mask == MPV_DLM_END && SFCON_IsVideoEndcodeSkip(sfd)) {
        sfmpv_SkipEndCodes(sfd, sj);
        *more = 1;
    } else if (partial == 0 && sfmpv_IsStreamEnd(sfd, mask, size)) {
        SFMPVF_TermDec(sfd);
    } else if (partial == 0 && size <= 4) {
        sfd->plyinf.x28 = 1;
    } else if (mask & (MPV_DLM_SEQ | MPV_DLM_PIC | MPV_DLM_GOP)) {
        sfmpv_GetHeadChunk(sfd, &ck);
        ret = sfmpv_DecodePicAtr(sfd, &ck, sj, mask, &result);
        if (ret == 0) {
            if (result == 0) {
                if (mask & wk->dlmmask) {
                    wk->dlmmask = MPV_DLM_PIC_BOUND;
                }
                wk->dlmwait = 1;
            }
            if (mask == MPV_DLM_SEQ && result == -2) {
                wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ;
            }
            *more = 1;
        }
    } else if (mask & MPV_DLM_SLICE1) {
        sfmpv_GetHeadChunk(sfd, &ck);
        if (sfmpv_IsSkip(sfd, &ck)) {
            if (SFD_TIM(sfd)->ttu3.val < SFD_TIM(sfd)->vofst.val) {
                SFD_TIM(sfd)->vstart = SFD_TIM(sfd)->ttu3;
            }
            before = SJRBF_GetFlowCnt(sj, 0, 1);
            result = MPV_SkipFrmSj(wk->mpv, sj);
            flow = SJRBF_GetFlowCnt(sj, 0, 1) - before;
            switch (result) {
            case 0:
                err = 0;
                break;
            case -2:
                err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -2);
                break;
            case -3:
                err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -3);
                break;
            default:
                err = SFLIB_SetErr(sfd, SFMPV_ERR_SKIP);
                break;
            }
            SFBUF_AddRtotSj(sfd, sfd->tr[SFMPV_TR].bufin, flow);
            sfd->plyinf.v_byte += flow;
            if (err == 0) {
                if (wk->picatr.x58 == 0) {
                    wk->skipret = 1;
                }
                SFPLY_AddSkipPic(sfd, 1, wk->picatr.pic_type);
            }
            ret = err;
            if (ret == 0) {
                *more = 1;
            }
        } else {
            ret = sfmpv_DecodeFrm(sfd, sj);
        }
    } else if (mask != MPV_DLM_END) {
        if (sfmpv_GoDdelim(sfd, sj, MPV_DLM_PIC_BOUND) > 0) {
            *more = 1;
        }
    }
    return ret;
}

/* The time code one picture after src: the frames are frm + frm2 + 1 plus half of x1c + field
 * (the repeated fields counted in half frames), carried into seconds, minutes and hours at the
 * nominal frame rate; the field is the remainder of that half. With drop frame time code a minute
 * that is not a tenth one starts at frame 2 instead of 0 or 1. Fills in the type, drop flag, time
 * and field of dst. */
static inline void sfmpv_NextTc(SFTIM_TC *src, SFTIM_TC *dst) {
    Sint32 fps = sfmpv_fps_round[src->type];
    Sint32 fields = src->x1c + src->field;
    Sint32 frames = src->frm + (src->frm2 + 1) + fields / 2;
    Sint32 field = fields % 2;
    Sint32 sec = src->sec + frames / fps;
    Sint32 min = src->min + sec / 60;
    Sint32 hour = src->hour + min / 60;

    frames %= fps;
    sec %= 60;
    min %= 60;
    if (src->drop != 0 && sec == 0 && min % 10 != 0 && (frames == 0 || frames == 1)) {
        frames = 2;
    }
    dst->type = src->type;
    dst->drop = src->drop;
    dst->hour = hour;
    dst->min = min;
    dst->sec = sec;
    dst->frm = frames;
    dst->field = field;
}

/* A sequence end code in concatenated play: adds the duration of the finished file to the
 * concatenation time, starts the times of the next file afresh and discards the end codes. The
 * duration is that of the time code after the last one when audio is off, else it comes from the
 * sample count the audio driver reports for the file (44100 Hz and none when the driver is not the
 * ADX one). Returns -1 when the audio driver has no sample count, 0 otherwise. */
Sint32 sfmpv_Concat(SFD sfd, SJ sj) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    SFTIM_WORK *tim = SFD_TIM(sfd);
    Sint32 total;
    Sint32 sfreq;
    Sint32 ncount;
    Sint32 tscale;
    SFTIM_TC tc;
    Sint32 diff;
    Bool read;

    if (SFSET_GetCond(sfd, SFD_COND_AUDIO_ON) == 0) {
        if (tim->ttu1.valid == 0) {
            diff = 0;
        } else {
            sfmpv_NextTc(&tim->ttu1.tc, &tc);
            tc.frm2 = 0;
            SFTIM_Tc2Time(&tc, &ncount, &tscale);
            diff = ncount - tim->ttu0.val;
        }
    } else {
        if (sfd->prm.trif_tbl[SFADXT_TR] != (SFD_TR_FUNC *)&SFD_tr_ad_adxt) {
            total = 0;
            sfreq = 44100;
            read = TRUE;
        } else {
            read = SFCON_ReadTotSmplQue(sfd, &total, &sfreq);
        }
        if (read == 0) {
            diff = -1;
        } else {
            tim->x1f0 += total;
            diff = UTY_MulDiv(tim->x1f0, tim->ttu0.unit, sfreq) - tim->ctime;
            if (diff < 0) {
                diff = 0;
            }
        }
        if (diff < 0) {
            return -1;
        }
    }
    if (diff > 0) {
        SFCON_UpdateConcatTime(sfd, diff);
        wk->nconcat++;
    }
    SFTIM_InitTtu(&tim->ttu0, 0x7FFFFFFF);
    SFTIM_InitTtu(&tim->ttu1, -1);
    wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ;
    sfmpv_SkipEndCodes(sfd, sj);
    return 0;
}

/* Sets a time (time code, position in the time base less base, unit) from a time code. */
static inline void sfmpv_SetTtu(SFTIM_TTU *ttu, SFTIM_TC tc, Sint32 base) {
    Sint32 ncount;
    Sint32 tscale;

    SFTIM_Tc2Time(&tc, &ncount, &tscale);
    ttu->tc = tc;
    ttu->val = ncount - base;
    ttu->unit = tscale;
    ttu->valid = 1;
}

/* Whether the time code of the new GOP does not follow on from that of the previous one: it
 * is not later than the previous time code or is later by at least condition 0x35 seconds. */
static inline Bool sfmpv_IsTcBreak(SFD sfd) {
    SFTIM_WORK *tim = SFD_TIM(sfd);
    SFTIM_TC tc;
    Sint32 now;
    Sint32 prev;
    Sint32 tscale;
    Sint32 limit;

    if (tim->ttu1.valid == 0) {
        return FALSE;
    }
    tc = tim->tc;
    SFTIM_Tc2Time(&tc, &now, &tscale);
    SFTIM_Tc2Time(&tim->ttu1.tc, &prev, &tscale);
    limit = prev + tscale * SFSET_GetCond(sfd, 0x35);
    if (now > prev && now < limit) {
        return FALSE;
    }
    return TRUE;
}

/* The video header analysis of the seek work, or 0 when no seek work is attached or after a
 * concatenation (nconcat > 0). */
static inline SFSEE_VHDR *sfmpv_GetVhdr(SFD sfd) {
    SFSEE_WORK *see = sfd->see.wk;

    if (see == 0) {
        return 0;
    }
    if (SFMPV_WK(sfd)->nconcat > 0) {
        return 0;
    }
    return &see->vid;
}

/* Decodes the sequence, GOP or picture header at the head of sj into the picture attributes (ck is
 * the first chunk of the input; mask the class of the start code). *result gets the decoder's
 * result; -2 means the header is to be ignored (a sequence header that changes the picture size or
 * that the application's check rejects). Then it matches the picture with the PTS recorded for its
 * position, sets the current time code and the times from it and, once, takes the stream
 * information from the first sequence header and sets the frame buffers up for it. Returns an error
 * code, 0 when there is none. */
Sint32 sfmpv_DecodePicAtr(SFD sfd, SJCK *ck, SJ sj, Sint32 mask, Sint32 *result) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    MPV mpv = wk->mpv;
    MPV_PICATR *picatr = &wk->picatr;
    SFTIM_WORK *tim;
    SFSEE_VHDR *vhdr;
    SFPTS_ENT ent;
    SFTIM_TC tc;
    SFTIM_TTU *ttu;
    void *cbobj;
    SFMPV_SIZECB sizecb;
    SFMPV_HDRCB hdrcb;
    Sint8 *slice;
    Sint8 *pic;
    Sint64 pts;
    Sint64 rawpts;
    Sint64 delta;
    Sint32 rate;
    Sint32 tmpref;
    Sint32 newgop;
    Sint32 before;
    Sint32 flow;
    Sint32 err;
    Sint32 ncount;
    Sint32 tscale;
    Sint32 bitrate;
    Sint32 bufsiz;
    Sint32 delay;
    Sint32 delay_bytes;
    Sint32 stamped;
    Sint32 seqhdr;
    Sint32 n;

    wk->picusr.len = 0;
    MPV_SetPicUsrBuf(mpv, wk->picusr.dat, wk->picusr.siz);
    before = SJRBF_GetFlowCnt(sj, 0, 1);
    *result = MPV_DecodePicAtrSj(mpv, sj);
    flow = SJRBF_GetFlowCnt(sj, 0, 1) - before;
    switch (*result) {
    case 0:
        err = 0;
        break;
    case -2:
        err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -2);
        break;
    case -3:
        err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -3);
        break;
    default:
        err = SFLIB_SetErr(sfd, SFMPV_ERR_PICATR_DEC);
        break;
    }
    SFBUF_AddRtotSj(sfd, sfd->tr[SFMPV_TR].bufin, flow);
    sfd->plyinf.v_byte += flow;
    if (err != 0) {
        return err;
    }
    if (*result == -2) {
        return 0;
    }
    err = MPV_GetPicAtr(mpv, picatr);
    *result = err;
    if (err != 0) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_PICATR_GET);
    }

    /* a sequence header: a change of the picture size is ignored, and so is a sequence the
     * application's check rejects; the callback and its object are both read with condition 0x5F,
     * as the original does */
    seqhdr = mask & MPV_DLM_SEQ;
    if (seqhdr) {
        if (sfd->vid.width > 0 &&
            (sfd->vid.width != picatr->width || sfd->vid.height != picatr->height)) {
            *result = -2;
            return 0;
        }
        sizecb = (SFMPV_SIZECB)SFSET_GetCond(sfd, 0x5F);
        cbobj = (void *)SFSET_GetCond(sfd, 0x5F);
        if (sizecb != 0 && sizecb(cbobj, picatr->width, picatr->height) != 0) {
            *result = -2;
            return 0;
        }
    }

    if (picatr->pic_type == 1) {
        wk->tmpref_adj = 0;
    } else if (sfd->prm.x38 == 3 && wk->ref[1] != 0) {
        Sint32 ref_tmpref = wk->ref[1]->picatr.temp_ref;

        if (picatr->pic_type == 2) {
            if (picatr->temp_ref < ref_tmpref && ref_tmpref < 0x200) {
                wk->tmpref_adj = 1;
            }
        } else if (picatr->pic_type == 3) {
            if (picatr->temp_ref >= ref_tmpref) {
                wk->tmpref_adj = 1;
            }
        }
    }
    MPV_GetPicUsr(mpv, 0, &wk->picusr.len);
    if (wk->last_ngop != picatr->ngop) {
        wk->last_ngop = picatr->ngop;
        wk->newgop = 1;
    } else {
        wk->newgop = 0;
    }

    /* the application's header callback gets the bytes up to the first slice */
    if (seqhdr) {
        hdrcb = (SFMPV_HDRCB)SFSET_GetCond(sfd, 0x4D);
        cbobj = (void *)SFSET_GetCond(sfd, 0x4E);
        if (hdrcb != 0) {
            slice = MPV_SearchDelim((Sint8 *)ck->data, ck->len, MPV_DLM_SLICE);
            if (slice != 0) {
                hdrcb(cbobj, ck->data, (Uint8 *)slice + 4 - ck->data);
            }
        }
    }

    /* the PTS recorded for this picture's position in the input gives the picture's time */
    pic = MPV_SearchDelim((Sint8 *)ck->data, ck->len, MPV_DLM_PIC);
    newgop = wk->newgop;
    tim = SFD_TIM(sfd);
    rawpts = -1;
    pts = -1;
    if (pic != 0) {
        SFPTS_ReadPtsQue(sfd, sfd->tr[SFMPV_TR].bufin, (Uint32)pic, &ent);
        if (ent.pts >= 0) {
            rate = SFTIM_prate[picatr->frame_rate];
            tmpref = picatr->temp_ref;
            if (tim->pts_base < 0) {
                delta = ent.pts - (Sint64)tmpref * SFMPV_PTS_TICKS_PER_KSEC / rate;
                tim->pts_base = delta > 0 ? delta : 0;
            }
            pts = ent.pts - tim->pts_base;
            pts = pts > 0 ? pts : 0;
            /* Only the first 4 bytes (the high word of the PTS) are compared, as in the
             * original. */
            if (memcmp(&wk->ptsent, &ent, 4) != 0) {
                wk->ptsent = ent;
                wk->pts_ofst = 0;
                wk->pts_tmpref = tmpref;
                if (picatr->pic_type == 3) {
                    wk->pts_max = 1;
                } else {
                    wk->pts_max = 0;
                }
                rawpts = ent.pts;
            } else {
                if (newgop) {
                    wk->pts_ofst = wk->pts_max + wk->pts_ofst + 1;
                    wk->pts_max = 0;
                    wk->pts_tmpref = 0;
                }
                n = tmpref - wk->pts_tmpref;
                wk->pts_max = wk->pts_max > n ? wk->pts_max : n;
                pts += (Sint64)(wk->pts_ofst + n) * SFMPV_PTS_TICKS_PER_KSEC / rate;
                pts = pts > 0 ? pts : 0;
            }
        }
    }
    wk->pts = rawpts;
    if ((mask & wk->dlmmask) == 0) {
        return 0;
    }

    sfmpv_CalcRepeatField(sfd, picatr, wk->newgop);
    newgop = wk->newgop;

    /* condition 0x34 is set once the stream is judged to carry its own times: a PTS, no GOP
     * header yet or picture attribute x57 nonzero, or a new GOP whose time code does
     * not follow on from the last */
    stamped = SFSET_GetCond(sfd, 0x34);
    if (stamped == 0) {
        if (pts >= 0 || picatr->ngop == 0 || picatr->x57 != 0 ||
            (newgop != 0 && sfmpv_IsTcBreak(sfd))) {
            SFSET_SetCond(sfd, 0x34, 1);
            stamped = 1;
        }
    }
    if (stamped == 1) {
        sfmpv_DoReformTc(sfd, picatr, pts, newgop);
    }

    ttu = &tim->ttu0;
    if (ttu->valid == 0) {
        tc = tim->tc;
        tc.frm2 = 0;
        sfmpv_SetTtu(ttu, tc, 0);
    }
    ttu = &tim->ttu3;
    sfmpv_SetTtu(ttu, tim->tc, tim->ttu0.val);
    if (tim->ttu1.val <= ttu->val) {
        tim->ttu1 = *ttu;
    }

    /* the stream information is taken from the first sequence header */
    if (sfd->vid.bitrate != 0) {
        return 0;
    }
    if (MPV_GetBitRate(mpv, &bitrate) != 0) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_BITRATE);
    }
    MPV_GetVbvBufSiz(mpv, &bufsiz, &delay, &delay_bytes);
    if (SFSET_GetCond(sfd, 0x3C) == 0) {
        wk->vbvsiz = 0;
    } else {
        n = SFBUF_GetRingBufSiz(sfd, 1);
        if (delay_bytes == -1) {
            delay_bytes = bufsiz;
        }
        wk->vbvsiz = delay_bytes < n ? delay_bytes : n;
    }
    vhdr = sfmpv_GetVhdr(sfd);
    if (vhdr != 0 && vhdr->analyzed == 0) {
        vhdr->hdrlen = ck->len < (Sint32)sizeof(vhdr->hdr) ? ck->len : (Sint32)sizeof(vhdr->hdr);
        MEM_Copy(vhdr->hdr, ck->data, vhdr->hdrlen);
        if (bitrate == SFMPV_BITRATE_VARIABLE) {
            vhdr->ncount = 0;
            vhdr->tscale = 0;
        } else {
            vhdr->ncount = bitrate * 50;
            vhdr->tscale = 1;
        }
        vhdr->ttu = tim->ttu0;
        vhdr->analyzed = 1;
    }
    sfd->vid.width = picatr->width;
    sfd->vid.height = picatr->height;
    sfd->vid.mb_width = picatr->mb_width;
    sfd->vid.mb_height = picatr->mb_height;
    sfd->vid.picrate = picatr->frame_rate;
    sfd->vid.bitrate = bitrate;
    sfd->x92c = bufsiz;
    return fn_803F5134(sfd, &sfd->vid);
}

/* The display time of a decoded frame from its time (ttu): the time with the concatenation time
 * added to it and the offset of the video start taken off, and the latest time seen. */
static inline void sfmpv_SetFrmTime(SFTIM_WORK *tim, SFMPV_FRM *frm) {
    frm->tunit = frm->ttu.unit;
    frm->ftime = tim->ctime + (frm->ttu.val - tim->vofst.val);
    frm->x4c = frm->ttu.val;
    frm->x50 = frm->ttu.val + tim->ctime;
    if (tim->x284 < frm->ftime) {
        tim->x284 = frm->ftime;
        tim->x288 = frm->tunit;
    }
}

/* Takes the GOP time code of the picture attributes, with the temporal reference as frames, as
 * the current time code and keeps the table of repeated fields (rfld, by temporal reference mod
 * 64): the repeat count of the picture and, as acc, the repeats counted before it, which give the
 * time code's field. newgop clears the table. The repeats a B picture brings are counted for the
 * reference picture that follows it (ref[1]), whose time is worked out again. */
static void sfmpv_CalcRepeatField(SFD sfd, MPV_PICATR *picatr, Sint32 newgop) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    SFTIM_WORK *tim = SFD_TIM(sfd);
    SFTIM_RFLD *rfld = tim->rfld;
    Sint32 first;
    Sint32 last;
    Sint32 idx;
    Sint32 i;
    Sint32 prev;
    Sint32 sum;
    Sint32 tscale;
    Sint32 ncount;

    tim->tc.type = picatr->frame_rate;
    tim->tc.drop = picatr->tc_drop;
    tim->tc.hour = picatr->tc_hour;
    tim->tc.min = picatr->tc_min;
    tim->tc.sec = picatr->tc_sec;
    tim->tc.frm = picatr->tc_pic;
    tim->tc.frm2 = picatr->temp_ref;
    tim->tc.x1c = picatr->x54;
    tim->tc.field = 0;

    if (newgop != 0) {
        for (i = 0; i < 64; i++) {
            rfld[i].rpt = -1;
        }
        rfld[0].acc = -1;
    } else if (picatr->pic_type == 1 || picatr->pic_type == 2) {
        /* the pictures between the previous reference picture and this one have no repeat count */
        first = wk->ref[1]->picatr.temp_ref;
        last = picatr->temp_ref;
        if (last < first) {
            last += 0x400;
        }
        for (i = first + 1; i < last; i++) {
            rfld[i % 64].rpt = -1;
        }
    }

    idx = picatr->temp_ref % 64;
    rfld[idx].rpt = tim->tc.x1c;
    if (newgop != 0) {
        rfld[idx].acc = 0;
    } else if (picatr->temp_ref == 0 && rfld[0].acc == -1) {
        rfld[0].acc = 0;
    } else {
        for (i = 0; i < 64; i++) {
            prev = (idx - i + 63) % 64;
            if (rfld[prev].rpt != -1) {
                rfld[idx].acc = rfld[prev].rpt + rfld[prev].acc;
                break;
            }
        }
    }
    tim->tc.field = rfld[idx].acc;

    if (picatr->pic_type == 3 && rfld[idx].rpt != 0) {
        sum = rfld[idx].rpt + rfld[idx].acc;
        idx = wk->ref[1]->picatr.temp_ref % 64;
        rfld[idx].acc = sum;
        wk->ref[1]->ttu.tc.field = rfld[idx].acc;
        SFTIM_Tc2Time(&wk->ref[1]->ttu.tc, &ncount, &tscale);
        wk->ref[1]->ttu.val = ncount - tim->ttu0.val;
        wk->ref[1]->ttu.unit = tscale;
        wk->ref[1]->ttu.valid = 1;
        if (tim->ttu1.val <= wk->ref[1]->ttu.val) {
            tim->ttu1 = wk->ref[1]->ttu;
        }
        sfmpv_SetFrmTime(tim, wk->ref[1]);
    }
}

/* Matching workaround: #pragma dont_inline around sfmpv_DoReformTc. The original calls it out of
 * line from sfmpv_DecodePicAtr; without the pragma the compiler expands it there (its one call),
 * which makes sfmpv_DecodePicAtr 827 words long instead of the original's 702 (371 differing words
 * instead of 231). Revisit when the original's source form is known. */
#pragma dont_inline on
/* Sets the current time code. With a PTS (newgop set, pts not negative) the time code is derived
 * from it; otherwise the time code continues from the last one (the one after it when a new GOP
 * began) or is zero when there is none yet and no seek work is attached. */
void sfmpv_DoReformTc(SFD sfd, MPV_PICATR *picatr, Sint64 pts, Sint32 newgop) {
    SFTIM_WORK *tim = SFD_TIM(sfd);
    Sint32 type = picatr->frame_rate;
    Sint32 drop = picatr->tc_drop;
    Sint32 tmpref = picatr->temp_ref;

    if (newgop != 0 && pts >= 0) {
        sfmpv_Pts2Tc(pts, type, drop, tmpref, &tim->tc);
        return;
    }
    if (tim->ttu1.valid == 0) {
        if (sfd->see.wk != 0) {
            return;
        }
        tim->tc.type = type;
        tim->tc.drop = 0;
        tim->tc.hour = 0;
        tim->tc.min = 0;
        tim->tc.sec = 0;
        tim->tc.frm = 0;
        return;
    }
    if (newgop != 0) {
        sfmpv_NextTc(&tim->ttu1.tc, &tim->tc);
        tim->rfld[0].acc = tim->tc.field;
        /* As in the original, tmpref is used without the modulo 64 of sfmpv_CalcRepeatField. */
        tim->rfld[tmpref].acc = tim->tc.field;
    } else {
        tim->tc.type = tim->ttu1.tc.type;
        tim->tc.drop = tim->ttu1.tc.drop;
        tim->tc.hour = tim->ttu1.tc.hour;
        tim->tc.min = tim->ttu1.tc.min;
        tim->tc.sec = tim->ttu1.tc.sec;
        tim->tc.frm = tim->ttu1.tc.frm;
    }
}
#pragma dont_inline off

/* The time code of a PTS (90 kHz): the field count at the picture rate less tmpref pictures (the
 * picture's temporal reference), split into hours, minutes, seconds and frames. Drop frame time
 * code at 29.97 and 59.94 Hz leaves out frames at the start of every minute but each tenth (the
 * constants of sfmpv_conv_29_97 and sfmpv_conv_59_94). */
void sfmpv_Pts2Tc(Sint64 pts, Sint32 type, Sint32 drop, Sint32 tmpref, SFTIM_TC *tc) {
    Sint32 rate = SFTIM_prate[type];
    Sint32 fps = sfmpv_fps_round[type];
    const Sint32 *conv;
    Sint32 fields;
    Sint32 frames;
    Sint32 hour;
    Sint32 min;
    Sint32 sec;
    Sint32 frm;
    Sint32 tenmin;
    Sint32 rest;

    fields = UTY_MulDivRound64(pts, rate * 2, SFMPV_PTS_TICKS_PER_KSEC);
    tc->field = fields & 1;
    frames = (fields >> 1) - tmpref;
    frames = frames > 0 ? frames : 0;
    tc->type = type;
    tc->drop = drop;
    if (drop != 0 && (rate == 29970 || rate == 59940)) {
        conv = sfmpv_conv_59_94;
        if (rate == 29970) {
            conv = sfmpv_conv_29_97;
        }
        hour = frames / conv[0];
        rest = frames % conv[0];
        tenmin = rest / conv[1];
        rest %= conv[1];
        if (rest < conv[2]) {
            min = 0;
            sec = rest / conv[5];
            frm = rest % conv[5];
        } else {
            rest -= conv[2];
            min = rest / conv[3] + 1;
            rest %= conv[3];
            if (rest < conv[4]) {
                sec = 0;
                frm = rest + conv[7];
            } else {
                rest -= conv[4];
                sec = rest / conv[5] + 1;
                frm = rest % conv[5];
            }
        }
        min += tenmin * conv[6];
    } else {
        sec = frames / fps;
        min = sec / 60;
        hour = min / 60;
        frm = frames % fps;
        sec %= 60;
        min %= 60;
    }
    tc->hour = hour;
    tc->min = min;
    tc->sec = sec;
    tc->frm = frm;
}

/* Sets a decoded frame to its initial state with its frame buffer. */
static inline void sfmpv_InitFrm(SFMPV_FRM *frm, void *buf) {
    frm->stat = SFMPV_FRM_FREE;
    frm->lock = 0;
    SFTIM_InitTtu(&frm->ttu, 0);
    frm->buf = buf;
    frm->ftime = 0;
    frm->tunit = 1;
    frm->x40 = 0;
    frm->x44 = 0;
    frm->gopno = 0;
    frm->x4c = 0;
    frm->x50 = 0;
    frm->x58 = -1;
    UTY_MemsetDword(&frm->picatr, -1, sizeof(frm->picatr) / 4);
}

/* Lays out the frame buffers for the stream's picture size (vid) in the memory the application
 * supplied: when the parameters give a base address (para.ta_base) for the decoded frames, as many
 * frames of this size as fit in the room of para.nfrm frames of the maximum size are laid out at
 * it and the reference frames at para.rfb_base; the frame objects take the buffers. Fails with
 * error 0xFF000F17 when the two reference frames of the size (laid out frmsize apart) do not fit in the
 * room of two of the maximum size or fewer than para.nfrm frames fit. Returns an error code, 0
 * when there is none. */
Sint32 fn_803F5134(SFD sfd, SFD_VIDINF *vid) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    Sint32 ypitch;
    Sint32 cpitch;
    Sint32 h16;
    Sint32 ysize;
    Sint32 csize;
    Sint32 frmsize;
    Sint32 maxypitch;
    Sint32 maxcpitch;
    Sint32 maxh16;
    Sint32 maxfrmsize;
    Sint32 n;
    Sint32 limit;
    Sint32 i;

    sfmpv_CalcPitch(vid->width, vid->height, &ypitch, &cpitch, &h16);
    ysize = h16 * ypitch;
    csize = h16 / 2 * cpitch;
    frmsize = ysize + csize * 2 + 32;
    sfmpv_CalcPitch(wk->para.width, wk->para.height, &maxypitch, &maxcpitch, &maxh16);
    maxfrmsize = maxh16 * maxypitch + maxh16 / 2 * maxcpitch * 2 + 32;
    if (frmsize * 2 > maxfrmsize * 2) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_FRMSIZE);
    }

    n = wk->para.nfrm;
    if (wk->para.ta_base != 0) {
        limit = wk->para.nfrm * maxfrmsize;
        /* As in the original, n ends at SFMPV_FRM_NUM + 1 when every frame fits, and the loop
         * below then writes one entry past the end of ta_adr. */
        for (n = 1; n <= SFMPV_FRM_NUM; n++) {
            if (n * frmsize > limit) {
                n--;
                break;
            }
        }
        if (n < wk->para.nfrm) {
            return SFLIB_SetErr(sfd, SFMPV_ERR_FRMSIZE);
        }
        wk->rfb_adr[0] = wk->para.rfb_base;
        wk->rfb_adr[1] = wk->para.rfb_base + frmsize;
        for (i = 0; i < n; i++) {
            wk->ta_adr[i] = wk->para.ta_base + i * frmsize;
        }
    }

    sfmpv_LayoutPlanes(&wk->rfbuf[0], wk->rfb_adr[0], ypitch, cpitch, h16);
    sfmpv_LayoutPlanes(&wk->rfbuf[1], wk->rfb_adr[1], ypitch, cpitch, h16);
    if (sfd->prm.x38 == 3) {
        /* two frames are kept as the reference pictures */
        wk->nfrm = (n < SFMPV_FRM_NUM - 2 ? n : SFMPV_FRM_NUM - 2) + 2;
        for (i = 0; i < 2; i++) {
            sfmpv_InitFrm(&wk->frm[i], wk->rfb_adr[i]);
        }
        for (i = 0; i < wk->nfrm - 2; i++) {
            sfmpv_InitFrm(&wk->frm[i + 2], wk->ta_adr[i]);
        }
        wk->ref[0] = SFMPVF_AllocFrm(sfd);
        wk->ref[1] = SFMPVF_AllocFrm(sfd);
    } else {
        wk->nfrm = n < SFMPV_FRM_NUM ? n : SFMPV_FRM_NUM;
        for (i = 0; i < wk->nfrm; i++) {
            sfmpv_InitFrm(&wk->frm[i], wk->ta_adr[i]);
        }
    }
    return 0;
}

/* Whether the picture to be decoded precedes the position the application asked to seek to: a
 * seek position is set, no video start offset has been taken yet and the picture's time is before
 * it. */
static inline Bool sfmpv_IsBeforeSeekPos(SFD sfd) {
    SFTIM_TTU *ttu = &SFD_TIM(sfd)->ttu3;
    SFTIM_TTU *vofst = &SFD_TIM(sfd)->vofst;

    if (sfd->see.req.pos < 0) {
        return FALSE;
    }
    if (vofst->valid != 0) {
        return FALSE;
    }
    if (UTY_CmpTime(sfd->see.req.pos, sfd->see.req.x08, ttu->val, ttu->unit) != 0) {
        return FALSE;
    }
    return TRUE;
}

/* Whether pictures of this type are switched off: conditions 2, 3 and 4 hold the switch for
 * I, P and B pictures; pictures of any other type are skipped. */
static inline Bool sfmpv_IsPicTypeOff(SFD sfd, Sint32 pic_type) {
    Sint32 enabled;

    switch (pic_type) {
    case 1:
        enabled = sfd->cond[2];
        break;
    case 2:
        enabled = sfd->cond[3];
        break;
    case 3:
        enabled = sfd->cond[4];
        break;
    default:
        return TRUE;
    }
    if (enabled != 0) {
        return FALSE;
    }
    return TRUE;
}

/* Whether the picture is empty (without coded macroblocks, as MPV_IsEmptyPpic and
 * MPV_IsEmptyBpic judge from the bytes of the picture), P and B pictures only and unless
 * condition 7 is set; counts the empty pictures found in the player information. */
static inline Bool sfmpv_IsEmptyPic(SFD sfd, SJCK *ck, Sint32 pic_type) {
    Bool empty;

    if (SFSET_GetCond(sfd, 7) != 0) {
        return FALSE;
    }
    if (pic_type == 3) {
        empty = MPV_IsEmptyBpic((Sint8 *)ck->data, ck->len, sfd->vid.mb_width * sfd->vid.mb_height);
        if (empty) {
            sfd->plyinf.x10++;
        }
        return empty;
    }
    if (pic_type == 2) {
        empty = MPV_IsEmptyPpic((Sint8 *)ck->data, ck->len, sfd->vid.mb_width * sfd->vid.mb_height);
        if (empty) {
            sfd->plyinf.x14++;
        }
        return empty;
    }
    return FALSE;
}

/* Whether the picture state calls for skipping this picture type: with picstat 2 the P and B
 * pictures, with picstat 3 the B pictures. */
static inline Bool sfmpv_IsPicStatSkip(SFD sfd, Sint32 pic_type) {
    Bool skip = FALSE;

    switch (SFMPV_WK(sfd)->picstat) {
    case 2:
        if (pic_type == 2 || pic_type == 3) {
            skip = TRUE;
        }
        break;
    case 3:
        if (pic_type == 3) {
            skip = TRUE;
        }
        break;
    }
    return skip;
}

/* Whether the picture is late: the display time of the frame (the application's check
 * SFD_SetUsrIsSkipFn when set) is behind the clock. Counts the skipped frames in a row. I and P
 * pictures update the time of the next input. */
static inline Bool sfmpv_IsLate(SFD sfd, Sint32 pic_type) {
    SFTIM_WORK *tim = SFD_TIM(sfd);
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    Sint32 ftime;
    Sint32 tunit;
    Sint32 ncount;
    Sint32 tscale;

    if (tim->vofst.valid == 0) {
        ftime = 0;
    } else {
        ftime = tim->ctime + (tim->ttu3.val - tim->vofst.val);
    }
    tunit = tim->ttu3.unit;
    if (tim->isskipfn != 0) {
        return tim->isskipfn(sfd, pic_type, ftime, tunit);
    }
    if (pic_type == 1) {
        SFTIM_UpdateItime(tim, ftime);
    }
    if (pic_type == 1 || pic_type == 2) {
        ftime = SFTIM_GetNextItime(tim, ftime);
    }
    if (SFTIM_GetSpeed(sfd) <= 1000 && wk->nskip >= sfd->cond[0x26]) {
        return FALSE;
    }
    SFTIM_GetTime(sfd, &ncount, &tscale);
    if (ncount < 0) {
        return FALSE;
    }
    ftime -= tunit * sfd->cond[0x2A] / sfd->cond[0x2B];
    if (UTY_CmpTime(ncount, tscale, ftime, tunit) != 0) {
        return FALSE;
    }
    wk->nskip++;
    return TRUE;
}

/* Decides whether the picture whose first slice is at the head of ck is skipped instead of
 * decoded, and advances the picture state (picstat) to follow: a skipped I or P picture puts the
 * decoder in state 2, a decoded one moves state 2 to 3 and 3 to 5 (5 is the state with all
 * pictures decoded; inferred from the transitions). A new GOP restarts at 5 when closed_gop is
 * set, else at 2 when broken_link is set (forced while there is no video start offset and
 * condition 0x49 is 1). Returns TRUE to skip. */
Bool sfmpv_IsSkip(SFD sfd, SJCK *ck) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    Sint32 pic_type;
    Bool skip;
    Sint32 state;
    Sint32 broken;
    Sint32 closed;

    if (SFSET_GetCond(sfd, 0x2F) == 1) {
        return TRUE;
    }
    if (SFSET_GetCond(sfd, 0x27) == 1) {
        return FALSE;
    }
    if (wk->picatr.x58 != 0) {
        return wk->skipret;
    }
    pic_type = wk->picatr.pic_type;
    if (sfmpv_IsBeforeSeekPos(sfd) || sfmpv_IsPicTypeOff(sfd, pic_type) ||
        sfmpv_IsEmptyPic(sfd, ck, pic_type) || sfmpv_IsPicStatSkip(sfd, pic_type) ||
        sfmpv_IsLate(sfd, pic_type)) {
        skip = TRUE;
    } else {
        skip = FALSE;
    }

    state = wk->picstat;
    if (wk->newgop != 0) {
        MPV_GetLinkFlg(wk->mpv, &closed, &broken);
        if (closed == 1) {
            state = 5;
        } else {
            if (SFD_TIM(sfd)->vofst.valid == 0 && SFSET_GetCond(sfd, 0x49) == 1) {
                broken = 1;
            }
            if (broken == 1) {
                state = 2;
            }
        }
    }
    if (skip == TRUE) {
        if (wk->picatr.pic_type == 1 || wk->picatr.pic_type == 2) {
            state = 2;
        }
    } else if (state == 2) {
        state = 3;
    } else if (state == 3) {
        state = 5;
    }
    wk->picstat = state;
    return skip;
}

/* Whether picstat is 2 or 3; with either, sfmpv_DecodeFrm keeps the time code's frm2. The
 * original's lowering of this switch also compares picstat with 4; the case set that produces it is
 * not known. */
static inline Bool sfmpv_IsPicStat23(SFD sfd) {
    switch (SFMPV_WK(sfd)->picstat) {
    case 2:
        return TRUE;
    case 3:
        return TRUE;
    default:
        return FALSE;
    }
}

/* Decodes one picture from the stream joint into a frame: sets up the frame, runs the decoder, adds
 * decode time and MPV-reported counters to the statistics. If data was consumed, it completes the
 * frame (display time, counts, state). With picture attribute x38 other than 3, the first picture
 * decoded into a frame leaves it pending and the next one completes it. Returns an error code, 0
 * when there is none. */
Sint32 sfmpv_DecodeFrm(SFD sfd, SJ sj) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    MPV mpv = wk->mpv;
    MPV_PICATR *picatr = &wk->picatr;
    SFMPV_FRM *frm;
    MPV_FRM para;
    SFTIM_WORK *tim = SFD_TIM(sfd);
    Sint32 ref_x40;
    Sint32 ref_x44;
    SFMPV_PICUSR *picusr;
    SFTIM_TTU *ttu0;
    SFTIM_TTU *vofst;
    SFTIM_TC tc;
    Sint32 tscale;
    Sint32 ncount;
    Sint64 start;
    Sint64 elapsed;
    Sint32 flow;
    Sint32 result;
    Sint32 err;

    if (sfmpv_SetFrmPara(sfd, picatr, &para, &frm) != 0) {
        return 0;
    }

    /* the counts of the reference frames a P or B picture is predicted from (the original's lowering
     * of this switch also compares pic_type with 4; the case set that produces it is not known) */
    if (sfd->prm.x38 == 3) {
        switch (wk->picatr.pic_type) {
        case 2:
            ref_x40 = wk->ref[0]->x40;
            ref_x44 = wk->ref[0]->x44;
            break;
        case 3:
            ref_x40 = wk->ref[0]->x40 + wk->ref[1]->x40;
            ref_x44 = wk->ref[0]->x44 + wk->ref[1]->x44;
            break;
        default:
            ref_x40 = 0;
            ref_x44 = 0;
            break;
        }
    } else {
        ref_x40 = 0;
        ref_x44 = 0;
    }
    picusr = frm->picusr;
    memcpy(picusr->buf, wk->picusr.dat, wk->picusr.len);
    picusr->len = wk->picusr.len;

    /* the first frame decoded fixes the video start offset from the GOP time code */
    ttu0 = &tim->ttu0;
    vofst = &tim->vofst;
    if (vofst->valid == 0) {
        tc = tim->ttu3.tc;
        if (!sfmpv_IsPicStat23(sfd) && sfd->cond[4] != 0) {
            tc.frm2 = 0;
        }
        SFTIM_Tc2Time(&tc, &ncount, &tscale);
        vofst->tc = tc;
        vofst->val = ncount - ttu0->val;
        vofst->unit = tscale;
        vofst->valid = 1;
    }

    start = fn_803FEA5C();
    flow = SJRBF_GetFlowCnt(sj, 0, 1);
    result = MPV_DecodeFrmSj(mpv, sj, &para);
    flow = SJRBF_GetFlowCnt(sj, 0, 1) - flow;
    elapsed = fn_803FEA5C() - start;
    SFTMR_AddTsum(&sfd->tsum[picatr->pic_type], elapsed);
    sfd->err.x0c += para.x28;
    sfd->err.x10 += para.x2c;

    switch (result) {
    case 0:
        err = 0;
        break;
    case -2:
        err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -2);
        break;
    case -3:
        err = flow > 0 ? 0 : SFLIB_SetErr(sfd, -3);
        break;
    default:
        err = SFLIB_SetErr(sfd, SFMPV_ERR_DECODE);
        break;
    }
    SFBUF_AddRtotSj(sfd, sfd->tr[SFMPV_TR].bufin, flow);
    sfd->plyinf.v_byte += flow;
    if (err != 0) {
        SFMPVF_FreeFrm(frm);
        return err;
    }

    if (flow > 0) {
        SFMPVF_SetGopStat(sfd, 0);
        frm->ttu = tim->ttu3;
        sfmpv_SetFrmTime(tim, frm);
        frm->gopno = wk->nconcat;
        frm->x40 = ref_x40 + wk->tmpref_adj + para.x28;
        frm->x44 = para.x2c + ref_x44;
        if (picatr->x38 == 3 || wk->pendfrm != 0) {
            wk->pendfrm = 0;
        } else {
            wk->pendfrm = frm;
        }
        wk->skipret = 0;
        wk->dlmwait = 0;
        if (wk->pendfrm == 0) {
            if (sfd->prm.x38 == 3 && (picatr->pic_type == 1 || picatr->pic_type == 2)) {
                SFMPVF_RefStbyFrm(frm);
            } else {
                SFMPVF_StbyFrm(frm);
            }
            MPV_GetDctCnt(mpv, &sfd->plyinf.x08, &sfd->plyinf.x0c);
            wk->nskip = 0;
        }
        SFPLY_AddDecPic(sfd, 1, picatr->pic_type);
    } else if (wk->pendfrm == 0) {
        SFMPVF_FreeFrm(frm);
    }
    return 0;
}

/* Takes the frame a picture is decoded into (the pending frame if there is one, otherwise a free
 * one; none free sets the flag in the player information and returns -1) and fills in the frame
 * parameter block for MPV_DecodeFrmSj: the picture attributes, the output frame and the two
 * reference frames the macroblocks predict from. I and P pictures (picture types 1 and 2) become
 * the newer reference and the previous one the older. */
Sint32 sfmpv_SetFrmPara(SFD sfd, MPV_PICATR *picatr, MPV_FRM *para, SFMPV_FRM **frm) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    Sint32 ypitch;
    Sint32 cpitch;
    Sint32 h16;

    if (wk->pendfrm != 0) {
        *frm = wk->pendfrm;
    } else {
        *frm = SFMPVF_AllocFrm(sfd);
        if (*frm == 0) {
            sfd->plyinf.x2c = 1;
            return -1;
        }
    }
    (*frm)->picatr = *picatr;
    (*frm)->pts = wk->pts;
    if (sfd->prm.x38 == 3) {
        if ((picatr->pic_type == 1 || picatr->pic_type == 2) && wk->pendfrm == 0) {
            SFMPVF_EndRefFrm(wk->ref[0]);
            wk->ref[0] = wk->ref[1];
            wk->ref[1] = *frm;
        }
        sfmpv_CalcPitch(picatr->width, picatr->height, &ypitch, &cpitch, &h16);
        sfmpv_LayoutPlanes(&para->rfb[0], wk->ref[0]->buf, ypitch, cpitch, h16);
        sfmpv_LayoutPlanes(&para->rfb[1], wk->ref[1]->buf, ypitch, cpitch, h16);
    } else {
        if (picatr->pic_type == 1 || picatr->pic_type == 2) {
            wk->refidx[0] ^= 1;
            wk->refidx[1] ^= 1;
            wk->ref[1] = *frm;
        }
        para->rfb[0] = wk->rfbuf[wk->refidx[0]];
        para->rfb[1] = wk->rfbuf[wk->refidx[1]];
    }
    para->buf = (*frm)->buf;
    para->picatr = &(*frm)->picatr;
    para->x28 = 0;
    para->x2c = 0;
    sfd->plyinf.x2c = 0;
    return 0;
}

/* Matching workaround: #pragma dont_inline around sfmpv_GoDdelim. The original calls it out of
 * line from sfmpv_DecodeOneUnit; without the pragma the compiler expands it there, which changes
 * the frame and most of the code of sfmpv_DecodeOneUnit (504 words against the original's 364, 322
 * differing instead of 170). Revisit when the original's source form of the search is known. */
#pragma dont_inline on
/* Discards the video input up to the next start code of the classes in mask (the start code
 * itself is kept) and counts the bytes: all of them in v_byte, and in v_skip as well when the
 * discarded bytes (the first three, or all of them when fewer) were not all zero. The
 * ring buffer data is two chunks, so a start code can straddle them; when none is found the last
 * three bytes stay, as they may begin one. Returns the number of bytes consumed. */
Sint32 sfmpv_GoDdelim(SFD sfd, SJ sj, Sint32 mask) {
    SFBUF_RINF inf;
    Uint8 *delim;
    Sint32 cls;
    Sint32 nbyte;
    Sint32 i;
    Sint32 len1;
    Uint8 *src;
    Uint8 *data2;
    Bool nonzero;

    if (SFBUF_RingGetRead(sfd, sfd->tr[SFMPV_TR].bufin, &inf) != 0) {
        return 0;
    }
    if (inf.ck1.len == 0) {
        return 0;
    }
    delim = sfmpv_FindDelimRing(&inf, mask, &cls);
    if (delim == 0) {
        nbyte = inf.ck1.len + inf.ck2.len - 3;
        nbyte = nbyte > 0 ? nbyte : 0;
    } else {
        nbyte = sfmpv_RingOffset(&inf, delim);
    }
    SFBUF_RingAddRead(sfd, sfd->tr[SFMPV_TR].bufin, nbyte);

    src = inf.ck1.data;
    len1 = inf.ck1.len;
    data2 = inf.ck2.data;
    nonzero = FALSE;
    for (i = 0; i < (nbyte < 3 ? nbyte : 3); i++, src++) {
        Uint8 *byte = i < len1 ? src : data2 + (i - len1);

        if (*byte != 0) {
            nonzero = TRUE;
            break;
        }
    }
    if (nonzero) {
        sfd->plyinf.v_skip += nbyte;
    }
    sfd->plyinf.v_byte += nbyte;
    return nbyte;
}
#pragma dont_inline off

/* Creates the decoder handle for the video stream when video is switched on. After a player
 * reset the picture user data area saved by SFMPV_Destroy is handed back. */
static Sint32 SFMPV_Create(SFD sfd) {
    SFMPV_WORK *wk;
    MPV hn;
    Sint32 err;

    if (SFSET_GetCond(sfd, SFD_COND_VIDEO_ON) == 0) {
        return 0;
    }
    wk = &sfd->mpv;
    sfd->tr[SFMPV_TR].hn = wk;
    err = sfmpv_InitInf(sfd, wk);
    if (err != 0) {
        return err;
    }
    hn = MPV_Create();
    if (hn == 0) {
        return SFLIB_SetErr(0, SFMPV_ERR_CREATE);
    }
    if (MPV_SetErrFunc(hn, sfmpv_ErrFn, sfd) != 0) {
        MPV_Destroy(hn);
        return SFLIB_SetErr(0, SFMPV_ERR_SETERRFN);
    }
    MPV_SetCond(hn, 1, SFSET_GetCond(sfd, 0));
    MPV_SetCond(hn, 2, SFSET_GetCond(sfd, 1));
    MPV_SetCond(hn, 6, sfd->prm.x38);
    wk->mpv = hn;
    if (SFPLY_GetResetFlg() != 0) {
        sfmpv_SetPicUsrBuf(sfd, sfmpv_picusr_pbuf, sfmpv_picusr_bufnum, sfmpv_picusr_buf1siz);
    }
    return 0;
}

/* Checks the parameters and buffers saved by SFD_SetMpvParaTbl: 0 when they describe a usable
 * frame pool, -1 otherwise. */
static inline Sint32 sfmpv_CheckPara(SFMPV_PARA *para) {
    Sint32 i;

    if (para->nfrm <= 0 || para->nfrm > SFMPV_FRM_NUM) {
        return -1;
    }
    if (para->rfb_base == 0 || para->ta_base == 0) {
        for (i = 0; i < 2; i++) {
            if (sfmpv_rfb_adr_tbl[i] == 0) {
                return -1;
            }
        }
        for (i = 0; i < para->nfrm; i++) {
            if (sfmpv_ta_adr_tbl[i] == 0) {
                return -1;
            }
        }
    }
    return 0;
}

/* Sets the work up from the saved resources: the decoder starts in the picture state 5 with all
 * frames free. */
Sint32 sfmpv_InitInf(SFD sfd, SFMPV_WORK *wk) {
    SFMPV_FRM *frm;
    int i;

    if (sfmpv_CheckPara(&sfmpv_para) != 0) {
        return SFLIB_SetErr(0, SFMPV_ERR_PARA);
    }
    wk->para = sfmpv_para;
    memcpy(wk->rfb_adr, sfmpv_rfb_adr_tbl, sizeof(wk->rfb_adr));
    memcpy(wk->ta_adr, sfmpv_ta_adr_tbl, sizeof(wk->ta_adr));
    wk->mpv = 0;
    wk->curfrm = 0;
    wk->picstat = 5;
    wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ;
    wk->refidx[0] = 0;
    wk->refidx[1] = 1;
    wk->termflg = 0;
    wk->gopstat = 0;
    wk->ref[0] = 0;
    wk->ref[1] = 0;
    wk->pendfrm = 0;
    wk->skipret = 0;
    wk->dlmwait = 0;
    wk->tmpref_adj = 0;
    frm = wk->frm;
    for (i = 0; i < SFMPV_FRM_NUM; i++, frm++) {
        frm->stat = SFMPV_FRM_FREE;
        frm->lock = 0;
        SFTIM_InitTtu(&frm->ttu, 0);
        frm->buf = wk->ta_adr[i];
        frm->ftime = 0;
        frm->tunit = 1;
        frm->x40 = 0;
        frm->x44 = 0;
        frm->gopno = 0;
        frm->x4c = 0;
        frm->x50 = 0;
        frm->x58 = -1;
        UTY_MemsetDword(&frm->picatr, -1, sizeof(frm->picatr) / 4);
    }
    wk->nskip = 0;
    wk->nconcat = 0;
    UTY_MemsetDword(&wk->picatr, -1, sizeof(wk->picatr) / 4);
    wk->last_ngop = -1;
    wk->newgop = 0;
    wk->vbvsiz = 0x7FFFFFFF;
    wk->pts_tmpref = 0;
    wk->pts_ofst = 0;
    wk->pts_max = 0;
    wk->ptsent.pts = -1;
    wk->ptsent.pos = 0;
    wk->ptsent.len = -1;
    sfmpv_ClearPicUsr(&wk->picusr);
    for (i = 0; i < SFMPV_FRM_NUM; i++) {
        wk->frm[i].picusr = &wk->picusr.slot[i];
    }
    return 0;
}

/* Error callback of the MPV handle: reports every MPV error code except 0, -2 and -3. */
void sfmpv_ErrFn(void *obj, Sint32 code) {
    switch (code) {
    case 0:
    case -2:
    case -3:
        break;
    default:
        SFLIB_SetErr((SFD)obj, code);
        break;
    }
}

/* Gives the decoder handle back, after saving the resources the application supplied. */
Sint32 SFMPV_Destroy(SFD sfd) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    MPV hn = wk->mpv;

    if (hn == 0) {
        return 0;
    }
    sfmpv_para = wk->para;
    memcpy(sfmpv_rfb_adr_tbl, wk->rfb_adr, sizeof(wk->rfb_adr));
    memcpy(sfmpv_ta_adr_tbl, wk->ta_adr, sizeof(wk->ta_adr));
    sfmpv_picusr_pbuf = wk->picusr.buf;
    sfmpv_picusr_bufnum = wk->picusr.num;
    sfmpv_picusr_buf1siz = wk->picusr.siz;
    if (MPV_Destroy(hn) != 0) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_DESTROY);
    }
    wk->mpv = 0;
    return 0;
}

Sint32 SFMPV_Standby(SFD sfd) {
    return 0;
}

Sint32 SFMPV_Start(SFD sfd) {
    return 0;
}

Sint32 SFMPV_Stop(SFD sfd) {
    return 0;
}

Sint32 SFMPV_Pause(SFD sfd) {
    return 0;
}

static Sint32 SFMPV_GetWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, SFMPV_ERR_NOTSUPPORTED);
}

Sint32 SFMPV_AddWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, SFMPV_ERR_NOTSUPPORTED);
}

/* Hands out the next frame to display: its information (SFD_VFRM_INF, *inf) when the frame is due,
 * otherwise *inf is 0. With frame ids the frame also gets an id, returned in *id. */
Sint32 SFMPV_GetRead(SFD sfd, SFD_VFRM_INF **inf, Sint32 *id) {
    SFMPV_FRM *frm = SFMPVF_HoldFrm(sfd);
    Sint32 frm_id;

    if (frm == 0) {
        *inf = 0;
        return 0;
    }
    sfmpv_SetFrmInf(sfd, frm, inf);
    SFD_TIM(sfd)->vterm = (*inf)->ftime;
    SFD_TIM(sfd)->vterm_unit = (*inf)->tunit;
    if (!SFTIM_IsGetFrmTime(sfd, *inf)) {
        *inf = 0;
        return 0;
    }
    if (sfd->x58 == 2) {
        frm_id = fn_803F894C(sfd);
        frm->x58 = frm_id;
        *id = frm_id;
    }
    return 0;
}

/* Fills in the picture information of the frame slot paired with frm, marks the slot as handed out
 * and makes frm the current frame. *out gets the information. */
void sfmpv_SetFrmInf(SFD sfd, SFMPV_FRM *frm, SFD_VFRM_INF **out) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    SFD_VFRM *vfrm = SFMPVF_SearchVfrmData(sfd, frm);

    *out = &vfrm->inf;
    vfrm->used = 1;
    wk->curfrm = frm;
    (*out)->width = frm->picatr.width;
    (*out)->height = frm->picatr.height;
    (*out)->mb_width = frm->picatr.mb_width;
    (*out)->mb_height = frm->picatr.mb_height;
    (*out)->pic_type = frm->picatr.pic_type;
    (*out)->ftime = frm->ftime;
    (*out)->tunit = frm->tunit;
    (*out)->x1c = sfd->prm.x38;
    (*out)->buf = frm->buf;
    (*out)->x24 = frm->x40;
    (*out)->x28 = frm->x44;
    (*out)->gopno = frm->gopno;
    (*out)->x30 = frm->x4c;
    (*out)->x34 = frm->x50;
    (*out)->picusr = frm->picusr;
    (*out)->x3c = frm->picatr.x40;
    (*out)->x40 = frm->picatr.x44;
    if (frm->picatr.x40 == 0) {
        (*out)->x48 = 2;
    } else {
        (*out)->x48 = 1;
    }
    (*out)->pts = frm->pts;
    (*out)->x58 = frm->picatr.x38;
    (*out)->x5c = frm->picatr.x3c;
    (*out)->bitrate = frm->picatr.bitrate;
    (*out)->vbv_size = frm->picatr.vbv_size;
    (*out)->x68 = frm->picatr.x50;
    (*out)->x6a = frm->picatr.x52;
    (*out)->x6c = frm->picatr.x55;
    (*out)->x6d = frm->picatr.x56;
    (*out)->x6e = frm->picatr.x57;
    (*out)->aspect = frm->picatr.aspect;
    (*out)->constrained = frm->picatr.constrained;
    (*out)->x71 = frm->picatr.x5b;
    (*out)->x72 = frm->picatr.x5c;
    (*out)->x73 = frm->picatr.x5d;
    (*out)->x74 = frm->picatr.x5e;
    (*out)->x75 = frm->picatr.x5f;
    (*out)->x76 = frm->picatr.x60;
    (*out)->x77 = frm->picatr.x61;
    (*out)->x78 = frm->picatr.x62;
    (*out)->x79 = frm->picatr.x63;
    (*out)->x7a = frm->picatr.x64;
}

/* Takes back a frame handed out by SFMPV_GetRead once the application has displayed it. With frame
 * ids the frame is found by id, otherwise it must be the one handed out last. */
Sint32 SFMPV_AddRead(SFD sfd, SFD_VFRM_INF *inf, Sint32 id) {
    Sint32 cs;
    Sint32 ret;
    SFMPV_WORK *wk;
    SFMPV_FRM *frm;
    SFD_VFRM *vfrm;

    SFLIB_LockCs(&cs);
    wk = SFMPV_WK(sfd);
    if (sfd->x58 == 2) {
        frm = fn_803F8FC0(sfd, id);
        if (frm == 0) {
            ret = SFLIB_SetErr(sfd, SFMPV_ERR_FRM_ID);
            goto done;
        }
        vfrm = SFMPVF_SearchVfrmData(sfd, frm);
    } else {
        vfrm = (SFD_VFRM *)((Uint8 *)inf - offsetof(SFD_VFRM, inf));
        if (vfrm->used != 1) {
            ret = SFLIB_SetErr(sfd, SFMPV_ERR_FRM_UNUSED);
            goto done;
        }
        frm = SFMPVF_SearchFrmObj(sfd, inf);
        if (wk->curfrm != frm) {
            ret = SFLIB_SetErr(sfd, SFMPV_ERR_FRM_NOT_CURRENT);
            goto done;
        }
    }
    vfrm->used = 0;
    SFMPVF_EndDrawFrm(frm);
    ret = 0;
done:
    SFLIB_UnlockCs(&cs);
    return ret;
}

/* Decodes the header data kept by the seek analysis; sets *analysed when it did. */
static inline Sint32 sfmpv_AnalyseSeekHdr(SFD sfd, Bool *analysed) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    SFSEE_VHDR *hdr;
    MPV hn;
    SJCK ck;
    Sint32 used;

    *analysed = FALSE;
    hdr = sfmpv_GetVhdr(sfd);
    if (hdr == 0) {
        return 0;
    }
    if (hdr->analyzed == 0) {
        return 0;
    }
    hn = wk->mpv;
    SFD_TIM(sfd)->ttu0 = hdr->ttu;
    ck.data = hdr->hdr;
    ck.len = hdr->hdrlen;
    if (MPV_DecodePicAtr(hn, &ck, &used) != 0) {
        return SFLIB_SetErr(sfd, SFMPV_ERR_PICATR);
    }
    *analysed = TRUE;
    return 0;
}

/* Prepares the decoder for a seek: decodes the header kept by the seek analysis and resets the
 * picture state and the delimiter mask. */
Sint32 SFMPV_Seek(SFD sfd) {
    SFMPV_WORK *wk = SFMPV_WK(sfd);
    Bool analysed;
    Sint32 err;

    err = sfmpv_AnalyseSeekHdr(sfd, &analysed);
    if (err != 0) {
        return err;
    }
    wk->picstat = 2;
    if (!analysed || SFSET_GetCond(sfd, SFD_COND_SEEK_GOP_DLM) == 0) {
        wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ;
    } else {
        wk->dlmmask = MPV_DLM_END | MPV_DLM_SEQ | MPV_DLM_GOP;
    }
    return 0;
}
