/* CRI Sofdec SFX frame converter (sfx_cnv.c): the conversion of a decoded frame by the converter
 * functions registered with the handle, and the functions that register them. Depending on the
 * component layout the conversion table is built first (luma range, alpha or the user table) and
 * the alpha part is converted separately.
 *
 * The file name is the one that holds the conversion table functions in the RE4 GameCube
 * decompilation; the message texts name the sfxcnv_ functions. The messages of functions that the
 * linker removed (an E5022301 work size check, SFX_MakeTblZ32 and SFX_MakeTblZ16) are not
 * reproduced, so this object's .rodata is 0x78 bytes short of the original.
 */
#include <string.h>

#include "CRI/sfx.h"
#include "CRI/sud.h"

/* Whether the layout keeps the picture in the upper half of the frame, so the output is half as
 * high. Inlined into sfxcnv_MakeDstFrm. */
static Bool sfxcnv_IsCnvUpHalf(Sint32 compo) {
    Bool uphalf;

    switch (compo) {
    case SFX_COMPO_YCC420PLN:
    case SFX_COMPO_0x31:
    case SFX_COMPO_0x41:
    case SFX_COMPO_0x51:
    case SFX_COMPO_0x61:
    case SFX_COMPO_0x71:
    case SFX_COMPO_0xF1:
    case SFX_COMPO_0x111:
    case SFX_COMPO_0x1001:
        uphalf = FALSE;
        break;
    case SFX_COMPO_YCC420PLN_UPHALF:
    case SFX_COMPO_0x101:
        uphalf = TRUE;
        break;
    default:
        SFXLIB_Error(NULL, NULL, "E201312: sfxcnv_IsCnvUpHalf : compo is invalid.");
        uphalf = FALSE;
        break;
    }
    return uphalf;
}

/* Crops the frame and describes the output buffer as the first plane of dst. */
void sfxcnv_MakeDstFrm(SFX_OBJ *sfx, SFX_SRCFRM *frm, Uint8 *buf, SFX_FRM *dst, Sint32 x, Sint32 y,
                       Sint32 width, Sint32 height) {
    Sint32 pitch;

    frm->width = width;
    frm->height = height;
    sfxcnv_CropFrm(frm, x, y);
    dst->pln[0].buf = buf;
    dst->pln[0].width = frm->width;
    if (sfxcnv_IsCnvUpHalf(sfx->compo) == TRUE) {
        dst->pln[0].height = frm->height / 2;
    } else {
        dst->pln[0].height = frm->height;
    }
    if (sfx->outbuf_width == 0) {
        pitch = frm->width;
    } else if (sfx->coladj == 0 && sfx->unit_width != 0) {
        pitch = sfx->outbuf_width / sfx->unit_width;
    } else {
        pitch = sfx->outbuf_width;
    }
    dst->pln[0].pitch = pitch;
    dst->frmfmt = 1;
}

void SFX_SetUnitWidth(SFX_OBJ *sfx, Sint32 unit_width) {
    sfx->unit_width = unit_width;
}

/* Converts the alpha part of the frame, which follows the picture in the planes, with the
 * function registered by SFX_SetAlpFn. */
void sfxcnv_ExecCopyAlphaByCbFunc(SFX_OBJ *sfx, SFX_SRCFRM *frm, SFX_FRM *dst) {
    SFX_CNVARG arg;
    SFX_FRM src;
    SFX_FRM alp;
    Sint32 frmfmt;
    Sint32 area;
    Sint32 half;

    sfxcnv_MakeCftSrcBuf(sfx, frm, &src);
    frmfmt = frm->frmfmt;
    switch (frmfmt) {
    case 3:
        area = src.pln[0].pitch * src.pln[0].height;
        half = area / 2;
        src.pln[0].buf += half;
        src.pln[1].buf += half / 2;
        src.pln[2].buf += half / 2;
        alp.pln[0].buf = dst->pln[0].buf;
        alp.pln[0].width = dst->pln[0].width;
        alp.pln[0].height = dst->pln[0].height;
        if (sfx->outbuf_width == 0) {
            alp.pln[0].pitch = src.pln[0].width * 4;
        } else if (sfx->coladj == 0 && sfx->unit_width != 0) {
            alp.pln[0].pitch = sfx->outbuf_width / sfx->unit_width;
        } else {
            alp.pln[0].pitch = sfx->outbuf_width;
        }
        break;
    /* Matching workaround: formats 1 and 2 have separate cases, and the format is read into a
     * local first, as in sfxcnv_MakeCftSrcBuf, whose separate case 1 and case 2 the original's
     * jump targets show. Both forms leave this code unchanged, but they give the function 31
     * statements, one more than -inline auto admits; with merged cases or without the local,
     * sfxcnv_ChkCompoByCbFunc inlines it instead of calling it as the original does (1134 words
     * instead of 1065). A named local for the offset of planes 1 and 2 adds a statement too, but
     * changes 9 words of this function. Revisit if the original's case 1 and case 2 bodies or the
     * unit's real inline setting are found. */
    case 1:
        break;
    case 2:
        break;
    default:
        SFXLIB_Error(sfx, frm, "E4011801: sfxcnv_ExecCopyAlphaByCbFunc : frame format is invalid.");
        break;
    }
    if (SFX_GetCcirFx() == 1) {
        arg.tbl = sfx->buf[0];
    } else {
        arg.tbl = NULL;
    }
    if (sfx->alpfn != NULL) {
        sfx->alpfn(&src, &alp, &arg);
    }
}

/* Fills out the planes of the frame the converters read. */
void sfxcnv_MakeCftSrcBuf(SFX_OBJ *sfx, SFX_SRCFRM *frm, SFX_FRM *out) {
    Sint32 frmfmt;

    frmfmt = frm->frmfmt;
    switch (frmfmt) {
    /* Formats 1 and 2 each have their own copy of the same body: one shared case compiles to a
     * shorter function (0xF4 bytes instead of the original's 0x12C). */
    case 1:
        out->frmfmt = 1;
        out->pln[0].buf = frm->pln[0].buf;
        out->pln[0].width = frm->width;
        out->pln[0].height = frm->pln[0].height;
        out->pln[0].pitch = frm->pln[0].pitch;
        break;
    case 2:
        out->frmfmt = 1;
        out->pln[0].buf = frm->pln[0].buf;
        out->pln[0].width = frm->width;
        out->pln[0].height = frm->pln[0].height;
        out->pln[0].pitch = frm->pln[0].pitch;
        break;
    case 3:
        out->frmfmt = 3;
        out->pln[0].buf = frm->pln[0].buf;
        out->pln[0].width = frm->width;
        out->pln[0].height = frm->pln[0].height;
        out->pln[0].pitch = frm->pln[0].pitch;
        out->pln[1].buf = frm->pln[1].buf;
        out->pln[1].width = frm->width / 2;
        out->pln[1].height = frm->pln[1].height;
        out->pln[1].pitch = frm->pln[1].pitch;
        out->pln[2].buf = frm->pln[2].buf;
        out->pln[2].width = frm->width / 2;
        out->pln[2].height = frm->pln[2].height;
        out->pln[2].pitch = frm->pln[2].pitch;
        break;
    default:
        SFXLIB_Error(sfx, frm, "E4111901: sfxcnv_MakeCftSrcBuf : frame format is invalid.");
        break;
    }
}

/* Whether the picture is split in fields (name inferred): never when SFX_GetForceSplit says so,
 * otherwise by the division mode of the handle, which -1 leaves to the picture user data. */
static Bool sfxcnv_IsDivField(SFX_OBJ *sfx, SFX_SRCFRM *frm) {
    Bool divfield;

    if (SFX_GetForceSplit() == 1) {
        divfield = FALSE;
    } else {
        switch (SFX_GetDivMode(sfx)) {
        case -1:
            divfield = FALSE;
            if (frm->usrdat != NULL) {
                if (SUD_AnalyTypeDivField(frm->usrdat, frm->usrsiz) == TRUE) {
                    if (SFX_GetField0x5C(sfx) == 0) {
                        divfield = TRUE;
                    }
                }
            }
            break;
        case 0:
            divfield = FALSE;
            break;
        case 1:
            divfield = TRUE;
            break;
        default:
            divfield = FALSE;
            break;
        }
    }
    return divfield;
}

/* Starts the plane at its last row and runs it upwards. Named as in the RE4 GameCube map, where
 * a 0x2C-byte function of this name does the same (inlined here). */
static void SFX_SetBottomUpPlnBuf(SFX_PLN *pln) {
    pln->buf += pln->pitch * (pln->height - 1);
    pln->pitch = -pln->pitch;
}

/* The alpha table of the layouts with three alpha levels. Named as in the RE4 GameCube map, whose
 * function of this name makes the same choice. */
static Sint32 SFX_DecideTableAlph3(SFX_OBJ *sfx, SFX_SRCFRM *frm) {
    Sint32 tbl;

    if (sfx->compo == SFX_COMPO_0x51) {
        tbl = SFX_TBL_ALP3110;
    } else if (sfx->compo == SFX_COMPO_0x61) {
        tbl = SFX_TBL_ALP3211;
    } else if (frm->field_0x94 == SFX_COMPO_0x51) {
        tbl = SFX_TBL_ALP3110;
    } else if (frm->field_0x94 == SFX_COMPO_0x61) {
        tbl = SFX_TBL_ALP3211;
    } else {
        tbl = SFX_TBL_ALP3211;
    }
    return tbl;
}

/* Luma range expansion (name inferred): 0 below 16, 1.164 times the distance from 16 up to 235, 255
 * above. Declared inline: the 1.164f constant then follows the "COMPO" string in .rodata and the
 * integer conversion constant follows the last message, as in the original; as a plain static
 * function both come ahead of "COMPO". */
static inline void sfxcnv_MakeLumiTbl(Uint8 *tbl) {
    Sint32 i;

    for (i = 0; i <= 15; i++) {
        tbl[i] = 0;
    }
    for (i = 16; i <= 235; i++) {
        tbl[i] = (Uint8)(1.164f * (Float32)(i - 16));
    }
    for (i = 236; i <= 255; i++) {
        tbl[i] = 255;
    }
}

/* Builds the conversion table in buf[0], unless it already holds that table. The table of the
 * alpha luma ramp is also built again when its settings changed. Declared inline: the original
 * has it inlined at each of its four calls in sfxcnv_ChkCompoByCbFunc. */
static inline void sfxcnv_MakeTable(SFX_OBJ *sfx, SFX_SRCFRM *frm, Sint32 tbl) {
    Bool need;

    need = TRUE;
    if (sfx->tbl == SFX_TBL_NONE) {
        need = FALSE;
    } else if (sfx->tbl == tbl) {
        switch (tbl) {
        case SFX_TBL_ALP_LUMI:
            if (SFXA_IsNeedUpdateLumiTbl(sfx->sfxa) != TRUE) {
                need = FALSE;
            }
            break;
        case SFX_TBL_LUMI:
        case SFX_TBL_ALP3110:
        case SFX_TBL_ALP3211:
        case SFX_TBL_USER:
            need = FALSE;
            break;
        case 0:
        case SFX_TBL_Z32:
        case 0x0C:
        case SFX_TBL_NONE:
        default:
            break;
        }
    }
    if (need != TRUE) {
        return;
    }
    sfx->tbl = tbl;
    switch (tbl) {
    case SFX_TBL_Z32:
    case SFX_TBL_Z16:
        SFXZ_MakeCnvZTbl(sfx->sfxz, frm->frmno, sfx->buf[0]);
        break;
    case SFX_TBL_ALP_LUMI:
        SFXA_MakeAlpLumiTbl(sfx->sfxa, frm->frmno, sfx->buf[0]);
        break;
    case SFX_TBL_ALP3110:
        SFXA_MakeAlp3110Tbl(sfx->sfxa, frm->frmno, sfx->buf[0]);
        break;
    case SFX_TBL_ALP3211:
        SFXA_MakeAlp3211Tbl(sfx->sfxa, frm->frmno, sfx->buf[0]);
        break;
    case SFX_TBL_USER:
        if (sfx->tblfn != NULL) {
            sfx->tblfn(sfx->buf[0]);
        }
        break;
    case SFX_TBL_LUMI:
        sfxcnv_MakeLumiTbl(sfx->buf[0]);
        break;
    case 0:
    case 3:
    case SFX_TBL_NONE:
    default:
        SFXLIB_Error(sfx, frm, "E201311: sfxcnv_MakeTable : compo is not support.");
        break;
    }
}

/* Converts the frame into dst with the function registered by SFX_SetCnvFn (name inferred). */
static void sfxcnv_ExecCnvByCbFunc(SFX_OBJ *sfx, SFX_SRCFRM *frm, SFX_FRM *dst, Uint8 *tbl) {
    SFX_CNVARG arg;
    SFX_FRM src;

    memset(&src, 0, sizeof(SFX_FRM));
    sfxcnv_MakeCftSrcBuf(sfx, frm, &src);
    arg.tbl = tbl;
    arg.work = sfx->work;
    if (sfx->outbuf_height == 0) {
        arg.height = frm->height;
    } else {
        arg.height = sfx->outbuf_height;
    }
    if (sfxcnv_IsDivField(sfx, frm) == TRUE) {
        arg.split = 1;
    } else {
        arg.split = 0;
    }
    if (SFX_GetBottomUp(sfx) == 1) {
        SFX_SetBottomUpPlnBuf(&dst->pln[0]);
    }
    if (sfx->cnvfn != NULL) {
        sfx->cnvfn(&src, dst, &arg);
    }
}

/* Converts the picture, then its alpha part (name inferred). The RE4 GameCube build also converts
 * this layout with a separate function that does the picture and then the alpha part. With the two
 * calls written in sfxcnv_ChkCompoByCbFunc instead, the frame record and argument block of the
 * inlined converter move from the stack offsets 0xC8 and 0x8 that the original uses to 0x194 and
 * 0x68. */
static void sfxcnv_ExecCnvWithAlpha(SFX_OBJ *sfx, SFX_SRCFRM *frm, SFX_FRM *dst) {
    sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, NULL);
    sfxcnv_ExecCopyAlphaByCbFunc(sfx, frm, dst);
}

/* Converts the frame into dst, after building the table the component layout needs. The name is
 * that in the message of the unsupported layouts. */
void sfxcnv_ChkCompoByCbFunc(SFX_OBJ *sfx, SFX_SRCFRM *frm, SFX_FRM *dst) {
    Sint32 compo;
    Sint32 tbl;

    compo = sfx->compo;
    if (compo == 0) {
        sfx->compo = SFXINF_GetStmInf(frm, "COMPO");
    }
    /* Original behaviour: the switch uses the value read before the lookup, so the first frame of
     * a new handle is reported as unsupported. */
    switch (compo) {
    case SFX_COMPO_YCC420PLN:
        if (frm->field_0x90 != 1) {
            sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, NULL);
            break;
        }
        /* fall through */
    case SFX_COMPO_0x1001:
        sfxcnv_MakeTable(sfx, frm, SFX_TBL_USER);
        sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, sfx->buf[0]);
        break;
    case SFX_COMPO_YCC420PLN_UPHALF:
        sfxcnv_MakeTable(sfx, frm, SFX_TBL_LUMI);
        sfxcnv_ExecCnvWithAlpha(sfx, frm, dst);
        break;
    case SFX_COMPO_0x31:
        sfxcnv_MakeTable(sfx, frm, SFX_TBL_ALP_LUMI);
        sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, sfx->buf[0]);
        break;
    case SFX_COMPO_0x41:
    case SFX_COMPO_0x51:
    case SFX_COMPO_0x61:
        tbl = SFX_DecideTableAlph3(sfx, frm);
        sfxcnv_MakeTable(sfx, frm, tbl);
        sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, sfx->buf[0]);
        break;
    case SFX_COMPO_0x101:
        sfxcnv_ExecCnvByCbFunc(sfx, frm, dst, NULL);
        break;
    case SFX_COMPO_0x71:
    case SFX_COMPO_0xF1:
    case SFX_COMPO_0x111:
    default:
        SFXLIB_Error(sfx, frm, "E4111902: sfxcnv_ChkCompoByCbFunc : compo is not support.");
        break;
    }
}

void SFX_SetTblFn(SFX_OBJ *sfx, SFX_TBLFN fn) {
    sfx->tblfn = fn;
}

void SFX_SetMakeAlp3110Fn(SFX_OBJ *sfx, SFXA_MakeAlp3Fn fn) {
    sfx->sfxa->make_alp3110 = fn;
}

void SFX_SetMakeAlp3211Fn(SFX_OBJ *sfx, SFXA_MakeAlp3Fn fn) {
    sfx->sfxa->make_alp3211 = fn;
}

void SFX_SetMakeLumiFn(SFX_OBJ *sfx, SFXA_MakeLumiFn fn) {
    sfx->sfxa->make_lumi = fn;
}

void SFX_SetAlpFn(SFX_OBJ *sfx, SFX_CNVFN fn) {
    sfx->alpfn = fn;
}

void SFX_SetCnvFn(SFX_OBJ *sfx, SFX_CNVFN fn) {
    sfx->cnvfn = fn;
}
