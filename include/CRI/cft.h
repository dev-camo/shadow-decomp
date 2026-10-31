#ifndef CRI_CFT_H
#define CRI_CFT_H

/* CRI Sofdec colour format transforms (cftcoladj.c, cftfx.c, cftyp422_ppc.c) and their library init
 * (cft_lib.c). */
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

/* One entry of the ARGB conversion table: the contribution of a component value to the alpha, red,
 * green and blue channels. The table makers of cftyp422_ppc.c fill 256 entries each for Y, Cb and
 * Cr; CFT_MakeArgb8888ColAdjTbl (cftfx.c) fills the same layout through its Float32 table
 * pointers. */
typedef struct {
    Float32 a;
    Float32 r;
    Float32 g;
    Float32 b;
} CFT_ARGBENT;
CHECK_SIZEOF(CFT_ARGBENT, 0x10);

/* cftcoladj.c */
void CFT_MakeInverseMtx3D(Float32 *m, Float32 *inv);
void CFT_MakeInvConvTableCustom(Uint8 *ytbl, Uint8 *cbtbl, Uint8 *crtbl);

/* cft_lib.c */
void CFT_Finish(void); /* name inferred: the counterpart of CFT_Init */
void CFT_Init(void);   /* name inferred: the library start called from SFX_Init */

/* cftyp422_ppc.c */
void CFT_MakeArgb8888Alp3211Tbl(void *tbl, Uint8 alp0, Uint8 alp1, Uint8 alp2);
void CFT_MakeArgb8888Alp3110Tbl(void *tbl, Uint8 alp0, Uint8 alp1, Uint8 alp2);
void CFT_MakeArgb8888AlpLumiTbl(Sint32 mode, Sint32 lo, Sint32 hi, void *tbl);
/* The paired-single converters called by CFT_Ycc420plnToArgb8888 (not yet decompiled): the one
 * that takes the conversion table and the plain one. tbl has the type of SFX_CNVARG.tbl, which is
 * shared with the byte-sized A256 user table; for this converter it holds the CFT_ARGBENT entries
 * built by the table makers above. */
void cnvDynamicYcc420plnToArgb8888(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst,
                                   const Uint8 *tbl);
void cnvStaticYcc420plnToArgb8888(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst);
void CFT_Ycc420plnToArgb8888(const SFX_FRM *frm, const SFX_FRM *dstfrm, const SFX_CNVARG *arg);
/* Fills the static YCC to RGB coefficient tables. */
void CFT_Ycc420plnToArgb8888Init(void);

/* cftfx.c */
void CFT_Ycc420plnToA256V(const SFX_FRM *frm, const SFX_FRM *dstfrm, const SFX_CNVARG *arg);
void CFT_MakeArgb8888ColAdjTbl(void *tbl);

#ifdef __cplusplus
}
#endif

#endif
