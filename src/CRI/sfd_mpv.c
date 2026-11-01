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
 * their order in .text and the variables in the reverse of their order in .bss.
 *
 * Not yet written: the decoding functions from sfmpv_GoDdelim to sfmpv_ExecServerSub and the layout
 * check sfmpv_ChkFatal. */
#include <stddef.h>
#include <string.h>

#include "CRI/mpv.h"
#include "CRI/sfd.h"
#include "CRI/uty.h"

#define SFMPV_WK(sfd) ((SFMPV_WORK *)(sfd)->tr[SFMPV_TR].hn)

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
#define SFMPV_ERR_FRM_ID 0xFF000F1F /* no frame has this id (released twice) */
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

/* Not written yet. */
Sint32 sfmpv_ChkFatal(void);
Sint32 sfmpv_ExecServerSub(SFD sfd);

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
Sint32 SFMPV_Seek(SFD sfd);
void sfmpv_ErrFn(void *obj, Sint32 code);
Sint32 sfmpv_InitInf(SFD sfd, SFMPV_WORK *wk);

const SFD_TR_IF SFD_tr_vd_mpv = {
    SFMPV_Init,     SFMPV_Finish,  SFMPV_ExecServer, SFMPV_Create, SFMPV_Destroy,
    SFMPV_Standby,  SFMPV_Start,   SFMPV_Stop,       SFMPV_Pause,  SFMPV_GetWrite,
    SFMPV_AddWrite, SFMPV_GetRead, SFMPV_AddRead,    SFMPV_Seek,
};

/* Rounded frame rate for each MPEG frame_rate_code (1 23.976 ... 8 60 Hz; inferred from the
 * values, the users of the table are not written yet). */
const Sint32 sfmpv_fps_round[9] = {0, 24, 24, 25, 30, 30, 50, 60, 60};
/* Drop-frame time code constants for 29.97 and 59.94 Hz: frames per hour, per ten minutes, in the
 * first minute of ten and in the other minutes, then four further constants. */
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
void SFD_SetMpvParaTbl(SFMPV_PARA *para, Uint32 *rfb, Uint32 *ta) {
    Sint32 i;

    sfmpv_para = *para;
    sfmpv_para.x10 = 0;
    sfmpv_para.x20 = 0;
    sfmpv_rfb_adr_tbl[0] = (void *)((rfb[0] + 31) & ~31);
    sfmpv_rfb_adr_tbl[1] = (void *)((rfb[1] + 31) & ~31);
    for (i = 0; i < SFMPV_FRM_NUM; i++) {
        if (i < para->nfrm) {
            sfmpv_ta_adr_tbl[i] = (void *)((ta[i] + 31) & ~31);
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

/* Lays out a frame buffer at buf as the decoder wants it: the luma plane of the picture size
 * rounded up to whole macroblocks, then the two chroma planes at half width and height. The rows
 * are padded to 32 bytes. */
void SFD_CalcYccPlane(void *buf, Sint32 width, Sint32 height, MPV_RFB *rfb) {
    Sint32 w16 = (width + 15) / 16 * 16;
    Sint32 ypitch = (w16 + 31) / 32 * 32;
    Sint32 cpitch = (w16 / 2 + 31) / 32 * 32;
    Sint32 h16 = (height + 15) / 16 * 16;

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

/* Brings up the MPV library on its work area and clears the saved resources. A failed layout
 * check hangs the system. */
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

Sint32 SFMPV_Finish(void) {
    MPV_Finish();
    return 0;
}

/* Runs the decoder step of the driver server. */
Sint32 SFMPV_ExecServer(SFD sfd) {
    return sfmpv_ExecServerSub(sfd);
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
 * the decode time and the error counts to the statistics and, if data was consumed, completes the
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
    Sint32 ref_ndct;
    Sint32 ref_nbyte;
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
            ref_ndct = wk->ref[0]->ndct;
            ref_nbyte = wk->ref[0]->nbyte;
            break;
        case 3:
            ref_ndct = wk->ref[0]->ndct + wk->ref[1]->ndct;
            ref_nbyte = wk->ref[0]->nbyte + wk->ref[1]->nbyte;
            break;
        default:
            ref_ndct = 0;
            ref_nbyte = 0;
            break;
        }
    } else {
        ref_ndct = 0;
        ref_nbyte = 0;
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
        frm->tunit = frm->ttu.unit;
        frm->ftime = tim->ctime + (frm->ttu.val - vofst->val);
        frm->x4c = frm->ttu.val;
        frm->x50 = frm->ttu.val + tim->ctime;
        if (tim->x284 < frm->ftime) {
            tim->x284 = frm->ftime;
            tim->x288 = frm->tunit;
        }
        frm->gopno = wk->nconcat;
        frm->ndct = ref_ndct + wk->tmpref_adj + para.x28;
        frm->nbyte = para.x2c + ref_nbyte;
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
        SFPLY_AddDecPic(sfd, 1, (void *)picatr->pic_type);
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
    Sint32 w16;
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
        w16 = (picatr->width + 15) / 16 * 16;
        ypitch = (w16 + 31) / 32 * 32;
        cpitch = (w16 / 2 + 31) / 32 * 32;
        h16 = (picatr->height + 15) / 16 * 16;
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

/* Finds the first start code of the classes in mask in data and reads its class (the class is not
 * used). Returns its address, 0 when there is none. */
static inline Sint8 *sfmpv_FindDelim(Sint8 *data, Sint32 len, Sint32 mask) {
    Sint8 *delim = MPV_SearchDelim(data, len, mask);

    if (delim != 0) {
        MPV_CheckDelim(delim);
        return delim;
    }
    return 0;
}

/* Discards the video input up to the next start code of the classes in mask (the start code
 * itself is kept) and counts the bytes: all of them in v_byte, and in v_skip as well when the
 * discarded bytes (the first three, or all of them when fewer) were not all zero. The
 * ring buffer data is two chunks, so a start code can straddle them; when none is found the last
 * three bytes stay, as they may begin one. Returns the number of bytes consumed. */
Sint32 sfmpv_GoDdelim(SFD sfd, SJ sj, Sint32 mask) {
    SFBUF_RINF inf;
    Sint8 edge[6];
    Sint8 *delim;
    Sint8 *q;
    Sint32 head;
    Sint32 tail;
    Sint32 nbyte;
    Sint32 i;
    Sint32 len1;
    Sint8 *src;
    Sint8 *data2;
    Bool nonzero;

    if (SFBUF_RingGetRead(sfd, sfd->tr[SFMPV_TR].bufin, &inf) != 0) {
        return 0;
    }
    if (inf.ck1.len == 0) {
        return 0;
    }
    delim = MPV_SearchDelim((Sint8 *)inf.ck1.data, inf.ck1.len, mask);
    if (delim != 0) {
        /* the class of the start code found is not used, but the original calls this here */
        MPV_CheckDelim(delim);
    } else if (inf.ck2.len == 0) {
        /* Matching workaround: delim is already 0 here, but the original clears it again (the li
         * at offset 0xE00 of the original object); without this assignment the function measures
         * 142 instead of 74 differing words. Revisit when the search's original structure is
         * found. */
        delim = 0;
    } else {
        /* look for a start code across the join of the two chunks */
        head = inf.ck1.len < 3 ? inf.ck1.len : 3;
        tail = inf.ck2.len < 3 ? inf.ck2.len : 3;
        memcpy(edge, inf.ck1.data + inf.ck1.len - head, head);
        memcpy(edge + head, inf.ck2.data, tail);
        q = edge;
        for (i = 0; i < head + tail - 3; i++, q++) {
            if (MPV_CheckDelim(q) & mask) {
                delim = (Sint8 *)inf.ck1.data + inf.ck1.len - head + i;
                goto found;
            }
        }
        delim = sfmpv_FindDelim((Sint8 *)inf.ck2.data, inf.ck2.len, mask);
    }
found:
    if (delim == 0) {
        nbyte = inf.ck1.len + inf.ck2.len - 3;
        nbyte = nbyte > 0 ? nbyte : 0;
    } else if ((Sint8 *)inf.ck1.data <= delim && delim < (Sint8 *)inf.ck1.data + inf.ck1.len) {
        nbyte = delim - (Sint8 *)inf.ck1.data;
    } else if ((Sint8 *)inf.ck2.data <= delim && delim < (Sint8 *)inf.ck2.data + inf.ck2.len) {
        nbyte = inf.ck1.len + (delim - (Sint8 *)inf.ck2.data);
    } else {
        nbyte = 0;
    }
    SFBUF_RingAddRead(sfd, sfd->tr[SFMPV_TR].bufin, nbyte);

    src = (Sint8 *)inf.ck1.data;
    len1 = inf.ck1.len;
    data2 = (Sint8 *)inf.ck2.data;
    nonzero = FALSE;
    for (i = 0; i < (nbyte < 3 ? nbyte : 3); i++, src++) {
        Sint8 *byte = i < len1 ? src : data2 + (i - len1);

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
    if (para->x10 == 0 || para->x20 == 0) {
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
    Sint32 i;

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
    for (i = 0; i < SFMPV_FRM_NUM; i++) {
        SFMPV_FRM *frm = &wk->frm[i];

        frm->stat = SFMPV_FRM_FREE;
        frm->lock = 0;
        SFTIM_InitTtu(&frm->ttu, 0);
        frm->buf = wk->ta_adr[i];
        frm->ftime = 0;
        frm->tunit = 1;
        frm->ndct = 0;
        frm->nbyte = 0;
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
    (*out)->ndct = frm->ndct;
    (*out)->nbyte = frm->nbyte;
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
    SFMPV_WORK *wk;
    SFMPV_FRM *frm;
    SFD_VFRM *vfrm;
    Sint32 ret;

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
