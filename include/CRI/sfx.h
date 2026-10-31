#ifndef CRI_SFX_H
#define CRI_SFX_H

/* CRI Sofdec SFX frame conversion: the frame description the converters read, the converter
 * handle and the library work (sfx_lib.c, sfx_inf.c, sfx_sud.c, sfx_alp.c, sfx_zmv.c). */
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

/* Leading part of the frame description handed to the registered converters: the frame format
 * followed by the Y, Cb and Cr planes. As a destination only the first plane is filled. This is a
 * partial view: the full frame record continues past these fields (its size is not established),
 * so the view has no size check. */
typedef struct {
    Sint32 frmfmt;  /* 0x00 */
    SFX_PLN pln[3]; /* 0x04 */
} SFX_FRM;

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

/* The Z conversion object (sfx_zmv.c) and the alpha object (sfx_alp.c). */
typedef struct SFXZ_OBJ SFXZ_OBJ;
typedef struct SFXA_OBJ SFXA_OBJ;

/* A converter handle (sfx_lib.c). Fields are named where SFX_Create or SFX_Destroy establishes
 * their role (handle flag, Z and alpha objects, work buffers); the rest keep offset names and are
 * used by the converters. */
typedef struct SFX_OBJ {
    Sint32 used;          /* 0x00 handle is taken */
    Sint32 field_0x04;    /* 0x04 cleared by SFX_Create */
    Sint32 field_0x08;    /* 0x08 cleared by SFX_Create */
    Sint32 field_0x0C;    /* 0x0C cleared by SFX_Create */
    Sint32 field_0x10[5]; /* 0x10 */
    SFXZ_OBJ *sfxz;       /* 0x24 */
    Sint32 field_0x28;    /* 0x28 set to 1 by SFX_Create */
    Sint32 field_0x2C;    /* 0x2C cleared by SFX_Create */
    SFXA_OBJ *sfxa;       /* 0x30 */
    Sint32 field_0x34;    /* 0x34 cleared by SFX_Create */
    Uint8 *buf[4];        /* 0x38 four consecutive 0x400-byte buffers carved from the work */
    Sint32 field_0x48[2]; /* 0x48 */
    void *work;           /* 0x50 the work area handed to SFX_Create */
    Sint32 wsize;         /* 0x54 its size */
    Sint32 field_0x58;    /* 0x58 set to -1 by SFX_Create */
    Sint32 field_0x5C[2]; /* 0x5C */
    Sint32 field_0x64;    /* 0x64 cleared by SFX_Create */
    void *field_0x68;     /* 0x68 converter callback, called by fn_803CFF44 */
    void *field_0x6C;     /* 0x6C converter callback, called by fn_803CFCB8 */
    void *field_0x70;     /* 0x70 callback, called with buf[0] by fn_803CFF44 */
    Sint32 field_0x74[9]; /* 0x74 */
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
void SFXLIB_Error(SFX_OBJ *sfx, SFX_FRM *frm, const Char8 *msg);
void SFX_Destroy(SFX_OBJ *sfx);
SFX_OBJ *SFX_Create(void *work, Sint32 wsize);
void SFX_SetErrFn(SFX_ERRFN fn, void *obj);
void SFX_Finish(void); /* name inferred: the counterpart of SFX_Init */
void SFX_Init(void);

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
