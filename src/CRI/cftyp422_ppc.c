/* CRI Sofdec ARGB8888 conversion of YCC 4:2:0 planar frames (cftyp422_ppc.c): the alpha table
 * makers registered with the SFX converter, the converter entry that picks the plain or the
 * table-driven paired-single converter, and the initialiser of the static YCC to RGB tables.
 *
 * Not yet decompiled: the two paired-single converters cnvDynamicYcc420plnToArgb8888 and
 * cnvStaticYcc420plnToArgb8888, declared in cft.h.
 *
 * File, function and table names follow RE4's GameCube symbols, whose functions of these names
 * come in the same order with the same roles; all but CFT_Ycc420plnToArgb8888 (which takes the
 * SFX frame records here, as CFT_Ycc420plnToA256V in cftfx.c does) and
 * CFT_Ycc420plnToArgb8888Init also have the same sizes.
 *
 * Built with -inline auto,deferred like cftfx.c: the compiler emits the functions and lays out
 * .bss in reverse order of their definitions.
 */
#include <stddef.h>

#include "CRI/cft.h"

/* Tables of the paired-single converters, which read them and save GQR4 in gqr_save around a
 * conversion: the luma gain and the green and blue contribution of Cb and the red and green
 * contribution of Cr. In reverse order these give the original layout: gqr_save at 0, cr_r at 4,
 * cr_g at 0x404, cb_b at 0x804, cb_g at 0xC04 and the 32-byte aligned y at 0x1020. */
Float32 y[256] __attribute__((aligned(32)));
static Float32 cb_g[256];
Float32 cb_b[256];
Float32 cr_g[256];
Float32 cr_r[256];
Uint32 gqr_save;

/* Clamps a luma ramp value to 0..255. */
#define CFT_CLIP255(x) ((x) < 0.0f ? 0.0f : ((x) > 255.0f ? 255.0f : (x)))

/* Fills the static tables of the paired-single converters with the coefficients 1.164 (luma),
 * 1.596 and -0.813 (Cr), -0.392 and 2.017 (Cb). */
void CFT_Ycc420plnToArgb8888Init(void) {
    Sint32 i;
    Float32 *py = y;
    Float32 *pcb_g = cb_g;
    Float32 *pcb_b = cb_b;
    Float32 *pcr_r = cr_r;
    Float32 *pcr_g = cr_g;

    /* The original loop is not unrolled; written with i < 256 it is unrolled eightfold. */
    for (i = 0; i != 256; i++) {
        *py++ = 1.164f * (Float32)(i - 16);
        *pcb_g++ = -0.392f * (Float32)(i - 128);
        *pcb_b++ = 2.017f * (Float32)(i - 128);
        *pcr_r++ = 1.596f * (Float32)(i - 128);
        *pcr_g++ = -0.813f * (Float32)(i - 128);
    }
}

/* Converter registered with SFX_SetCnvFn: copies the planes and the output record of the frames
 * and runs the converter that uses the conversion table when the argument block has one. */
void CFT_Ycc420plnToArgb8888(const SFX_FRM *frm, const SFX_FRM *dstfrm, const SFX_CNVARG *arg) {
    CFT_YCC420PLN src;
    CFT_ARGBDST dst;

    src.y = frm->pln[0].buf;
    src.cb = frm->pln[1].buf;
    src.cr = frm->pln[2].buf;
    src.ywidth = frm->pln[0].pitch;
    src.cbwidth = frm->pln[1].pitch;
    src.crwidth = frm->pln[2].pitch;
    dst.buf = (Uint32 *)dstfrm->pln[0].buf;
    dst.width = dstfrm->pln[0].width;
    dst.height = dstfrm->pln[0].height;
    dst.pitch = dstfrm->pln[0].pitch;
    if (arg->tbl == NULL) {
        cnvStaticYcc420plnToArgb8888(&src, &dst);
    } else {
        cnvDynamicYcc420plnToArgb8888(&src, &dst, arg->tbl);
    }
}

/* The table makers fill tbl with 256 CFT_ARGBENT entries each for Y, Cb and Cr. */

/* Table maker registered with SFX_SetMakeLumiFn: the Cb and Cr entries, and the luma entries with
 * the BT.601 gain and an alpha ramp over the luma values lo..hi. mode (the alpha object's
 * lumi_mode) selects the direction of the ramp: 1 makes the alpha fall from 255 to 0 over the
 * range, any other value makes it rise from 0 to 255. */
void CFT_MakeArgb8888AlpLumiTbl(Sint32 mode, Sint32 lo, Sint32 hi, void *tbl) {
    CFT_ARGBENT *ytbl = tbl;
    CFT_ARGBENT *cbtbl = ytbl + 256;
    CFT_ARGBENT *crtbl = ytbl + 512;
    Sint32 range;
    Float32 scale;
    Sint32 i;

    for (i = 0; i < 256; i++) {
        ytbl[i].b = 0.5f + 1.164f * (Float32)(i - 16);
        ytbl[i].g = 0.5f + 1.164f * (Float32)(i - 16);
        ytbl[i].r = 0.5f + 1.164f * (Float32)(i - 16);
        cbtbl[i].b = 0.5f + 2.017f * (Float32)(i - 128);
        cbtbl[i].g = 0.5f + -0.392f * (Float32)(i - 128);
        cbtbl[i].r = 0.0f;
        cbtbl[i].a = 0.0f;
        crtbl[i].b = 0.0f;
        crtbl[i].g = 0.5f + -0.813f * (Float32)(i - 128);
        crtbl[i].r = 0.5f + 1.596f * (Float32)(i - 128);
        crtbl[i].a = 0.0f;
    }
    range = hi - lo;
    scale = 255.0f / (Float32)range;
    if (mode == 1) {
        for (i = 0; i < 256; i++) {
            if (i < lo) {
                ytbl[i].a = 255.0f;
            } else if (i > hi) {
                ytbl[i].a = 0.0f;
            } else {
                ytbl[i].a = scale * (Float32)(range - (i - lo));
            }
        }
    } else {
        for (i = 0; i < 256; i++) {
            if (i < lo) {
                ytbl[i].a = 0.0f;
            } else if (i > hi) {
                ytbl[i].a = 255.0f;
            } else {
                ytbl[i].a = scale * (Float32)(i - lo);
            }
        }
    }
}

/* Table maker registered with SFX_SetMakeAlp3110Fn: the Cb and Cr entries, and luma entries in
 * three bands with the alpha levels alp0, alp1 and alp2. The colour channels of a luma value i
 * get 0.0 for 0..8, 0.5 + (i - 16) * 255 / 110 (0.5 below 16) for 9..133, and the falling ramp
 * 0.5 + (251 - i) * 255 / 110 (0.5 above 251) for 134..255. */
void CFT_MakeArgb8888Alp3110Tbl(void *tbl, Uint8 alp0, Uint8 alp1, Uint8 alp2) {
    CFT_ARGBENT *ytbl = tbl;
    CFT_ARGBENT *cbtbl = ytbl + 256;
    CFT_ARGBENT *crtbl = ytbl + 512;
    Sint32 i;

    for (i = 0; i < 256; i++) {
        cbtbl[i].b = 0.5f + 2.017f * (Float32)(i - 128);
        cbtbl[i].g = 0.5f + -0.392f * (Float32)(i - 128);
        cbtbl[i].r = 0.0f;
        cbtbl[i].a = 0.0f;
        crtbl[i].b = 0.0f;
        crtbl[i].g = 0.5f + -0.813f * (Float32)(i - 128);
        crtbl[i].r = 0.5f + 1.596f * (Float32)(i - 128);
        crtbl[i].a = 0.0f;
    }
    for (i = 0; i < 9; i++) {
        ytbl[i].b = 0.0f;
        ytbl[i].g = 0.0f;
        ytbl[i].r = 0.0f;
        ytbl[i].a = (Float32)alp0;
    }
    for (i = 9; i < 134; i++) {
        Float32 v = 0.5f + (255.0f / 110.0f) * CFT_CLIP255((Float32)i - 16.0f);
        ytbl[i].b = v;
        ytbl[i].g = v;
        ytbl[i].r = v;
        ytbl[i].a = (Float32)alp1;
    }
    for (i = 134; i < 256; i++) {
        Float32 v = 0.5f + (255.0f / 110.0f) * CFT_CLIP255(251.0f - (Float32)i);
        ytbl[i].b = v;
        ytbl[i].g = v;
        ytbl[i].r = v;
        ytbl[i].a = (Float32)alp2;
    }
}

/* Table maker registered with SFX_SetMakeAlp3211Fn: the Cb and Cr entries, and luma entries in
 * three bands with the alpha levels alp0, alp1 and alp2. The colour channels of a luma value i
 * get the expanded value of luma 0, 0.5 - 16 * 255 / 219, for 0..47, 0.5 + (i - 68) * 255 / 55
 * (0.5 below 68) for 48..129, and the falling ramp 0.5 + (247 - i) * 255 / 111 (0.5 above 247) for
 * 130..255.
 */
void CFT_MakeArgb8888Alp3211Tbl(void *tbl, Uint8 alp0, Uint8 alp1, Uint8 alp2) {
    CFT_ARGBENT *ytbl = tbl;
    CFT_ARGBENT *cbtbl = ytbl + 256;
    CFT_ARGBENT *crtbl = ytbl + 512;
    Sint32 i;

    for (i = 0; i < 256; i++) {
        cbtbl[i].b = 0.5f + 2.017f * (Float32)(i - 128);
        cbtbl[i].g = 0.5f + -0.392f * (Float32)(i - 128);
        cbtbl[i].r = 0.0f;
        cbtbl[i].a = 0.0f;
        crtbl[i].b = 0.0f;
        crtbl[i].g = 0.5f + -0.813f * (Float32)(i - 128);
        crtbl[i].r = 0.5f + 1.596f * (Float32)(i - 128);
        crtbl[i].a = 0.0f;
    }
    for (i = 0; i < 48; i++) {
        ytbl[i].b = -16.0f * (255.0f / 219.0f) + 0.5f;
        ytbl[i].g = -16.0f * (255.0f / 219.0f) + 0.5f;
        ytbl[i].r = -16.0f * (255.0f / 219.0f) + 0.5f;
        ytbl[i].a = (Float32)alp0;
    }
    for (i = 48; i < 130; i++) {
        Float32 v = 0.5f + (255.0f / 55.0f) * CFT_CLIP255((Float32)i - 68.0f);
        ytbl[i].b = v;
        ytbl[i].g = v;
        ytbl[i].r = v;
        ytbl[i].a = (Float32)alp1;
    }
    for (i = 130; i < 256; i++) {
        Float32 v = 0.5f + (255.0f / 111.0f) * CFT_CLIP255(247.0f - (Float32)i);
        ytbl[i].b = v;
        ytbl[i].g = v;
        ytbl[i].r = v;
        ytbl[i].a = (Float32)alp2;
    }
}
