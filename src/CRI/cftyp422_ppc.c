/* CRI Sofdec ARGB8888 conversion of YCC 4:2:0 planar frames (cftyp422_ppc.c): the alpha table
 * makers registered with the SFX converter, the paired-single converters into RGBA8 4x4 tiles,
 * the converter entry that picks the plain or the table-driven one, and the initialiser of the
 * static YCC to RGB tables.
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

/* Tables of the static converter, the luma gain and the green and blue contribution of Cb and the
 * red and green contribution of Cr, and gqr_save, where both paired-single converters keep GQR4
 * during a conversion. In reverse order these give the original layout: gqr_save at 0, cr_r at 4,
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

/* The two paired-single converters write the output as RGBA8 4x4 tiles of 64 bytes: a 32-byte AR
 * half (the alpha and red byte pairs, 8 bytes per pixel row) followed by a 32-byte GB half (the
 * green and blue pairs). A tile takes four luma rows, each read as one big-endian word of four
 * samples, and two rows of Cb and Cr, each read as one halfword of two samples: the high byte
 * serves pixels 0 and 1, the low byte pixels 2 and 3, chroma row 0 serves luma rows 0 and 1 and
 * chroma row 1 luma rows 2 and 3. Red is the luma value plus the Cr red value, green the luma value
 * plus the Cb and Cr green values, blue the luma value plus the Cb blue value. The table values
 * and the static converter's alpha are loaded with lfs, which on the Gekko fills both halves of the
 * paired-single register. The luma values of pixels 0 and 1 and of pixels 2 and 3 are paired, the
 * chroma values added to both halves, and the (alpha, red) and (green, blue) pairs stored with
 * psq_st through GQR4, which the converters set to unsigned 8-bit quantisation with scale 0
 * (0x00040004) and restore from gqr_save at the end.
 * The first luma word and the chroma halfwords of a tile are read before dcbz is applied to the
 * addresses of its two halves, and the table values are looked up again for each pixel row.
 *
 * The tile counts (ywidth / 4 tiles per tile row, height / 4 tile rows) and the output skip of
 * (pitch - ywidth) / 4 tiles after each tile row are signed divisions. The plane strides are
 * unsigned shifts that round down: the luma stride is ywidth truncated to a multiple of 4 (ywidth 5
 * steps 4 bytes per row and 12 bytes for three rows) and the chroma stride is cbwidth truncated to
 * a multiple of 2 (cbwidth 3 steps 2 bytes per row and 4 bytes for two rows). crwidth is not read:
 * the Cr plane is stepped with the Cb stride. */

/* Converter with the static tables of CFT_Ycc420plnToArgb8888Init and alpha 255. */
void cnvStaticYcc420plnToArgb8888(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst) {
    Sint32 wblk = src->ywidth / 4;
    Sint32 hblk = dst->height / 4;
    Sint32 dskip = (dst->pitch - src->ywidth) / 4 * 16; /* in words, 16 per tile */
    Uint32 ystride = (Uint32)src->ywidth >> 2;          /* luma words per row */
    Uint32 cstride = (Uint32)src->cbwidth >> 1;         /* chroma halfwords per row */
    const Uint32 *yp = (const Uint32 *)src->y;
    const Uint16 *cbrow = (const Uint16 *)src->cb;
    const Uint16 *crrow = (const Uint16 *)src->cr;
    register Uint32 *ar = dst->buf;     /* AR half of the tile */
    register Uint32 *gb = dst->buf + 8; /* GB half of the tile */
    const Uint16 *cbp0;
    const Uint16 *crp0;
    const Uint16 *cbp1;
    const Uint16 *crp1;
    Uint32 luma;
    Uint32 cb;
    Uint32 cr;
    register Uint32 gqr;
    register Uint32 gqr_u8 = 0x00040004;
    register Float32 alpha = 255.0f;
    /* table values of the four luma samples of a row and of the two chroma samples of a row */
    register Float32 l0, l1, l2, l3;
    register Float32 cr_r0, cr_g0, cb_g0, cb_b0, cr_r1, cr_g1, cb_g1, cb_b1;
    /* the summed green chroma values, the luma pairs, the channel pairs and the pair stored */
    register __vec2x32float__ grn0, grn1, l01, l23, red01, grn01, blu01, red23, grn23, blu23, pair;
    Sint32 i;
    Sint32 j;

    asm {
        mfspr gqr, GQR4
    }
    gqr_save = gqr;
    asm {
        mtspr GQR4, gqr_u8
    }
    for (i = 0; i < hblk; i++) {
        cbp0 = cbrow;
        crp0 = crrow;
        cbp1 = cbrow + cstride;
        crp1 = crrow + cstride;
        for (j = 0; j < wblk; j++) {
            /* pixel row 0, with chroma row 0; the plane words are read before the dcbz */
            luma = yp[0];
            cb = *cbp0;
            cr = *crp0;
            __dcbz(ar, 0);
            __dcbz(gb, 0);
            cr_r0 = cr_r[cr >> 8];
            cr_g0 = cr_g[cr >> 8];
            cb_g0 = cb_g[cb >> 8];
            cb_b0 = cb_b[cb >> 8];
            cr_r1 = cr_r[cr & 0xFF];
            cr_g1 = cr_g[cr & 0xFF];
            cb_g1 = cb_g[cb & 0xFF];
            cb_b1 = cb_b[cb & 0xFF];
            l0 = y[luma >> 24];
            l1 = y[(luma >> 16) & 0xFF];
            l2 = y[(luma >> 8) & 0xFF];
            l3 = y[luma & 0xFF];
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, alpha, red01
                psq_st pair, 0x0(ar), 0, 4
                ps_merge11 pair, alpha, red01
                psq_st pair, 0x2(ar), 0, 4
                ps_merge10 pair, alpha, red23
                psq_st pair, 0x4(ar), 0, 4
                ps_merge11 pair, alpha, red23
                psq_st pair, 0x6(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x0(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x2(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x4(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x6(gb), 0, 4
            }

            /* pixel row 1, with chroma row 0 */
            luma = yp[ystride];
            cr_r0 = cr_r[cr >> 8];
            cr_g0 = cr_g[cr >> 8];
            cb_g0 = cb_g[cb >> 8];
            cb_b0 = cb_b[cb >> 8];
            cr_r1 = cr_r[cr & 0xFF];
            cr_g1 = cr_g[cr & 0xFF];
            cb_g1 = cb_g[cb & 0xFF];
            cb_b1 = cb_b[cb & 0xFF];
            l0 = y[luma >> 24];
            l1 = y[(luma >> 16) & 0xFF];
            l2 = y[(luma >> 8) & 0xFF];
            l3 = y[luma & 0xFF];
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, alpha, red01
                psq_st pair, 0x8(ar), 0, 4
                ps_merge11 pair, alpha, red01
                psq_st pair, 0xa(ar), 0, 4
                ps_merge10 pair, alpha, red23
                psq_st pair, 0xc(ar), 0, 4
                ps_merge11 pair, alpha, red23
                psq_st pair, 0xe(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x8(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0xa(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0xc(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0xe(gb), 0, 4
            }
            yp += ystride * 2;

            /* pixel row 2, with chroma row 1 */
            luma = yp[0];
            cb = *cbp1;
            cr = *crp1;
            cr_r0 = cr_r[cr >> 8];
            cr_g0 = cr_g[cr >> 8];
            cb_g0 = cb_g[cb >> 8];
            cb_b0 = cb_b[cb >> 8];
            cr_r1 = cr_r[cr & 0xFF];
            cr_g1 = cr_g[cr & 0xFF];
            cb_g1 = cb_g[cb & 0xFF];
            cb_b1 = cb_b[cb & 0xFF];
            l0 = y[luma >> 24];
            l1 = y[(luma >> 16) & 0xFF];
            l2 = y[(luma >> 8) & 0xFF];
            l3 = y[luma & 0xFF];
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, alpha, red01
                psq_st pair, 0x10(ar), 0, 4
                ps_merge11 pair, alpha, red01
                psq_st pair, 0x12(ar), 0, 4
                ps_merge10 pair, alpha, red23
                psq_st pair, 0x14(ar), 0, 4
                ps_merge11 pair, alpha, red23
                psq_st pair, 0x16(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x10(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x12(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x14(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x16(gb), 0, 4
            }

            /* pixel row 3, with chroma row 1 */
            luma = yp[ystride];
            cr_r0 = cr_r[cr >> 8];
            cr_g0 = cr_g[cr >> 8];
            cb_g0 = cb_g[cb >> 8];
            cb_b0 = cb_b[cb >> 8];
            cr_r1 = cr_r[cr & 0xFF];
            cr_g1 = cr_g[cr & 0xFF];
            cb_g1 = cb_g[cb & 0xFF];
            cb_b1 = cb_b[cb & 0xFF];
            l0 = y[luma >> 24];
            l1 = y[(luma >> 16) & 0xFF];
            l2 = y[(luma >> 8) & 0xFF];
            l3 = y[luma & 0xFF];
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, alpha, red01
                psq_st pair, 0x18(ar), 0, 4
                ps_merge11 pair, alpha, red01
                psq_st pair, 0x1a(ar), 0, 4
                ps_merge10 pair, alpha, red23
                psq_st pair, 0x1c(ar), 0, 4
                ps_merge11 pair, alpha, red23
                psq_st pair, 0x1e(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x18(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x1a(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x1c(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x1e(gb), 0, 4
            }
            yp = yp - ystride * 2 + 1;
            cbp0++;
            crp0++;
            cbp1++;
            crp1++;
            ar += 16;
            gb += 16;
        }
        yp += ystride * 3;
        cbrow += cstride * 2;
        crrow += cstride * 2;
        ar += dskip;
        gb += dskip;
    }
    gqr = gqr_save;
    asm {
        mtspr GQR4, gqr
    }
}

/* Table-driven converter: the values come from the CFT_ARGBENT entries of tbl, the Y entries
 * first, then 256 Cb and 256 Cr entries. The luma value is the red member of the Y entry and the
 * alpha its alpha member; Cr contributes its red and green members, Cb its green and blue
 * members. */
void cnvDynamicYcc420plnToArgb8888(const CFT_YCC420PLN *src, const CFT_ARGBDST *dst,
                                   const Uint8 *tbl) {
    const CFT_ARGBENT *ytbl = (const CFT_ARGBENT *)tbl;
    const CFT_ARGBENT *cbtbl = ytbl + 256;
    const CFT_ARGBENT *crtbl = ytbl + 512;
    Sint32 wblk = src->ywidth / 4;
    Sint32 hblk = dst->height / 4;
    Sint32 dskip = (dst->pitch - src->ywidth) / 4 * 16; /* in words, 16 per tile */
    Uint32 ystride = (Uint32)src->ywidth >> 2;          /* luma words per row */
    Uint32 cstride = (Uint32)src->cbwidth >> 1;         /* chroma halfwords per row */
    const Uint32 *yp = (const Uint32 *)src->y;
    const Uint16 *cbrow = (const Uint16 *)src->cb;
    const Uint16 *crrow = (const Uint16 *)src->cr;
    register Uint32 *ar = dst->buf;     /* AR half of the tile */
    register Uint32 *gb = dst->buf + 8; /* GB half of the tile */
    const Uint16 *cbp0;
    const Uint16 *crp0;
    const Uint16 *cbp1;
    const Uint16 *crp1;
    Uint32 luma;
    Uint32 cb;
    Uint32 cr;
    register Uint32 gqr;
    register Uint32 gqr_u8 = 0x00040004;
    /* table values of the four luma samples of a row (luma and alpha) and of the two chroma
     * samples of a row */
    register Float32 l0, l1, l2, l3, a0, a1, a2, a3;
    register Float32 cr_r0, cr_g0, cb_g0, cb_b0, cr_r1, cr_g1, cb_g1, cb_b1;
    /* the summed green chroma values, the luma pairs, the channel pairs and the pair stored */
    register __vec2x32float__ grn0, grn1, l01, l23, red01, grn01, blu01, red23, grn23, blu23, pair;
    Sint32 i;
    Sint32 j;

    asm {
        mfspr gqr, GQR4
    }
    gqr_save = gqr;
    asm {
        mtspr GQR4, gqr_u8
    }
    for (i = 0; i < hblk; i++) {
        cbp0 = cbrow;
        crp0 = crrow;
        cbp1 = cbrow + cstride;
        crp1 = crrow + cstride;
        for (j = 0; j < wblk; j++) {
            /* pixel row 0, with chroma row 0; the plane words are read before the dcbz */
            luma = yp[0];
            cb = *cbp0;
            cr = *crp0;
            __dcbz(ar, 0);
            __dcbz(gb, 0);
            cr_r0 = crtbl[cr >> 8].r;
            cr_g0 = crtbl[cr >> 8].g;
            cb_g0 = cbtbl[cb >> 8].g;
            cb_b0 = cbtbl[cb >> 8].b;
            cr_r1 = crtbl[cr & 0xFF].r;
            cr_g1 = crtbl[cr & 0xFF].g;
            cb_g1 = cbtbl[cb & 0xFF].g;
            cb_b1 = cbtbl[cb & 0xFF].b;
            l0 = ytbl[luma >> 24].r;
            l1 = ytbl[(luma >> 16) & 0xFF].r;
            l2 = ytbl[(luma >> 8) & 0xFF].r;
            l3 = ytbl[luma & 0xFF].r;
            a0 = ytbl[luma >> 24].a;
            a1 = ytbl[(luma >> 16) & 0xFF].a;
            a2 = ytbl[(luma >> 8) & 0xFF].a;
            a3 = ytbl[luma & 0xFF].a;
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, a0, red01
                psq_st pair, 0x0(ar), 0, 4
                ps_merge11 pair, a1, red01
                psq_st pair, 0x2(ar), 0, 4
                ps_merge10 pair, a2, red23
                psq_st pair, 0x4(ar), 0, 4
                ps_merge11 pair, a3, red23
                psq_st pair, 0x6(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x0(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x2(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x4(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x6(gb), 0, 4
            }

            /* pixel row 1, with chroma row 0 */
            luma = yp[ystride];
            cr_r0 = crtbl[cr >> 8].r;
            cr_g0 = crtbl[cr >> 8].g;
            cb_g0 = cbtbl[cb >> 8].g;
            cb_b0 = cbtbl[cb >> 8].b;
            cr_r1 = crtbl[cr & 0xFF].r;
            cr_g1 = crtbl[cr & 0xFF].g;
            cb_g1 = cbtbl[cb & 0xFF].g;
            cb_b1 = cbtbl[cb & 0xFF].b;
            l0 = ytbl[luma >> 24].r;
            l1 = ytbl[(luma >> 16) & 0xFF].r;
            l2 = ytbl[(luma >> 8) & 0xFF].r;
            l3 = ytbl[luma & 0xFF].r;
            a0 = ytbl[luma >> 24].a;
            a1 = ytbl[(luma >> 16) & 0xFF].a;
            a2 = ytbl[(luma >> 8) & 0xFF].a;
            a3 = ytbl[luma & 0xFF].a;
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, a0, red01
                psq_st pair, 0x8(ar), 0, 4
                ps_merge11 pair, a1, red01
                psq_st pair, 0xa(ar), 0, 4
                ps_merge10 pair, a2, red23
                psq_st pair, 0xc(ar), 0, 4
                ps_merge11 pair, a3, red23
                psq_st pair, 0xe(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x8(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0xa(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0xc(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0xe(gb), 0, 4
            }
            yp += ystride * 2;

            /* pixel row 2, with chroma row 1 */
            luma = yp[0];
            cb = *cbp1;
            cr = *crp1;
            cr_r0 = crtbl[cr >> 8].r;
            cr_g0 = crtbl[cr >> 8].g;
            cb_g0 = cbtbl[cb >> 8].g;
            cb_b0 = cbtbl[cb >> 8].b;
            cr_r1 = crtbl[cr & 0xFF].r;
            cr_g1 = crtbl[cr & 0xFF].g;
            cb_g1 = cbtbl[cb & 0xFF].g;
            cb_b1 = cbtbl[cb & 0xFF].b;
            l0 = ytbl[luma >> 24].r;
            l1 = ytbl[(luma >> 16) & 0xFF].r;
            l2 = ytbl[(luma >> 8) & 0xFF].r;
            l3 = ytbl[luma & 0xFF].r;
            a0 = ytbl[luma >> 24].a;
            a1 = ytbl[(luma >> 16) & 0xFF].a;
            a2 = ytbl[(luma >> 8) & 0xFF].a;
            a3 = ytbl[luma & 0xFF].a;
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, a0, red01
                psq_st pair, 0x10(ar), 0, 4
                ps_merge11 pair, a1, red01
                psq_st pair, 0x12(ar), 0, 4
                ps_merge10 pair, a2, red23
                psq_st pair, 0x14(ar), 0, 4
                ps_merge11 pair, a3, red23
                psq_st pair, 0x16(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x10(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x12(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x14(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x16(gb), 0, 4
            }

            /* pixel row 3, with chroma row 1 */
            luma = yp[ystride];
            cr_r0 = crtbl[cr >> 8].r;
            cr_g0 = crtbl[cr >> 8].g;
            cb_g0 = cbtbl[cb >> 8].g;
            cb_b0 = cbtbl[cb >> 8].b;
            cr_r1 = crtbl[cr & 0xFF].r;
            cr_g1 = crtbl[cr & 0xFF].g;
            cb_g1 = cbtbl[cb & 0xFF].g;
            cb_b1 = cbtbl[cb & 0xFF].b;
            l0 = ytbl[luma >> 24].r;
            l1 = ytbl[(luma >> 16) & 0xFF].r;
            l2 = ytbl[(luma >> 8) & 0xFF].r;
            l3 = ytbl[luma & 0xFF].r;
            a0 = ytbl[luma >> 24].a;
            a1 = ytbl[(luma >> 16) & 0xFF].a;
            a2 = ytbl[(luma >> 8) & 0xFF].a;
            a3 = ytbl[luma & 0xFF].a;
            asm {
                ps_add grn0, cr_g0, cb_g0
                ps_add grn1, cr_g1, cb_g1
                ps_merge11 l01, l0, l1
                ps_merge11 l23, l2, l3
                ps_add red01, l01, cr_r0
                ps_add grn01, l01, grn0
                ps_add blu01, l01, cb_b0
                ps_add red23, l23, cr_r1
                ps_add grn23, l23, grn1
                ps_add blu23, l23, cb_b1
                ps_merge10 pair, a0, red01
                psq_st pair, 0x18(ar), 0, 4
                ps_merge11 pair, a1, red01
                psq_st pair, 0x1a(ar), 0, 4
                ps_merge10 pair, a2, red23
                psq_st pair, 0x1c(ar), 0, 4
                ps_merge11 pair, a3, red23
                psq_st pair, 0x1e(ar), 0, 4
                ps_merge00 pair, grn01, blu01
                psq_st pair, 0x18(gb), 0, 4
                ps_merge11 pair, grn01, blu01
                psq_st pair, 0x1a(gb), 0, 4
                ps_merge00 pair, grn23, blu23
                psq_st pair, 0x1c(gb), 0, 4
                ps_merge11 pair, grn23, blu23
                psq_st pair, 0x1e(gb), 0, 4
            }
            yp = yp - ystride * 2 + 1;
            cbp0++;
            crp0++;
            cbp1++;
            crp1++;
            ar += 16;
            gb += 16;
        }
        yp += ystride * 3;
        cbrow += cstride * 2;
        crrow += cstride * 2;
        ar += dskip;
        gb += dskip;
    }
    gqr = gqr_save;
    asm {
        mtspr GQR4, gqr
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
