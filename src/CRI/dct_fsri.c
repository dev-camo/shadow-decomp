/* CRI DCT library, paired-single IDCT (dct_fsri.c): the 8x8 inverse DCT of the six blocks of a
 * macroblock for the MPV decoder. Blocks with only a DC coefficient are filled directly; the others
 * go through a separable paired-single kernel (row pass into the work buffer, column pass stored as
 * 16-bit values through GQR7). The kernels are hand-written paired-single assembly inside the C
 * function. The coefficients arrive pre-scaled (DCT_FsriInitScaleTbl) and with two rows of a block
 * interleaved into one 16-entry row (DCT_FsriInitScanTbl), the order the kernel loads in pairs.
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/dct.h"

/* Kernel multipliers, each in both halves of a pair: sqrt(2), 1/cos(3pi/8), 1/cos(pi/8) and
 * 2cos(3pi/8); then the +0.5 and -0.5 pairs that ps_sel picks by sign to round the results. */
Float32 B0TableOrg[12] = {
    1.4142135381698608f,
    1.4142135381698608f,
    2.613126039505005f,
    2.613126039505005f,
    1.0823922157287598f,
    1.0823922157287598f,
    0.7653668522834778f,
    0.7653668522834778f,
    0.5f,
    0.5f,
    -0.5f,
    -0.5f,
};

Float32 PreIDCT[64][64];
Float64 sfsd_scale_tbl[64];
const Char8 *dctfsri_version_dummy;

static const Float64 scale8[8] = {
    0.3535533905932738, 0.4903926402016152, 0.46193976625564337, 0.4157348061512726,
    0.3535533905932738, 0.2777851165098011, 0.1913417161825449,  0.09754516100806414,
};

/* IDCT of the macroblock's blocks selected by `cbp` (block 0 in the sign bit, -1 = all six).
 * DC-only blocks are filled with their rounded DC value; the others go through the paired-single
 * row pass into the work buffer and the column pass, stored as 16-bit values through GQR7 into
 * the output blocks of the DCT parameter block.
 */
void DCT_FsriTransCore(DCT_PA *pa, Sint32 cbp) {
    register Sint8 *flg = pa->cbp;
    register Float32 *blk;
    register Sint16 **tbl;
    register Uint8 *work;
    register Float32 *src;
    register Uint8 *dst;
    register Sint32 cnt;
    register Sint16 *o;
    register Float32 *p;
    /* the kernel's paired-single values: the multipliers c1..c6 and the temporaries (the
     * compiler assigns their registers) */
    register __vec2x32float__ t0, c1, c2, c3, c4, c5, c6, t1, t2, t3, t4, t5, t6, t7, t8;
    Sint32 i;
    int k;
    Float32 dc;
    Sint32 n;
    Uint32 *out;

    work = pa->work;
    p = B0TableOrg;
    asm {
        psq_lu c1, 0x0(p), 0, 0
        psq_lu c2, 0x8(p), 0, 0
        psq_lu c3, 0x8(p), 0, 0
        psq_lu c4, 0x8(p), 0, 0
        psq_lu c5, 0x8(p), 0, 0
        psq_lu c6, 0x8(p), 0, 0
    }
    blk = pa->blk;
    tbl = pa->tbl;
    for (i = 0; i < 6; i++) {
        if (cbp < 0) {
            if (flg[0] == 0) {
                /* the 64 16-bit outputs are filled as 32 words holding the value twice */
                dc = blk[0];
                out = (Uint32 *)tbl[0];
                if (dc < 0.0) {
                    n = (Sint16)(dc - 0.5f);
                } else {
                    n = (Sint16)(0.5f + dc);
                }
                n = (n << 16) | (Uint16)n;
                out += 32;
                for (k = 0; k < 32; k++) {
                    *--out = n;
                }
            } else {
                o = tbl[0];
                asm {
        subi src, blk, 0x8
        subi dst, work, 0x8
        li cnt, 0x4
row_loop:
        psq_lu t0, 0x8(src), 0, 0
        psq_lu t1, 0x8(src), 0, 0
        psq_lu t2, 0x8(src), 0, 0
        psq_lu t3, 0x8(src), 0, 0
        psq_lu t4, 0x8(src), 0, 0
        psq_lu t5, 0x8(src), 0, 0
        psq_lu t6, 0x8(src), 0, 0
        psq_lu t7, 0x8(src), 0, 0
        ps_sub t8, t2, t6
        ps_add t6, t2, t6
        ps_sub t2, t0, t4
        ps_mul t8, t8, c1
        ps_add t4, t0, t4
        ps_sub t8, t8, t6
        ps_sub t0, t4, t6
        ps_add t6, t4, t6
        ps_sub t4, t2, t8
        ps_add t8, t2, t8
        ps_sub t2, t5, t3
        ps_add t3, t5, t3
        ps_sub t5, t1, t7
        ps_add t7, t1, t7
        ps_sub t1, t7, t3
        ps_add t3, t7, t3
        ps_sub t7, t2, t5
        ps_mul t5, t5, c3
        ps_mul t2, t2, c2
        ps_mul t7, t7, c4
        ps_mul t1, t1, c1
        ps_sub t5, t5, t7
        ps_sub t2, t2, t7
        ps_sub t7, t6, t3
        ps_sub t5, t5, t3
        ps_add t3, t6, t3
        ps_sub t1, t1, t5
        ps_sub t6, t8, t5
        ps_add t5, t8, t5
        ps_sub t2, t2, t1
        ps_sub t8, t0, t2
        ps_add t2, t0, t2
        ps_sub t0, t4, t1
        ps_add t1, t4, t1
        ps_merge00 t4, t3, t5
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge00 t4, t1, t2
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge00 t4, t8, t0
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge00 t4, t6, t7
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge11 t4, t3, t5
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge11 t4, t1, t2
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge11 t4, t8, t0
        psq_stu t4, 0x8(dst), 0, 0
        ps_merge11 t4, t6, t7
        subic. cnt, cnt, 0x1
        psq_stu t4, 0x8(dst), 0, 0
        bgt row_loop
        mr dst, o
        mr src, work
        li cnt, 0x4
col_loop:
        psq_lu t0, 0x0(src), 0, 0
        psq_lu t5, 0x20(src), 0, 0
        psq_lu t6, 0x20(src), 0, 0
        psq_lu t3, 0x20(src), 0, 0
        psq_lu t8, 0x20(src), 0, 0
        psq_lu t2, 0x20(src), 0, 0
        psq_lu t7, 0x20(src), 0, 0
        psq_lu t1, 0x20(src), 0, 0
        ps_sub t4, t6, t7
        ps_add t7, t6, t7
        ps_sub t6, t0, t8
        ps_mul t4, t4, c1
        ps_add t8, t0, t8
        ps_sub t4, t4, t7
        ps_sub t0, t8, t7
        ps_add t7, t8, t7
        ps_sub t8, t6, t4
        ps_add t4, t6, t4
        ps_sub t6, t2, t3
        ps_add t3, t2, t3
        ps_sub t2, t5, t1
        ps_add t1, t5, t1
        ps_sub t5, t1, t3
        ps_add t3, t1, t3
        ps_sub t1, t6, t2
        ps_mul t2, t2, c3
        ps_mul t6, t6, c2
        ps_mul t1, t1, c4
        ps_mul t5, t5, c1
        ps_sub t2, t2, t1
        ps_sub t6, t6, t1
        ps_sub t1, t7, t3
        ps_sub t2, t2, t3
        ps_add t3, t7, t3
        ps_sub t5, t5, t2
        ps_sub t7, t4, t2
        ps_add t2, t4, t2
        ps_sub t6, t6, t5
        ps_sub t4, t0, t6
        ps_add t6, t0, t6
        ps_sub t0, t8, t5
        ps_add t5, t8, t5
        ps_sel t8, t3, c5, c6
        ps_add t3, t3, t8
        ps_sel t8, t2, c5, c6
        ps_add t2, t2, t8
        ps_sel t8, t5, c5, c6
        ps_add t5, t5, t8
        psq_stu t3, 0x0(dst), 0, 7
        ps_sel t8, t6, c5, c6
        ps_add t6, t6, t8
        psq_stu t2, 0x10(dst), 0, 7
        ps_sel t8, t4, c5, c6
        psq_stu t5, 0x10(dst), 0, 7
        ps_add t4, t4, t8
        psq_stu t6, 0x10(dst), 0, 7
        ps_sel t8, t0, c5, c6
        ps_add t0, t0, t8
        ps_sel t8, t7, c5, c6
        psq_stu t4, 0x10(dst), 0, 7
        ps_add t7, t7, t8
        psq_stu t0, 0x10(dst), 0, 7
        ps_sel t8, t1, c5, c6
        psq_stu t7, 0x10(dst), 0, 7
        ps_add t1, t1, t8
        subic. cnt, cnt, 0x1
        psq_stu t1, 0x10(dst), 0, 7
        ble done
        subi src, src, 0xd8
        subi dst, dst, 0x6c
        b col_loop
    done:
                }
            }
        }
        cbp <<= 1;
        blk += 64;
        tbl++;
        flg++;
    }
}

/* Interleaved position of coefficient i (raster order): rows 2m and 2m+1 of a block become one
 * 16-entry row, the even row in the even slots. An out-of-range position hangs. (name inferred) */
static inline int dctfsri_Idx(int i) {
    int q = i / 8;
    int r = i % 8;
    int n;

    if (q % 2 == 0) {
        r *= 2;
    } else {
        q--;
        r = r * 2 + 1;
    }
    n = r + q * 8;
    if (n < 0 || n >= 256) {
        for (;;) {
        }
    }
    return n;
}

/* Maps each raster position in seq to its interleaved position. */
void DCT_FsriInitScanTbl(const Sint8 *seq, Sint8 *scan) {
    int i;

    for (i = 0; i < 64; i++) {
        scan[i] = (Sint8)dctfsri_Idx(seq[i]);
    }
}

/* IDCT of the blocks flagged in the coded block pattern (non-intra macroblocks). */
void DCT_FsriTransCbp(DCT_PA *pa) {
    DCT_FsriTransCore(pa, pa->cbp_msk);
}

/* IDCT of all six blocks (intra macroblocks). */
void DCT_FsriTrans6Blk(DCT_PA *pa) {
    DCT_FsriTransCore(pa, -1);
}

/* Sets GQR7 to signed 16-bit loads and stores, scale 0 (the column pass stores through it). */
void DCT_FsriSetGqr(void) {
    asm {
        li r0, 0x7
        oris r0, r0, 0x7
        mtspr GQR7, r0
    }
}

/* Fills PreIDCT: row Idx(n) holds the double-precision reference IDCT of a block whose only
 * coefficient is n, set to 1 / sfsd_scale_tbl[n]. Nothing in this file reads the table.
 *
 * Matching workaround: opt_loop_invariants off. The original recomputes the row index
 * dctfsri_Idx(n) for every stored element; by default MWCC hoists it out of the inner loop
 * (initSparseTbl 143 words instead of the original's 148, 75 of them differing). Revisit when the
 * original's compiler flags or loop form are known. */
#pragma opt_loop_invariants off
void initSparseTbl(void) {
    Float64 in[64];
    Float64 out[64];
    int n;

    memset(PreIDCT, 0, sizeof(PreIDCT));
    DCT_AcInit();
    for (n = 0; n < 64; n++) {
        int k;
        for (k = 0; k < 64; k++) {
            if (k == n) {
                in[k] = 1.0 / sfsd_scale_tbl[k];
            } else {
                in[k] = 0.0;
            }
        }
        DCT_AcIdctDouble(in, out);
        for (k = 0; k < 64; k++) {
            PreIDCT[dctfsri_Idx(n)][k] = (Float32)out[k];
        }
    }
}
#pragma opt_loop_invariants reset

/* Clears a DCT parameter block. */
void DCT_FsriInitPa(DCT_PA *pa) {
    memset(pa, 0, sizeof(DCT_PA));
}

/* Writes sfsd_scale_tbl to tbl in single precision and interleaved order. */
void DCT_FsriInitScaleTbl(Float32 *tbl) {
    int i;

    for (i = 0; i < 64; i++) {
        tbl[dctfsri_Idx(i)] = (Float32)sfsd_scale_tbl[i];
    }
}

/* Library init: the 8x8 separable scale table (scale8 outer product) and the PreIDCT table. */
void DCT_FsriInit(void) {
    Sint32 i;
    Sint32 j;

    dctfsri_version_dummy = DCT_GetVerStr();
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            sfsd_scale_tbl[i * 8 + j] = scale8[i] * scale8[j];
        }
    }
    initSparseTbl();
}
