/* CRI Sofdec MW player frame converter front end (mwsfdsfx.c): the conversion of a player frame
 * into the converter's source frame, the additional-info (tag) stream, and the converter's
 * creation and settings.
 *
 * Names follow the RE4 GameCube decompilation of the same file. This version differs from it: the
 * file starts with a generic conversion helper (fn_80403520) and its whole-frame entry
 * (fn_804035A8), the frame conversion reads the Sofdec header table itself, and several of RE4's
 * accessors are absent.
 *
 * The original file continues after 0x80403E44 (USA) in two units of their own,
 * Game/fn_80403E44.c (fn_80403E44) and CRI/fn_80403E4C.c (MWSFSFX_Finish, mwsfsfx_SfxErrCbFn and
 * MWSFSFX_Init), which match; they stay separate until this unit matches too.
 *
 * The original object also held player functions the linker removed. Their error texts remain in
 * .rodata between the surviving functions' texts, which fixes the order of the definitions below;
 * the stand-ins that reproduce them are marked.
 */
#include <string.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/sud.h"

/* Converts the part (x, y, width, height) of the player frame into buf with the converter cnvfn
 * (registered with SFX_SetCnvFn first), the way the converter library's frame conversion
 * entries do. */
void fn_80403520(MWPLY mwply, MWS_FRM *frm, Uint8 *buf, SFX_CNVFN cnvfn, Sint32 x, Sint32 y,
                 Sint32 width, Sint32 height) {
    SFX_SRCFRM src;
    SFX_FRM dst;
    SFX_OBJ *sfx;

    sfx = mwply->sfx;
    MWSFSFX_CnvFrmInfToSfx(mwply, frm, &src);
    sfxcnv_MakeDstFrm(sfx, &src, buf, &dst, x, y, width, height);
    SFX_SetCnvFn(sfx, cnvfn);
    sfxcnv_ChkCompoByCbFunc(sfx, &src, &dst);
}

/* Converts the whole player frame into buf with cnvfn. */
void fn_804035A8(MWPLY mwply, MWS_FRM *frm, Uint8 *buf, SFX_CNVFN cnvfn) {
    fn_80403520(mwply, frm, buf, cnvfn, 0, 0, frm->width, frm->height);
}

/* Matching workaround: stand-ins for functions of the original file that the linker removed
 * (mwPlyFxGetCompoMode, mwPlyGetCnvBottomUp, mwPlySetCnvBottomUp, MWSFD_MakeTblZ32 and
 * MWSFD_MakeTblZ16, mwPlyAttachAddInfBuf, then mwPlyFxGetOutZoffset, mwPlyFxSetOutZoffset,
 * mwPlyFxGetOutZscale, mwPlyFxSetOutZscale, mwPlyFxGetOutBufPitchHeight, mwPlyFxSetCompoMode,
 * mwPlyFxCnvFrmZ32 and mwPlyFxCnvFrmZ16). Only their error texts are known, from the literals
 * that stay in .rodata at their positions; the bodies are not the original's. Never called.
 * Revisit when the stripped code is known. */
static void mwsfsfx_StrippedFxGetCompoMode(void) {
    MWSFSVM_Error("E2011915: mwPlyFxGetCompoMode: handle is invalid.");
}

static void mwsfsfx_StrippedGetCnvBottomUp(void) {
    MWSFSVM_Error("E404011: mwPlyGetCnvBottomUp: handle is invalid.");
}

static void mwsfsfx_StrippedSetCnvBottomUp(void) {
    MWSFSVM_Error("E404010: mwPlySetCnvBottomUp: handle is invalid.");
}

static void mwsfsfx_StrippedMakeTblZ32(void) {
    MWSFSVM_Error("E202285: MWSFD_MakeTblZ32: handle is invalid.");
    MWSFSVM_Error("E202286: MWSFD_MakeTblZ32: getfrm is failed.");
}

static void mwsfsfx_StrippedMakeTblZ16(void) {
    MWSFSVM_Error("E202283: MWSFD_MakeTblZ16: handle is invalid.");
    MWSFSVM_Error("E202284: MWSFD_MakeTblZ16: getfrm is failed.");
}

static void mwsfsfx_StrippedAttachAddInfBuf(SJCK *ck) {
    SJCK out;

    SJ_SearchTag(ck, "SFXINFS", "SFXINFE", &out);
    MWSFSVM_Error("W2121001 : mwPlyAttachAddInfBuf(): bufsize is short.");
}

/* MPEG-only files (mode 2) have no additional-info stream. Helper and name inferred, after the same
 * helper in the RE4 decompilation. */
static inline Bool mwsftag_IsNoAinf(MWPLY mwply) {
    return mwply->prm.mode == 2;
}

/* Reads the tag block ("CRITAGS" to "CRITAGE") out of the additional-info stream. */
static void mwsftag_GetAinfFromSj(MWPLY mwply) {
    SJCK ck;
    SJCK out;
    SJCK rd;
    SJ sj;
    Sint32 ndata;

    sj = mwply->ainf_sj;
    ndata = SJ_GetNumData(sj, SJ_CK_DATA);
    if (ndata == 0) {
        mwply->tag_ptr = NULL;
        mwply->tag_size = 0;
        mwply->tag_flg = 1;
        return;
    }
    ck.data = mwply->ainf_buf;
    ck.len = ndata;
    if (SJ_SearchTag(&ck, "CRITAGS", "CRITAGE", &out) == NULL) {
        mwply->tag_ptr = NULL;
        mwply->tag_size = 0;
        mwply->tag_flg = 1;
        return;
    }
    if (mwply->addinf_buf != NULL) {
        memcpy(mwply->addinf_buf, out.data, out.len);
        mwply->tag_ptr = mwply->addinf_buf;
        mwply->tag_size = out.len;
        mwply->tag_flg = 1;
        SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &rd);
        SJ_PutChunk(sj, SJ_CK_FREE, &rd);
        SJ_Reset(sj);
    } else {
        mwply->tag_ptr = out.data;
        mwply->tag_size = out.len;
        mwply->tag_flg = 1;
        if (mwsftag_IsNoAinf(mwply) != TRUE && mwply->ainf_sj != NULL) {
            SFD_SetUsrSj(mwply->sfd, 2, NULL, NULL);
        }
    }
}

/* Hands the tag record the converter needs ("SFXINFS" to "SFXINFE" in the tag block) to the
 * converter, after reading a new tag block from the additional-info stream. */
void MWSFTAG_UpdateTagInf(MWPLY mwply) {
    SJCK out;
    SJCK ck;
    SFX_OBJ *sfx;

    if (mwply->ainf_sj == NULL) {
        return;
    }
    mwsftag_GetAinfFromSj(mwply);
    sfx = mwply->sfx;
    if (mwply->tag_ptr == NULL) {
        SFX_SetTagInf(sfx, NULL, 0);
        return;
    }
    ck.data = mwply->tag_ptr;
    ck.len = mwply->tag_size;
    if (SJ_SearchTag(&ck, "SFXINFS", "SFXINFE", &out) == NULL) {
        SFX_SetTagInf(sfx, NULL, 0);
        return;
    }
    SFX_SetTagInf(sfx, out.data, out.len);
}

/* Empties the additional-info stream joint (new play). */
void MWSFTAG_ResetAinfSj(MWPLY mwply) {
    if (mwply->ainf_sj != NULL) {
        SJ_Reset(mwply->ainf_sj);
    }
}

/* No tag block seen yet. */
void MWSFTAG_InitTagInf(MWPLY mwply) {
    mwply->tag_flg = 0;
    mwply->tag_ptr = NULL;
    mwply->tag_size = 0;
    mwply->tag_no = -1;
}

/* Registers the additional-info stream joint as SFD user output channel 2; 0, or -1 when
 * SFD_SetUsrSj fails. Files of mode 2 (MPEG video only) have no such stream. */
Sint32 MWSFTAG_SetAinfSj(MWPLY mwply) {
    if (mwsftag_IsNoAinf(mwply) == TRUE) {
        return 0;
    }
    if (mwply->ainf_sj == NULL) {
        return 0;
    }
    return (SFD_SetUsrSj(mwply->sfd, 2, mwply->ainf_sj, NULL) != 0) ? -1 : 0;
}

/* Destroys the additional-info stream joint. */
void MWSFTAG_DestroyAinfSj(MWPLY mwply) {
    if (mwply->ainf_sj != NULL) {
        SJ_Destroy(mwply->ainf_sj);
    }
}

/* Whether the creation parameters ask for the additional-info stream: component layout 0 or
 * 0x101. The original expands this test in MWSFTAG_CreateAinfSj, which comes before the public
 * MWSFTAG_IsUseAinfSj. */
static inline Bool mwsftag_IsUseAinfSj(MWSFD_CRPRM *prm) {
    if (prm->compo == 0 || prm->compo == 0x101) {
        return TRUE;
    }
    return FALSE;
}

/* Creates the additional-info stream joint over its buffer when the layout uses one (NULL
 * otherwise, and after destroying the player when the joint cannot be created). */
SJ MWSFTAG_CreateAinfSj(MWPLY mwply) {
    SJ sj;

    if (mwsftag_IsUseAinfSj(&mwply->prm) != TRUE) {
        return NULL;
    }
    sj = SJRBF_Create(mwply->ainf_buf, mwply->ainf_bsize, 0);
    if (sj == NULL) {
        MWSFSVM_Error("E201211 mwPlyCreate: can't create AddInfSJ");
        mwSfdDestroy(mwply);
        return NULL;
    }
    return sj;
}

Bool MWSFTAG_IsUseAinfSj(MWSFD_CRPRM *prm) {
    return mwsftag_IsUseAinfSj(prm);
}

/* Matching workaround: stand-ins, as above, for mwPlyFxGetOutZoffset, mwPlyFxSetOutZoffset,
 * mwPlyFxGetOutZscale and mwPlyFxSetOutZscale. */
static void mwsfsfx_StrippedFxGetOutZoffset(void) {
    MWSFSVM_Error("E2011921: mwPlyFxGetOutZoffset: handle is invalid.");
}

static void mwsfsfx_StrippedFxSetOutZoffset(void) {
    MWSFSVM_Error("E2011920: mwPlyFxSetOutZoffset: handle is invalid.");
}

static void mwsfsfx_StrippedFxGetOutZscale(void) {
    MWSFSVM_Error("E2011919: mwPlyFxGetOutZscale: handle is invalid.");
}

static void mwsfsfx_StrippedFxSetOutZscale(void) {
    MWSFSVM_Error("E2011918: mwPlyFxSetOutZscale: handle is invalid.");
}

/* Output texture size of the frame converters; the colour adjustment is reset. */
void MWSFSFX_SetOutBufSize(MWPLY mwply, Sint32 width, Sint32 height) {
    SFX_OBJ *sfx;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E306091: MWSFSFX_SetOutBufSize: handle is invalid.");
        return;
    }
    sfx = mwply->sfx;
    SFX_SetOutBufSize(sfx, width, height);
    SFX_SetColAdj(sfx, 0);
}

/* Matching workaround: stand-in, as above, for mwPlyFxGetOutBufPitchHeight. */
static void mwsfsfx_StrippedFxGetOutBufPitchHeight(void) {
    MWSFSVM_Error("E307092: mwPlyFxGetOutBufPitchHeight: handle is invalid.");
}

/* Sets the converter's component layout. */
void MWSFSFX_SetCompoMode(MWPLY mwply, Sint32 compo) {
    SFX_SetCompoMode(mwply->sfx, compo);
}

/* Matching workaround: stand-ins, as above, for mwPlyFxSetCompoMode, mwPlyFxCnvFrmZ32 and
 * mwPlyFxCnvFrmZ16. */
static void mwsfsfx_StrippedFxSetCompoMode(void) {
    MWSFSVM_Error("E201214: mwPlyFxSetCompoMode: handle is invalid.");
    MWSFSVM_Error("E204011: mwPlyFxSetCompoMode: COMPO_Z needs setting in MWPLY Creation.");
    MWSFSVM_Error("E204012: mwPlyFxSetCompoMode: COMPO_AUTO needs setting in MWPLY Creation.");
}

static void mwsfsfx_StrippedFxCnvFrmZ32(void) {
    MWSFSVM_Error("E2011913: mwPlyFxCnvFrmZ32: handle is invalid.");
    MWSFSVM_Error("E2011914: mwPlyFxCnvFrmZ32: getfrm is failed.");
}

static void mwsfsfx_StrippedFxCnvFrmZ16(void) {
    MWSFSVM_Error("E2011911: mwPlyFxCnvFrmZ16: handle is invalid.");
    MWSFSVM_Error("E2011912: mwPlyFxCnvFrmZ16: getfrm is failed.");
}

/* Fills one plane description of the converter's source frame. Helper and name inferred, after the
 * same helper in the RE4 decompilation. */
static inline void mwsfsfx_SetPln(SFX_SRCPLN *pln, Uint8 *buf, Sint32 pitch, Sint32 height) {
    pln->buf = buf;
    pln->pitch = pitch;
    pln->height = height;
}

/* Component layout of the frame's header (YCC 4:2:0 planar without a valid entry). The frame's
 * gopno (its header number) selects the Sofdec header table entry. Helper and name inferred. */
static inline Sint32 mwsfsfx_GetFrmFxType(MWPLY mwply, MWS_FRM *frm) {
    Sint32 idx = frm->gopno % MWSFFRM_SFHINF_NUM;
    Sint32 fxtype;

    if (mwply->sfhinf[idx].valid != 1) {
        fxtype = SFX_COMPO_YCC420PLN;
    } else {
        fxtype = mwply->sfhinf[idx].fxtype;
    }
    return fxtype;
}

/* Whether the frame's header asks for colour-space conversion. Helper and name inferred. */
static inline Bool mwsfsfx_IsFrmHdrCcs(MWPLY mwply, MWS_FRM *frm) {
    Sint32 idx = frm->gopno % MWSFFRM_SFHINF_NUM;
    Sint32 ccs;

    if (mwply->sfhinf[idx].valid != 1) {
        ccs = 0;
    } else {
        ccs = mwply->sfhinf[idx].ccs;
    }
    return ccs == 1;
}

/* Translates a player frame into the converter's source frame (planes, size, picture structure,
 * chroma format and positions, tag information and colour conversion flag) and, unless the
 * creation fixed it, takes the player's component layout from the frame's header; then programs
 * the converter with the player's component layout. */
void MWSFSFX_CnvFrmInfToSfx(MWPLY mwply, MWS_FRM *frm, SFX_SRCFRM *src) {
    MWSFD_YCCPLN ycc;
    Sint32 tag_len;
    Uint8 *tag_ptr;
    Sint32 val;
    Sint32 width;
    Sint32 height;
    Bool ccs;
    Sint32 compo;

    switch (frm->fmt) {
    case 1:
        val = 1;
        break;
    case 2:
        val = 2;
        break;
    case 3:
        val = 3;
        break;
    default:
        MWSFSVM_Error("E201184 : MwsfdBufFmt value is invalid.");
        val = 3;
        break;
    }
    src->frmfmt = val;
    width = frm->width;
    height = frm->height;
    src->width = width;
    src->height = height;
    if (frm->fmt != 3) {
        mwsfsfx_SetPln(&src->pln[0], frm->bufadr, width, height);
    } else {
        mwPlyCalcYccPlane(frm->bufadr, width, height, &ycc);
        mwsfsfx_SetPln(&src->pln[0], ycc.y, ycc.ywidth, height);
        mwsfsfx_SetPln(&src->pln[1], ycc.cb, ycc.cbwidth, height / 2);
        mwsfsfx_SetPln(&src->pln[2], ycc.cr, ycc.crwidth, height / 2);
    }
    src->frmno = frm->time2;
    SFX_GetTagInf(mwply->sfx, &tag_ptr, &tag_len);
    src->tag_ptr = tag_ptr;
    src->tag_len = tag_len;
    src->field_0x58 = 0;
    src->field_0x5C = 0;
    val = 3;
    switch (mwply->pic_struct) {
    case 1:
        val = 1;
        break;
    case 2:
        val = 2;
        break;
    case 3:
        val = 3;
        break;
    default:
        MWSFSVM_Error("E301272 : picture_structure is invalid.");
        break;
    }
    src->pic_struct = val;
    val = 1;
    switch (mwply->chroma_format) {
    case 1:
        val = 1;
        break;
    case 2:
        val = 2;
        break;
    case 3:
        val = 3;
        break;
    default:
        MWSFSVM_Error("E301273 : chroma_format is invalid.");
        break;
    }
    src->chroma_format = val;
    src->field_0x68 = mwply->x90;
    src->field_0x6C = mwply->x94;
    src->field_0x70 = mwply->x98;
    val = 1;
    switch (mwply->chromapos_h) {
    case 0:
        val = 0;
        break;
    case 1:
        val = 1;
        break;
    default:
        MWSFSVM_Error("E301274 : chromapos is invalid.");
        break;
    }
    src->chromapos_h = val;
    val = 1;
    switch (mwply->chromapos_v) {
    case 0:
        val = 0;
        break;
    case 1:
        val = 1;
        break;
    default:
        MWSFSVM_Error("E301274 : chromapos is invalid.");
        break;
    }
    src->chromapos_v = val;
    src->usrdat = frm->sudat;
    src->usrsiz = frm->sulen;
    ccs = mwsfsfx_IsFrmHdrCcs(mwply, frm);
    if (frm->sudat != NULL) {
        if (SUD_AnalyTypeCcs(frm->sudat, frm->sulen) == 1) {
            ccs = TRUE;
        } else {
            ccs = FALSE;
        }
    }
    src->field_0x90 = ccs;
    src->field_0x94 = mwsfsfx_GetFrmFxType(mwply, frm);
    if (mwply->compo_fix == 0) {
        compo = mwsfsfx_GetFrmFxType(mwply, frm);
        if (compo != -1) {
            mwply->compo = compo;
        } else {
            mwply->compo = SFX_COMPO_YCC420PLN;
        }
    }
    SFX_SetCompoMode(mwply->sfx, mwply->compo);
}

/* Destroys the converter handle. */
void MWSFSFX_Destroy(SFX_OBJ *sfx) {
    SFX_Destroy(sfx);
}

/* Creates a converter handle in the given work area. */
SFX_OBJ *MWSFSFX_Create(void *work, Sint32 wsize) {
    return SFX_Create(work, wsize);
}
