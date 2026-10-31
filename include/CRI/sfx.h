#ifndef CRI_SFX_H
#define CRI_SFX_H

/* CRI Sofdec SFX frame conversion: the frame description the converters read. */
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

/* The Z conversion object (sfx_zmv.c). */
typedef struct SFXZ_OBJ SFXZ_OBJ;

/* sfx_lib.c */
Sint32 SFX_GetCcirFx(void);

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
