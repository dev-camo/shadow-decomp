/* CRI Sofdec MW player decoded frames (mwsfdfrm.c): the Sofdec header callback, the table of what
 * the headers said about the video, and mwPlyGetCurFrm / mwPlyRelCurFrm, which convert the
 * decoder's frame into the application's.
 *
 * Names follow the RE4 GameCube decompilation of the same file. This version differs from it: the
 * application's frame record has an extra field (x04), and the picture user data search and the
 * frame/field decision are part of the frame conversion.
 *
 * The original object also held player functions the linker removed. Their error texts remain
 * in .rodata between the surviving functions' texts, which fixes the order of the definitions
 * below. mwPlyIsNextFrmReady and mwsffrm_DecideFrmType are real functions reconstructed from the
 * code inlined into their callers; the stand-ins are marked.
 */
#include <string.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/sfh.h"
#include "CRI/sud.h"
#include "CRI/uty.h"

/* stream id of the video element stream in Sofdec headers */
#define MWSFFRM_VID_STMID 0xE0

/* size of the header block the header callback needs to analyse */
#define MWSFFRM_SFH_SIZE 0x800

/* Matching workaround: stand-in for a function of the original file that the linker removed:
 * mwPlyGetNumRemainFrm. Only its error text is known (the literal stays in .rodata at this
 * position); the body is not the original's. Never called. The same holds for each stand-in
 * below. Revisit when the stripped code is known. */
static void mwsffrm_StrippedNumRemainFrm(void) {
    MWSFSVM_Error("E409171: mwPlyGetNumRemainFrm: handle is invalid.");
}

/* Sofdec header callback (SFD_COND_HDRCB_FN): records the colour type, frame count and component
 * layout of the video the header describes.
 *
 * Not matching yet: the code is identical except that the handle and the Sofdec header object
 * exchange registers (17 differing words). */
static void mwsffrm_AnalySofdecHeader(void *obj, void *data, Sint32 size) {
    MWPLY mwply = obj;
    SFH sfh;
    Sint32 issfd;
    Sint32 coltype;
    Sint32 exist;
    Sint32 num;
    Sint32 type;
    Sint32 ccs;
    Sint32 maxfrm;
    Sint32 fxtype;
    Sint32 wr;

    mwply->sfh_cnt = mwply->sfh_cnt + 1;
    if (size < MWSFFRM_SFH_SIZE || data == NULL) {
        return;
    }
    sfh = SFH_Create(data, size);
    if (sfh == NULL) {
        return;
    }
    if (SFH_IsSfdHeader(sfh, &issfd) != TRUE || issfd != TRUE) {
        SFH_Destroy(sfh);
        return;
    }

    ccs = 0;
    if (SFH_IsExistStmId(sfh, MWSFFRM_VID_STMID, &exist) != TRUE || exist != TRUE) {
        ccs = 0;
    } else if (SFH_AnlyFtrColType(sfh, MWSFFRM_VID_STMID, &coltype) != TRUE) {
        ccs = 0;
    } else if (coltype == 3) {
        ccs = 1;
    }

    if (SFH_AnlyMaxFrmNum(sfh, &num) != TRUE) {
        maxfrm = -1;
    } else {
        maxfrm = num;
    }

    if (SFH_AnlyFtrFxType(sfh, MWSFFRM_VID_STMID, &type) != TRUE) {
        fxtype = SFX_COMPO_YCC420PLN;
    } else {
        switch (type) {
        case 1:
            fxtype = SFX_COMPO_YCC420PLN_UPHALF;
            break;
        case 3:
            fxtype = SFX_COMPO_0x51;
            break;
        case 6:
            fxtype = SFX_COMPO_0x61;
            break;
        case 0:
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:
        default:
            fxtype = SFX_COMPO_YCC420PLN;
            break;
        }
    }

    wr = mwply->sfh_wr;
    mwply->sfhinf[wr].no = mwply->sfh_cnt - 1;
    mwply->sfhinf[wr].ccs = ccs;
    mwply->sfhinf[wr].maxfrm = maxfrm;
    mwply->sfhinf[wr].fxtype = fxtype;
    mwply->sfhinf[wr].valid = 1;
    mwply->sfh_wr = mwply->sfh_wr + 1;
    mwply->sfh_wr = mwply->sfh_wr % MWSFFRM_SFHINF_NUM;
    SFH_Destroy(sfh);
}

/* Installs the Sofdec header callback on the player's SFD handle. */
void MWSFFRM_SetShfCbFn(MWPLY mwply) {
    SFD sfd = mwply->sfd;

    SFD_SetCond(sfd, SFD_COND_HDRCB_FN, (Sint32)mwsffrm_AnalySofdecHeader);
    SFD_SetCond(sfd, SFD_COND_HDRCB_OBJ, (Sint32)mwply);
}

/* Empties the Sofdec header table; every slot starts with the default component layout. */
void MWSFFRM_InitSfhInfTable(MWPLY mwply) {
    Sint32 i;

    mwply->sfh_cnt = 0;
    mwply->sfh_cur = 0;
    mwply->sfh_wr = 0;
    for (i = 0; i < MWSFFRM_SFHINF_NUM; i++) {
        mwply->sfhinf[i].valid = 0;
        mwply->sfhinf[i].no = 0;
        mwply->sfhinf[i].ccs = 0;
        mwply->sfhinf[i].maxfrm = 0;
        mwply->sfhinf[i].fxtype = SFX_COMPO_YCC420PLN;
    }
}

/* Matching workaround: stand-ins, as above, for mwPlyGetNumSkipDisp and mwPlyGetNextPicUsr. */
static void mwsffrm_StrippedNumSkipDisp(void) {
    MWSFSVM_Error("E202231: mwPlyGetNumSkipDisp: handle is invalid.");
}

static void mwsffrm_StrippedNextPicUsr(void) {
    MWSFSVM_Error("E3122201: mwPlyGetNextPicUsr: handle is invalid.");
}

/* Whether the decoder has another frame ready; the frame skipping of mwPlyGetCurFrm asks it. */
Sint32 mwPlyIsNextFrmReady(MWPLY mwply) {
    SFD sfd;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122618: mwPlyIsNextFrmReady: handle is invalid.");
        return 0;
    }
    sfd = mwPlyGetSfdHn(mwply);
    if (sfd == NULL) {
        return 0;
    }
    return SFD_IsNextFrmReady(sfd);
}

/* Gives the frame mwPlyGetCurFrm handed out back to the decoder. */
void mwPlyRelCurFrm(MWPLY mwply) {
    SFD_VFRM_INF *frm;
    Sint32 ngetfrm;
    Sint32 nrelfrm;
    SFD sfd;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122615: mwPlyRelCurFrm: handle is invalid.");
        return;
    }
    frm = mwply->curfrm;
    ngetfrm = mwply->ngetfrm;
    nrelfrm = mwply->nrelfrm;
    if (mwply->frmapi == 0) {
        mwply->frmapi = 1;
    } else if (mwply->frmapi != 1) {
        MWSFSVM_Error("E4110902: Don't use another type get/rel frame API together.");
    }
    sfd = mwPlyGetSfdHn(mwply);
    if (ngetfrm > nrelfrm) {
        SFD_RelFrm(sfd, frm);
        mwply->nrelfrm++;
        mwply->ngetfrm = mwply->nrelfrm;
    }
}

/* Matching workaround: stand-ins, as above, for mwPlyRelFrm and mwl_convPtypeToSFD. */
static void mwsffrm_StrippedRelFrm(void) {
    MWSFSVM_Error("E410132: mwPlyRelFrm: handle is invalid.");
}

static void mwsffrm_StrippedConvPtypeToSFD(void) {
    MWSFSVM_Error("mwl_convPtypeToSFD : Invalid Ptype");
}

/* Plane addresses and strides of a planar YCC 4:2:0 frame buffer of the given size. */
void mwPlyCalcYccPlane(void *buf, Sint32 width, Sint32 height, MWSFD_YCCPLN *out) {
    MPV_RFB rfb;

    SFD_CalcYccPlane(buf, width, height, &rfb);
    out->y = rfb.pln[2];
    out->cb = rfb.pln[0];
    out->cr = rfb.pln[1];
    out->ywidth = rfb.ypitch;
    out->cbwidth = rfb.cpitch;
    out->crwidth = rfb.cpitch;
}

/* Frame type from the picture structure (SFD_VFRM_INF.x58): 2 for field pictures and for frame
 * pictures with x6c 0, else 0. Not declared inline: as a static inline function its error text is
 * pooled after mwl_convPtypeFromSFD's. */
static void mwsffrm_DecideFrmType(SFD_VFRM_INF *vfrm, Sint32 *ftype) {
    *ftype = 0;
    switch (vfrm->x58) {
    case 3:
        if (vfrm->x6c == 0) {
            *ftype = 2;
        }
        break;
    case 1:
    case 2:
        *ftype = 2;
        break;
    case 0:
    default:
        MWSFSVM_Error("E301271: mwsffrm_DecideFrmType() : Invalid Pstruct");
        break;
    }
}

/* Buffer format (SFD_VFRM_INF.x1c to MWS_FRM.fmt, MwsfdBufFmt): 1, 2 or 3; everything else is 3.
 * Helper and name inferred, after the same helper in the RE4 decompilation and by analogy with
 * mwl_convPtypeFromSFD. */
static inline Sint32 mwl_convFtypeFromSFD(Sint32 ftype) {
    Sint32 ret;

    switch (ftype) {
    case 1:
        ret = 1;
        break;
    case 2:
        ret = 2;
        break;
    case 3:
        ret = 3;
        break;
    default:
        ret = 3;
        break;
    }
    return ret;
}

/* Picture type (SFD_VFRM_INF.pic_type to MWS_FRM.ptype): 1 to 4; others are reported and 1. */
static inline Sint32 mwl_convPtypeFromSFD(Sint32 ptype) {
    Sint32 ret;

    switch (ptype) {
    case 1:
        ret = 1;
        break;
    case 2:
        ret = 2;
        break;
    case 3:
        ret = 3;
        break;
    case 4:
        ret = 4;
        break;
    default:
        MWSFSVM_Error("mwl_convPtypeFromSFD : Invalid Ptype");
        ret = 1;
        break;
    }
    return ret;
}

/* Converts the decoder's frame record into the application's, including its picture user data and
 * the frame/field decision. */
static void mwl_convFrmInfFromSFD(MWPLY mwply, SFD_VFRM_INF *vfrm, MWS_FRM *frm) {
    Sint32 time;
    Sint32 ftype;
    Sint32 ptype;
    SFD sfd;
    Sint32 scale;
    void *bufadr;
    Sint32 fps;
    Sint32 time2;
    MWSFD_PICUSR *pptr;
    MWSFD_PICUSR *noptr;
    Sint32 usrlen;
    Uint8 *usrptr;
    SFMPV_PICUSR *usr;
    Sint32 sulen;
    Uint8 *sudat;
    Uint8 *dat;
    Sint32 len;
    Sint32 fmt;
    Sint32 width;
    Sint32 height;
    Sint32 mb_width;
    Sint32 mb_height;
    Sint32 x34;
    Sint32 tunit;
    Sint32 x30;
    Sint32 x24;
    Sint32 x28;
    Sint32 gopno;

    sfd = mwply->sfd;
    bufadr = vfrm->buf;
    fmt = mwl_convFtypeFromSFD(vfrm->x1c);
    width = vfrm->width;
    height = vfrm->height;
    mb_width = vfrm->mb_width;
    mb_height = vfrm->mb_height;
    ptype = mwl_convPtypeFromSFD(vfrm->pic_type);
    x34 = vfrm->x34;
    tunit = vfrm->tunit;
    x30 = vfrm->x30;
    gopno = vfrm->gopno;
    x24 = vfrm->x24;
    x28 = vfrm->x28;
    if (SFD_GetFps(sfd, &fps) != 0) {
        MWSFSVM_Error("E201301: MWSFD: GetFps failed.");
    }
    scale = tunit * 1000;
    time = UTY_MulDiv(x34, fps, scale);
    time2 = UTY_MulDiv(x30, fps, scale);
    noptr = &mwply->picusr;
    frm->bufadr = bufadr;
    frm->fmt = fmt;
    frm->width = width;
    frm->height = height;
    frm->mb_width = mb_width;
    frm->mb_height = mb_height;
    frm->ptype = ptype;
    frm->fps = fps;
    frm->time = time;
    frm->x34 = x34;
    frm->tunit = tunit;
    frm->gopno = gopno;
    frm->time2 = time2;
    frm->x30 = x30;
    frm->x3c = x24;
    frm->x40 = x28;

    pptr = mwply->picusr_ptr;
    usrptr = vfrm->picusr->buf;
    usrlen = vfrm->picusr->len;
    if (MWSFD_GetUsePicUsr() != 1) {
        frm->usrdat = NULL;
        frm->usrlen = 0;
    } else if (pptr == noptr) {
        frm->usrdat = NULL;
        frm->usrlen = 0;
    } else {
        if (usrptr != NULL && usrlen > 4) {
            usrptr += 4;
            usrlen -= 4;
        }
        frm->usrdat = usrptr;
        frm->usrlen = usrlen;
    }

    sudat = NULL;
    sulen = 0;
    usr = vfrm->picusr;
    usrptr = usr->buf;
    usrlen = usr->len;
    frm->sudat = NULL;
    frm->sulen = 0;
    if (usrptr != NULL && usrlen > 4) {
        SUD_SearchSudDat(usrptr + 4, usrlen - 4, (void **)&sudat, &sulen);
    }
    if (sudat != NULL && sulen > 0) {
        frm->sudat = sudat;
        frm->sulen = sulen;
    }

    len = frm->sulen;
    dat = frm->sudat;
    /* Fields by the picture structure, or when the SUD record asks for field division. */
    mwsffrm_DecideFrmType(vfrm, &ftype);
    if (MWSFD_GetUsePicUsr() == 1 && SUD_AnalyTypeDivField(dat, len) == 1) {
        ftype = 2;
    }
    frm->ftype = ftype;
    memcpy(frm->ext, &vfrm->x48, sizeof(frm->ext));
}

/* Hands out the next decoded frame (bufadr NULL when there is none), first skipping frames that
 * are already late.
 *
 * Matching workaround: the handles are taken as void * and converted to typed locals, as in
 * mwPlySetOutVol. With typed parameters the handle and the frame record exchange registers
 * throughout the function (38 differing words). Revisit when the original public prototype is
 * known. */
void mwPlyGetCurFrm(void *obj, void *frmobj) {
    MWPLY mwply = obj;
    MWS_FRM *frm = frmobj;
    SFD sfd;
    void *vfrm;
    SFD_VFRM_INF *inf;
    Sint32 i;
    Sint32 nskip;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122614: mwPlyGetCurFrm: handle is invalid.");
        memset(frm, 0, sizeof(MWS_FRM));
        frm->bufadr = NULL;
        return;
    }
    if (mwply->frmapi == 0) {
        mwply->frmapi = 1;
    } else if (mwply->frmapi != 1) {
        MWSFSVM_Error("E4110902: Don't use another type get/rel frame API together.");
    }
    sfd = mwPlyGetSfdHn(mwply);
    if (sfd == NULL) {
        memset(frm, 0, sizeof(MWS_FRM));
        frm->bufadr = NULL;
        return;
    }

    SFD_GetFrm(sfd, &vfrm);
    if (vfrm != NULL && mwply->noskip == 0) {
        nskip = mwply->prm.nfrm_pool_wk;
        for (i = 0; i < nskip; i++) {
            if (mwPlyIsNextFrmReady(mwply) != 1) {
                break;
            }
            SFD_RelFrm(sfd, vfrm);
            mwply->nskipdisp++;
            SFD_GetFrm(sfd, &vfrm);
        }
    }

    if (vfrm != NULL) {
        mwply->ngetfrm++;
        mwply->curfrm = vfrm;
        /* Matching workaround: the frame is read through a typed copy of the void * that SFD_GetFrm
         * fills. With vfrm typed SFD_VFRM_INF * (passed as (void **)&vfrm) the function is 0x238
         * bytes instead of 0x220 and 48 words differ. Revisit with the original SFD_GetFrm
         * prototype. */
        inf = vfrm;
        mwply->pic_struct = inf->x58;
        mwply->chroma_format = inf->x5c;
        mwply->x90 = inf->x6c;
        mwply->x94 = inf->x6d;
        mwply->x98 = inf->x6e;
        mwply->chromapos_h = inf->x3c;
        mwply->chromapos_v = inf->x40;
        mwply->xa4 = 0;
        mwl_convFrmInfFromSFD(mwply, vfrm, frm);
        frm->x04 = frm->time;
        mwply->sfh_cur = frm->gopno;
        if (mwply->tag_no < frm->gopno) {
            MWSFTAG_UpdateTagInf(mwply);
        }
        mwply->tag_no = frm->gopno;
    } else {
        frm->bufadr = NULL;
    }
}

/* Matching workaround: stand-ins, as above, for mwPlyGetFrm, mwPlyGetFrmSync and
 * mwPlySetFrmSync. */
static void mwsffrm_StrippedGetFrm(void) {
    MWSFSVM_Error("E410131: mwPlyGetFrm: handle is invalid.");
}

static void mwsffrm_StrippedGetFrmSync(void) {
    MWSFSVM_Error("E2010801: mwPlyGetFrmSync: handle is invalid.");
}

static void mwsffrm_StrippedSetFrmSync(void) {
    MWSFSVM_Error("E1122629: mwPlySetFrmSync: handle is invalid.");
}
