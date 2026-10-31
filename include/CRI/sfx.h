#ifndef CRI_SFX_H
#define CRI_SFX_H

/* CRI Sofdec SFX frame conversion: the frame description the converters read, the converter
 * handle and the library work (sfx_lib.c, sfx_set.c, sfx_inf.c, sfx_sud.c, sfx_alp.c,
 * sfx_zmv.c). */
#include "CRI/cri_xpt.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

/* one colour plane of a frame */
typedef struct {
    Uint8 *buf;    /* 0x00 */
    Sint32 width;  /* 0x04 */
    Sint32 height; /* 0x08 */
    Sint32 pitch;  /* 0x0C line stride */
} SFX_PLN;
CHECK_SIZEOF(SFX_PLN, 0x10);

/* The frame description handed to the registered converters: the frame format followed by the Y,
 * Cb and Cr planes. As a destination only the first plane is filled. The size is that of the
 * zeroed frame records on the stack of the converter (sfx_cnv.c). */
typedef struct {
    Sint32 frmfmt;        /* 0x00 */
    SFX_PLN pln[3];       /* 0x04 */
    Sint32 field_0x34[4]; /* 0x34 */
} SFX_FRM;
CHECK_SIZEOF(SFX_FRM, 0x44);

/* One plane of a decoded frame as the application hands it to the converter. */
typedef struct {
    Uint8 *buf;        /* 0x00 */
    Sint32 pitch;      /* 0x04 line stride */
    Sint32 height;     /* 0x08 */
    Sint32 field_0x0C; /* 0x0C */
} SFX_SRCPLN;
CHECK_SIZEOF(SFX_SRCPLN, 0x10);

/* Leading part of a decoded frame as the application hands it to the converter: the frame
 * format, the Y, Cb and Cr planes, the picture size and the fields the converter reads from the
 * rest of the record. Partial view: the size of the full record is not established; the size
 * check covers these fields only. */
typedef struct {
    Sint32 frmfmt;         /* 0x00 */
    SFX_SRCPLN pln[3];     /* 0x04 */
    Sint32 field_0x34[4];  /* 0x34 */
    Sint32 width;          /* 0x44 */
    Sint32 height;         /* 0x48 */
    Sint32 frmno;          /* 0x4C argument of the table builders */
    Sint32 field_0x50[14]; /* 0x50 */
    Uint8 *usrdat;         /* 0x88 picture user data, NULL when there is none */
    Sint32 usrsiz;         /* 0x8C its size */
    Sint32 field_0x90;     /* 0x90 */
    Sint32 field_0x94;     /* 0x94 */
} SFX_SRCFRM;
CHECK_SIZEOF(SFX_SRCFRM, 0x98);

/* Component layouts: values of SFX_OBJ.compo and results of SFXINF_GetStmInf (names inferred). */
#define SFX_COMPO_YCC420PLN 0x11
#define SFX_COMPO_YCC420PLN_UPHALF 0x21
#define SFX_COMPO_0x31 0x31
#define SFX_COMPO_0x41 0x41
#define SFX_COMPO_0x51 0x51
#define SFX_COMPO_0x61 0x61
#define SFX_COMPO_0x71 0x71
#define SFX_COMPO_0xF1 0xF1
#define SFX_COMPO_0x101 0x101
#define SFX_COMPO_0x111 0x111
#define SFX_COMPO_0x1001 0x1001

/* Third argument of the registered converters. tbl is always set; work, height and split are set
 * only by sfxcnv_ExecCnvByCbFunc. */
typedef struct {
    Uint8 *tbl;           /* 0x00 conversion table, NULL for none */
    void *work;           /* 0x04 the work area of the handle */
    Sint32 field_0x08[4]; /* 0x08 */
    Sint32 height;        /* 0x18 output height */
    Sint32 split;         /* 0x1C 1 when the picture is split in fields */
} SFX_CNVARG;
CHECK_SIZEOF(SFX_CNVARG, 0x20);

/* A converter registered with SFX_SetCnvFn or SFX_SetAlpFn: converts the source frame into the
 * destination. */
typedef void (*SFX_CNVFN)(const SFX_FRM *src, const SFX_FRM *dst, const SFX_CNVARG *arg);

/* The table maker registered with SFX_SetTblFn: builds the user table at tbl. */
typedef void (*SFX_TBLFN)(void *tbl);

/* The Z conversion object (sfx_zmv.c) and the alpha object (sfx_alp.c). */
typedef struct SFXZ_OBJ SFXZ_OBJ;
typedef struct SFXA_OBJ SFXA_OBJ;

/* A converter handle (sfx_lib.c, sfx_set.c, sfx_cnv.c). Fields are named where SFX_Create,
 * SFX_Destroy or the converter establishes their role; the rest keep offset names. */
typedef struct SFX_OBJ {
    Sint32 used;          /* 0x00 handle is taken */
    Sint32 compo;         /* 0x04 component layout of the frames, 0 until it is known */
    Sint32 outbuf_width;  /* 0x08 output buffer size, 0 for the frame size */
    Sint32 outbuf_height; /* 0x0C */
    Sint32 coladj;        /* 0x10 */
    Sint32 taginf_flg;    /* 0x14 the tag information is set */
    Uint8 *tag_ptr;       /* 0x18 */
    Sint32 tag_len;       /* 0x1C */
    Sint32 field_0x20;    /* 0x20 */
    SFXZ_OBJ *sfxz;       /* 0x24 */
    Sint32 field_0x28;    /* 0x28 set to 1 by SFX_Create */
    Sint32 field_0x2C;    /* 0x2C cleared by SFX_Create */
    SFXA_OBJ *sfxa;       /* 0x30 */
    Sint32 tbl;           /* 0x34 the conversion table built in buf[0] */
    Uint8 *buf[4];        /* 0x38 four consecutive 0x400-byte buffers carved from the work */
    Sint32 field_0x48[2]; /* 0x48 */
    void *work;           /* 0x50 the work area handed to SFX_Create */
    Sint32 wsize;         /* 0x54 its size */
    Sint32 div_mode;      /* 0x58 split in fields: 0 never, 1 always, -1 by the user data */
    Sint32 field_0x5C;    /* 0x5C */
    Sint32 field_0x60;    /* 0x60 */
    Sint32 bottom_up;     /* 0x64 the output planes run bottom-up */
    SFX_CNVFN cnvfn;      /* 0x68 converter of the frame */
    SFX_CNVFN alpfn;      /* 0x6C converter of the alpha part */
    SFX_TBLFN tblfn;      /* 0x70 builds the user table in buf[0] */
    Sint32 unit_width;    /* 0x74 */
    Sint32 field_0x78[8]; /* 0x78 */
} SFX_OBJ;
CHECK_SIZEOF(SFX_OBJ, 0x98);

/* Error callback installed by SFX_SetErrFn. */
typedef void (*SFX_ERRFN)(void *obj, const Char8 *msg);

#define SFX_MAX_HN 8
#define SFX_BUF_SIZE 0x400

/* Smallest work area SFX_Create accepts: 0x3000 plus the slack for rounding its start up to 32
 * bytes. */
#define SFX_MIN_WORK_SIZE (0x3000 + 0x1F)

/* The library work: the handle table and the error callback. */
typedef struct {
    Sint32 hn_cnt;          /* 0x00 handles in use */
    Sint32 max_hn;          /* 0x04 */
    SFX_ERRFN errfn;        /* 0x08 */
    void *errobj;           /* 0x0C */
    Sint32 err_cnt;         /* 0x10 errors reported so far */
    Sint32 ccir_fx;         /* 0x14 see SFX_GetCcirFx */
    SFX_OBJ hn[SFX_MAX_HN]; /* 0x18 */
    Sint32 field_0x4D8[4];  /* 0x4D8 */
} SFX_LIBWORK;
CHECK_SIZEOF(SFX_LIBWORK, 0x4E8);

/* sfx_lib.c */
Sint32 SFX_GetForceSplit(void); /* name inferred: returns sfxcnv_forcesplit */
Sint32 SFX_GetCcirFx(void);
void SFXLIB_Error(SFX_OBJ *sfx, SFX_SRCFRM *frm, const Char8 *msg);
void SFX_Destroy(SFX_OBJ *sfx);
SFX_OBJ *SFX_Create(void *work, Sint32 wsize);
void SFX_SetErrFn(SFX_ERRFN fn, void *obj);
void SFX_Finish(void); /* name inferred: the counterpart of SFX_Init */
void SFX_Init(void);

/* sfx_set.c. SFX_GetTagInf, SFX_SetTagInf, SFX_SetColAdj, SFX_SetOutBufSize and
 * SFX_SetCompoMode are named as in the RE4 GameCube map, where the functions have the same sizes;
 * the names of sfxcnv_CropFrm and the three getters are inferred. */
void sfxcnv_CropFrm(SFX_SRCFRM *frm, Sint32 x, Sint32 y);
Sint32 SFX_GetBottomUp(SFX_OBJ *sfx);
Sint32 SFX_GetField0x5C(SFX_OBJ *sfx);
Sint32 SFX_GetDivMode(SFX_OBJ *sfx);
void SFX_GetTagInf(SFX_OBJ *sfx, Uint8 **tag_ptr, Sint32 *tag_len);
void SFX_SetTagInf(SFX_OBJ *sfx, Uint8 *tag_ptr, Sint32 tag_len);
void SFX_SetColAdj(SFX_OBJ *sfx, Sint32 coladj);
void SFX_SetOutBufSize(SFX_OBJ *sfx, Sint32 width, Sint32 height);
void SFX_SetCompoMode(SFX_OBJ *sfx, Sint32 compo);

/* sfx_inf.c */
Sint32 SFXINF_GetStmInf(SFX_SRCFRM *frm, const Char8 *tag);

/* sfx_sud.c */
void SFXSUD_Finish(void); /* name inferred: the counterpart of SFXSUD_Init */
void SFXSUD_Init(void);

/* frmno of the table builders below: the same argument for all four, which the caller passes from
 * SFX_SRCFRM.frmno. SFXZ_MakeCnvZTbl uses it as the frame index into the "ZMFDATA" tag; the SFXA
 * builders do not use it. */

/* sfx_alp.c */
Sint32 SFXA_IsNeedUpdateLumiTbl(SFXA_OBJ *sfxa);
void SFXA_MakeAlp3211Tbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl);
void SFXA_MakeAlp3110Tbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl);
void SFXA_MakeAlpLumiTbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl);
void SFXA_Destroy(SFXA_OBJ *sfxa);
SFXA_OBJ *SFXA_Create(void);
void SFXA_Finish(void); /* name inferred: the counterpart of SFXA_Init */
void SFXA_Init(void);

/* sfx_zmv.c */
void SFXZ_SetTagInf(SFXZ_OBJ *sfxz, Uint8 *tag_ptr, Sint32 tag_len);
void SFXZ_MakeCnvZTbl(SFXZ_OBJ *sfxz, Sint32 frmno, void *tbl);
void SFXZ_Destroy(SFXZ_OBJ *sfxz);
SFXZ_OBJ *SFXZ_Create(void);
void SFXZ_Finish(void); /* name inferred: the counterpart of SFXZ_Init */
void SFXZ_Init(void);

#ifdef __cplusplus
}
#endif

#endif
