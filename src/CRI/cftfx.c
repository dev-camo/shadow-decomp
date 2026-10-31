/* Sofdec colour format transforms (cftfx.c): the colour adjustment table for the ARGB8888
 * output (the RGB to YUV matrix cft_rgb_yuv_coeff inverted, through the custom inverse tables)
 * and the YCC 4:2:0 planar luma to alpha mask ("A256V") of RGBA8 4x4 tiles: the alpha bytes are
 * ANDed with the luma, directly or through a user table.
 *
 * Adapted from the RE4 GameCube decompilation of the same file. This game's library identifies
 * itself as CRI CFT/GC Ver.1.72 and adds a wrapper that takes the frame records of the SFX
 * converter.
 */
#include <stddef.h>

#include "CRI/cft.h"

/* RGB to YUV matrix of Sofdec's own coefficients */
Float32 cft_rgb_yuv_coeff[9] = {
    0.30078125f, 0.5859375f,  0.11328125f, -0.171875f,   -0.33984375f,
    0.51171875f, 0.51171875f, -0.4296875f, -0.08203125f,
};

/* .bss is laid out in reverse declaration order (-inline auto,deferred), as are the functions */
Uint8 cft_conv_y_itbl[0x100];
Uint8 cft_conv_u_itbl[0x100];
Uint8 cft_conv_v_itbl[0x100];
Float32 *cft_ptr_y_rgb;
static Float32 *cft_ptr_cb_rgb;
Float32 *cft_ptr_cr_rgb;
Float32 cft_yuv_rgb_coeff[9];
/* No function of this game uses it (its user in the file is not present); it keeps the .bss
 * layout. */
Float32 cft_basic_ccir601[9];

/* One row of a 4x4 RGBA8 tile's AR half: the alpha bytes ANDed with the luma of the row, four
 * samples read as one word. */
#define CFT_A256_ROW_STATIC(dst, w, lo)                                                            \
    (lo) = (((w) << 16) & 0xFF000000) | (((w) & 0xFF) << 8);                                       \
    (dst)[0] &= (((w) & 0xFF000000) | (((w) >> 8) & 0x0000FF00)) | 0x00FF00FF;                     \
    (dst)[1] &= (lo) | 0x00FF00FF

/* Alpha masked by the luma, without a table. */
void cnvStaticYcc420plnToA256V(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst) {
    Sint32 i;
    Sint32 j;
    Sint32 wblk = dst->width / 4;
    Sint32 hblk = dst->height / 4;
    Sint32 ystep = src->ywidth / sizeof(Uint32);
    Sint32 dskip = (dst->pitch - dst->width) / 4 * 16;
    Uint32 *d = dst->buf;
    const Uint32 *y = (const Uint32 *)src->y;
    Uint32 w;
    Uint32 lo;

    for (i = 0; i < hblk; i++) {
        for (j = 0; j < wblk; j++) {
            w = *y;
            y += ystep;
            CFT_A256_ROW_STATIC(d, w, lo);
            w = *y;
            y += ystep;
            CFT_A256_ROW_STATIC(d + 2, w, lo);
            w = *y;
            y += ystep;
            CFT_A256_ROW_STATIC(d + 4, w, lo);
            w = *y;
            y += ystep;
            CFT_A256_ROW_STATIC(d + 6, w, lo);
            y = y - ystep * 4 + 1;
            d += 16;
        }
        /* As in the original, the step to the next tile row does not add ywidth - width (the table
         * converter's yskip does), so it is only correct when width equals ywidth. */
        y += ystep * 3;
        d += dskip;
    }
}

/* One row of a 4x4 RGBA8 tile's AR half: the alpha bytes ANDed with the table values of the four
 * luma samples, built in v0 and v1. y advances past the four samples. */
#define CFT_A256_ROW(dst, y, tbl, v0, v1)                                                          \
    (v0) = (Uint32)(tbl)[(y)[0]] << 24;                                                            \
    (v0) |= (Uint32)(tbl)[(y)[1]] << 8;                                                            \
    (v1) = (Uint32)(tbl)[(y)[2]] << 24;                                                            \
    (v1) |= (Uint32)(tbl)[(y)[3]] << 8;                                                            \
    (y) += 4;                                                                                      \
    (dst)[0] &= (v0) | 0x00FF00FF;                                                                 \
    (dst)[1] &= (v1) | 0x00FF00FF

/* Alpha masked by the luma through a user table. y walks down the four rows of a tile and then
 * back up to the next tile. */
void cnvDynamicYcc420plnToA256UserTable(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst,
                                        const Uint8 *tbl) {
    Sint32 i;
    Sint32 j;
    Sint32 wblk = dst->width / 4;
    Sint32 hblk = dst->height / 4;
    Sint32 ywidth = src->ywidth;
    Sint32 yskip = ywidth * 3 + (ywidth - dst->width);
    Sint32 dskip = (dst->pitch - dst->width) / 4 * 16;
    const Uint8 *y = src->y;
    Uint32 *d = dst->buf;
    Uint32 v0;
    Uint32 v1;

    for (i = 0; i < hblk; i++) {
        for (j = 0; j < wblk; j++) {
            CFT_A256_ROW(d, y, tbl, v0, v1);
            y += ywidth - 4;
            CFT_A256_ROW(d + 2, y, tbl, v0, v1);
            y += ywidth - 4;
            CFT_A256_ROW(d + 4, y, tbl, v0, v1);
            y += ywidth - 4;
            CFT_A256_ROW(d + 6, y, tbl, v0, v1);
            y += ywidth - 4;
            y = y - ywidth * 4 + 4;
            d += 16;
        }
        y += yskip;
        d += dskip;
    }
}

/* Registered converter (SFX_CNVFN): alpha masked by the luma. A NULL table in the argument block
 * selects the conversion without a table. The const frame records and argument block are
 * required: with a non-const argument block the table load moves after the frame copies, and with
 * non-const records each copy is loaded and stored in turn. */
void CFT_Ycc420plnToA256V(const SFX_FRM *frm, const SFX_FRM *dstfrm, const SFX_CNVARG *arg) {
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
        cnvStaticYcc420plnToA256V(&src, &dst);
    } else {
        cnvDynamicYcc420plnToA256UserTable(&src, &dst, arg->tbl);
    }
}

/* ARGB8888 colour adjustment: three 256-entry float tables (alpha, R, G, B contribution of the
 * component) at tbl, tbl + 0x1000 and tbl + 0x2000. */
void CFT_MakeArgb8888ColAdjTbl(void *tbl) {
    Sint32 i;

    cft_ptr_y_rgb = (Float32 *)tbl;
    cft_ptr_cb_rgb = (Float32 *)((Uint8 *)tbl + 0x1000);
    cft_ptr_cr_rgb = (Float32 *)((Uint8 *)tbl + 0x2000);
    CFT_MakeInvConvTableCustom(cft_conv_y_itbl, cft_conv_u_itbl, cft_conv_v_itbl);
    CFT_MakeInverseMtx3D(cft_rgb_yuv_coeff, cft_yuv_rgb_coeff);
    for (i = 0; i < 256; i++) {
        cft_ptr_y_rgb[(i << 2) + 1] = cft_yuv_rgb_coeff[0] * (Float32)cft_conv_y_itbl[i];
        cft_ptr_y_rgb[(i << 2) + 2] = cft_yuv_rgb_coeff[3] * (Float32)cft_conv_y_itbl[i];
        cft_ptr_y_rgb[(i << 2) + 3] = cft_yuv_rgb_coeff[6] * (Float32)cft_conv_y_itbl[i];
        cft_ptr_y_rgb[(i << 2) + 0] = 255.0f;
        cft_ptr_cb_rgb[(i << 2) + 1] =
            cft_yuv_rgb_coeff[1] * ((Float32)cft_conv_u_itbl[i] - 128.0f);
        cft_ptr_cb_rgb[(i << 2) + 2] =
            cft_yuv_rgb_coeff[4] * ((Float32)cft_conv_u_itbl[i] - 128.0f);
        cft_ptr_cb_rgb[(i << 2) + 3] =
            cft_yuv_rgb_coeff[7] * ((Float32)cft_conv_u_itbl[i] - 128.0f);
        cft_ptr_cr_rgb[(i << 2) + 1] =
            cft_yuv_rgb_coeff[2] * ((Float32)cft_conv_v_itbl[i] - 128.0f);
        cft_ptr_cr_rgb[(i << 2) + 2] =
            cft_yuv_rgb_coeff[5] * ((Float32)cft_conv_v_itbl[i] - 128.0f);
        cft_ptr_cr_rgb[(i << 2) + 3] =
            cft_yuv_rgb_coeff[8] * ((Float32)cft_conv_v_itbl[i] - 128.0f);
    }
}
