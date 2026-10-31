#ifndef CRI_CFT_H
#define CRI_CFT_H

/* CRI Sofdec colour format transforms (cftcoladj.c, cftfx.c). */
#include "CRI/cri_xpt.h"
#include "CRI/sfx.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

/* planar YCC 4:2:0 source of the tile converters */
typedef struct {
    Uint8 *y;             /* 0x00 */
    Uint8 *cb;            /* 0x04 */
    Uint8 *cr;            /* 0x08 */
    Sint32 ywidth;        /* 0x0C */
    Sint32 cbwidth;       /* 0x10 */
    Sint32 crwidth;       /* 0x14 */
    Sint32 field_0x18[4]; /* 0x18 unknown; the wrapper's stack frame needs the 0x28 size */
} CFT_YCC420PLN;
CHECK_SIZEOF(CFT_YCC420PLN, 0x28);

/* destination of the tile converters */
typedef struct {
    Uint32 *buf;          /* 0x00 */
    Sint32 width;         /* 0x04 */
    Sint32 height;        /* 0x08 */
    Sint32 pitch;         /* 0x0C */
    Sint32 field_0x10[2]; /* 0x10 unknown; the wrapper's stack frame needs the 0x18 size */
} CFT_ARGBDST;
CHECK_SIZEOF(CFT_ARGBDST, 0x18);

/* cftcoladj.c */
void CFT_MakeInverseMtx3D(Float32 *m, Float32 *inv);
void CFT_MakeInvConvTableCustom(Uint8 *ytbl, Uint8 *cbtbl, Uint8 *crtbl);

/* cftfx.c */
void CFT_Ycc420plnToA256V(const SFX_FRM *frm, const SFX_FRM *dstfrm, Uint8 *const *tbl);
void CFT_MakeArgb8888ColAdjTbl(void *tbl);

#ifdef __cplusplus
}
#endif

#endif
