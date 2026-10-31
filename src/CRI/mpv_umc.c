/* CRI Sofdec MPV motion compensation front end (mpv_umc.c): builds each macroblock from the
 * reference frames through the 8x8 (chroma) and 16x16 (luma) one-reference kernels of mpv_mc.c and
 * mpv_mcy.c, adds the IDCT output and writes it into the output frame; skipped macroblocks are
 * copied from the reference. The merge and intra output kernels and the GQR setup are hand-written
 * paired-single assembly (quantised psq loads and stores through GQR3-5, ps_merge, mtspr).
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mpv.h"

typedef void (*MPVUMC_MCFUNC)(MPVMC *mc);

/* 1.0, loaded into both halves of a paired single by mpvumc_BiMakeMb (name inferred) */
static const Float32 mpvumc_one = 1.0f;

/* half-pel kernels by [cond[3]][vertical half][horizontal half]: 16x16 luma and 8x8 chroma */
MPVUMC_MCFUNC mpvumc_oneref_y[2][2][2];
static MPVUMC_MCFUNC mpvumc_oneref[2][2][2];

/* B picture: the n - 1 macroblocks skipped before the current one repeat the previous
 * macroblock's prediction (mc_func[0]) without a residual. */
void MPVUMC_BpicSkipped(MPV mpv, Sint32 n) {
    Sint32 mb_end;
    void (*func)(MPV mpv);

    mpv->cbp_code = 0;
    mb_end = mpv->mb_addr;
    func = mpv->mc_func[0];
    mpv->mb_addr -= n - 1;
    mpv->mb_x -= n - 1;
    while (mpv->mb_x < 0) {
        mpv->mb_x += mpv->picatr.mb_width;
        mpv->mb_y--;
    }
    while (mpv->mb_addr < mb_end) {
        func(mpv);
        mpv->mb_x++;
        if (mpv->mb_x >= mpv->picatr.mb_width) {
            mpv->mb_x = 0;
            mpv->mb_y++;
        }
        mpv->mb_addr++;
    }
}

/* two chroma rows (one double each) / one luma row (two doubles) from s to d through the
 * temporaries a and b, with and without clearing the destination cache line first; p is the pitch
 * in doubles */
#define MPVUMC_CPY_C2Z(d, s, p, a, b)                                                              \
    do {                                                                                           \
        __dcbz((d), 0);                                                                            \
        (a) = (s)[0];                                                                              \
        __dcbz((d), (p) * 8);                                                                      \
        (b) = (s)[(p)];                                                                            \
        (s) += (p) * 2;                                                                            \
        (d)[0] = (a);                                                                              \
        (d)[(p)] = (b);                                                                            \
        (d) += (p) * 2;                                                                            \
    } while (0)
#define MPVUMC_CPY_C2(d, s, p, a, b)                                                               \
    do {                                                                                           \
        (a) = (s)[0];                                                                              \
        (b) = (s)[(p)];                                                                            \
        (s) += (p) * 2;                                                                            \
        (d)[0] = (a);                                                                              \
        (d)[(p)] = (b);                                                                            \
        (d) += (p) * 2;                                                                            \
    } while (0)
#define MPVUMC_CPY_YZ(d, s, p, a, b)                                                               \
    do {                                                                                           \
        __dcbz((d), 0);                                                                            \
        (a) = (s)[0];                                                                              \
        (b) = (s)[1];                                                                              \
        (s) += (p);                                                                                \
        (d)[0] = (a);                                                                              \
        (d)[1] = (b);                                                                              \
        (d) += (p);                                                                                \
    } while (0)
#define MPVUMC_CPY_Y(d, s, p, a, b)                                                                \
    do {                                                                                           \
        (a) = (s)[0];                                                                              \
        (b) = (s)[1];                                                                              \
        (s) += (p);                                                                                \
        (d)[0] = (a);                                                                              \
        (d)[1] = (b);                                                                              \
        (d) += (p);                                                                                \
    } while (0)

/* Copies the macroblock at ofs[] (chroma, luma offset) from src to dst, two chroma rows or one
 * luma row per step, clearing each destination cache line first when the offset is 32-byte
 * aligned. */
void mpvumc_PpicSkipMb(Sint32 *ofs, MPV_RFB *src, MPV_RFB *dst) {
    Sint32 p;
    Float64 *s;
    Float64 *d;
    Float64 a;
    Float64 b;

    p = dst->cpitch / 8;
    if ((ofs[0] & 0x1f) == 0) {
        s = (Float64 *)(src->pln[0] + ofs[0]);
        d = (Float64 *)(dst->pln[0] + ofs[0]);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        s = (Float64 *)(src->pln[1] + ofs[0]);
        d = (Float64 *)(dst->pln[1] + ofs[0]);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
        MPVUMC_CPY_C2Z(d, s, p, a, b);
    } else {
        s = (Float64 *)(src->pln[0] + ofs[0]);
        d = (Float64 *)(dst->pln[0] + ofs[0]);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
        s = (Float64 *)(src->pln[1] + ofs[0]);
        d = (Float64 *)(dst->pln[1] + ofs[0]);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
        MPVUMC_CPY_C2(d, s, p, a, b);
    }
    p = dst->ypitch / 8;
    if ((ofs[1] & 0x1f) == 0) {
        s = (Float64 *)(src->pln[2] + ofs[1]);
        d = (Float64 *)(dst->pln[2] + ofs[1]);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
        MPVUMC_CPY_YZ(d, s, p, a, b);
    } else {
        s = (Float64 *)(src->pln[2] + ofs[1]);
        d = (Float64 *)(dst->pln[2] + ofs[1]);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
        MPVUMC_CPY_Y(d, s, p, a, b);
    }
}

/* P picture: the n - 1 macroblocks skipped before the current one are copied from frm.rfb[0] to
 * frm.rfb[1]. */
void MPVUMC_PpicSkipped(MPV mpv, Sint32 n) {
    Sint32 mb_end;
    MPV_RFB *src;
    MPV_RFB *dst;
    Sint32 ofs[2];
    Sint32 y8;
    Sint32 y16;

    src = &mpv->frm.rfb[0];
    mb_end = mpv->mb_addr;
    dst = src + 1;
    mpv->mb_addr -= n - 1;
    mpv->mb_x -= n - 1;
    while (mpv->mb_x < 0) {
        mpv->mb_x += mpv->picatr.mb_width;
        mpv->mb_y--;
    }
    while (mpv->mb_addr < mb_end) {
        y8 = mpv->mb_y * 8;
        y16 = y8 * 2;
        ofs[0] = mpv->mb_x * 8 + y8 * src->cpitch;
        ofs[1] = mpv->mb_x * 16 + y16 * src->ypitch;
        mpvumc_PpicSkipMb(ofs, src, dst);
        mpv->mb_x++;
        if (mpv->mb_x >= mpv->picatr.mb_width) {
            mpv->mb_x = 0;
            mpv->mb_y++;
        }
        mpv->mb_addr++;
    }
}

/* Averages the two predictions (wk->work, wk->work2) of the macroblock, adds the IDCT blocks
 * (wk->buf0) of the blocks coded in cbp and stores the six blocks through ob. */
void mpvumc_BiMakeMb(MPVMC_WORK *wk, MPVCMC_OUTBLK *ob, Sint32 cbp) {
    asm {
        lis r6, mpvumc_one@ha
        lwz r8, MPVMC_WORK.buf0(r3)
        addi r6, r6, mpvumc_one@l
        lwz r7, MPVMC_WORK.work(r3)
        psq_l f8, 0x0(r6), 1, 0
        addi r9, r4, MPVCMC_OUTBLK.oi
        lwz r3, MPVMC_WORK.work2(r3)
        li r10, 0x0
        ps_merge11 f8, f8, f8
L00:
        cmpwi r5, 0x0
        lwz r4, MPVCMC_REF.p(r9)
        lwz r6, MPVCMC_REF.n(r9)
        addi r9, r9, 0x8
        blt L04
        clrlwi. r0, r4, 27
        bne L02
        li r0, 0x4
        mtctr r0
        addi r8, r8, 0x80
L01:
        dcbz r0, r4
        psq_l f0, 0x0(r7), 0, 4
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x2(r7), 0, 4
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x4(r7), 0, 4
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0x6(r7), 0, 4
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        dcbz r0, r4
        psq_l f0, 0x8(r7), 0, 4
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0xa(r7), 0, 4
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0xc(r7), 0, 4
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0xe(r7), 0, 4
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        addi r7, r7, 0x10
        addi r3, r3, 0x10
        bdnz L01
        b L08
L02:
        li r0, 0x4
        mtctr r0
        addi r8, r8, 0x80
L03:
        psq_l f0, 0x0(r7), 0, 4
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x2(r7), 0, 4
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x4(r7), 0, 4
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0x6(r7), 0, 4
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        psq_l f0, 0x8(r7), 0, 4
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0xa(r7), 0, 4
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0xc(r7), 0, 4
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0xe(r7), 0, 4
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        addi r7, r7, 0x10
        addi r3, r3, 0x10
        bdnz L03
        b L08
L04:
        clrlwi. r0, r4, 27
        bne L06
        li r0, 0x4
        mtctr r0
L05:
        dcbz r0, r4
        psq_l f0, 0x0(r7), 0, 4
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x2(r7), 0, 4
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x4(r7), 0, 4
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0x6(r7), 0, 4
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_l f4, 0x0(r8), 0, 5
        psq_l f5, 0x4(r8), 0, 5
        psq_l f6, 0x8(r8), 0, 5
        psq_l f7, 0xc(r8), 0, 5
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        dcbz r0, r4
        psq_l f0, 0x8(r7), 0, 4
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0xa(r7), 0, 4
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0xc(r7), 0, 4
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0xe(r7), 0, 4
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_l f4, 0x10(r8), 0, 5
        psq_l f5, 0x14(r8), 0, 5
        psq_l f6, 0x18(r8), 0, 5
        psq_l f7, 0x1c(r8), 0, 5
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        addi r7, r7, 0x10
        addi r3, r3, 0x10
        addi r8, r8, 0x20
        bdnz L05
        b L08
L06:
        li r0, 0x4
        mtctr r0
L07:
        psq_l f0, 0x0(r7), 0, 4
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x2(r7), 0, 4
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x4(r7), 0, 4
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0x6(r7), 0, 4
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_l f4, 0x0(r8), 0, 5
        psq_l f5, 0x4(r8), 0, 5
        psq_l f6, 0x8(r8), 0, 5
        psq_l f7, 0xc(r8), 0, 5
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        psq_l f0, 0x8(r7), 0, 4
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0xa(r7), 0, 4
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0xc(r7), 0, 4
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0xe(r7), 0, 4
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_l f4, 0x10(r8), 0, 5
        psq_l f5, 0x14(r8), 0, 5
        psq_l f6, 0x18(r8), 0, 5
        psq_l f7, 0x1c(r8), 0, 5
        ps_add f0, f0, f8
        ps_add f1, f1, f8
        ps_add f2, f2, f8
        ps_add f3, f3, f8
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 5
        psq_st f1, 0x2(r4), 0, 5
        psq_st f2, 0x4(r4), 0, 5
        psq_st f3, 0x6(r4), 0, 5
        add r4, r4, r6
        addi r7, r7, 0x10
        addi r3, r3, 0x10
        addi r8, r8, 0x20
        bdnz L07
L08:
        addi r10, r10, 0x1
        slwi r5, r5, 1
        cmpwi r10, 0x6
        blt L00
    }
}

/* Adds the IDCT blocks (wk->buf0) of the blocks coded in cbp to the prediction (wk->work) and
 * stores the six blocks through ob. */
void mpvumc_OneMakeMb(MPVMC_WORK *wk, MPVCMC_OUTBLK *ob, Sint32 cbp) {
    asm {
        lwz r6, MPVMC_WORK.buf0(r3)
        addi r7, r4, MPVCMC_OUTBLK.oi
        lwz r3, MPVMC_WORK.work(r3)
        li r9, 0x0
L09:
        cmpwi r5, 0x0
        lwz r4, MPVCMC_REF.p(r7)
        lwz r8, MPVCMC_REF.n(r7)
        addi r7, r7, 0x8
        blt L12
        clrlwi. r0, r4, 27
        addi r6, r6, 0x80
        srwi r0, r8, 3
        bne L10
        dcbz r0, r4
        slwi r0, r0, 3
        lfd f8, 0x0(r3)
        add r8, r4, r0
        lfd f9, 0x8(r3)
        lfd f10, 0x10(r3)
        lfd f11, 0x18(r3)
        stfd f8, 0x0(r4)
        dcbz r0, r8
        stfd f9, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        stfd f10, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        stfd f11, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        lfd f9, 0x28(r3)
        lfd f10, 0x30(r3)
        lfd f11, 0x38(r3)
        lfd f8, 0x20(r3)
        stfd f8, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        stfd f9, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        stfd f10, 0x0(r8)
        add r8, r8, r0
        dcbz r0, r8
        stfd f11, 0x0(r8)
        b L11
L10:
        lfd f8, 0x0(r3)
        slwi r0, r0, 3
        lfd f9, 0x8(r3)
        add r8, r4, r0
        lfd f10, 0x10(r3)
        lfd f11, 0x18(r3)
        stfd f8, 0x0(r4)
        stfd f9, 0x0(r8)
        add r8, r8, r0
        stfd f10, 0x0(r8)
        add r8, r8, r0
        stfd f11, 0x0(r8)
        add r8, r8, r0
        lfd f9, 0x28(r3)
        lfd f10, 0x30(r3)
        lfd f11, 0x38(r3)
        lfd f8, 0x20(r3)
        stfd f8, 0x0(r8)
        add r8, r8, r0
        stfd f9, 0x0(r8)
        add r8, r8, r0
        stfd f10, 0x0(r8)
        stfdx f11, r8, r0
L11:
        addi r3, r3, 0x40
        b L16
L12:
        clrlwi. r0, r4, 27
        bne L14
        li r0, 0x4
        mtctr r0
L13:
        dcbz r0, r4
        psq_l f0, 0x0(r6), 0, 3
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x4(r6), 0, 3
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x8(r6), 0, 3
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0xc(r6), 0, 3
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 4
        psq_st f1, 0x2(r4), 0, 4
        psq_st f2, 0x4(r4), 0, 4
        psq_st f3, 0x6(r4), 0, 4
        add r4, r4, r8
        dcbz r0, r4
        psq_l f0, 0x10(r6), 0, 3
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0x14(r6), 0, 3
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0x18(r6), 0, 3
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0x1c(r6), 0, 3
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 4
        psq_st f1, 0x2(r4), 0, 4
        psq_st f2, 0x4(r4), 0, 4
        psq_st f3, 0x6(r4), 0, 4
        add r4, r4, r8
        addi r3, r3, 0x10
        addi r6, r6, 0x20
        bdnz L13
        b L16
L14:
        li r0, 0x4
        mtctr r0
L15:
        psq_l f0, 0x0(r6), 0, 3
        psq_l f4, 0x0(r3), 0, 4
        psq_l f1, 0x4(r6), 0, 3
        psq_l f5, 0x2(r3), 0, 4
        psq_l f2, 0x8(r6), 0, 3
        psq_l f6, 0x4(r3), 0, 4
        psq_l f3, 0xc(r6), 0, 3
        psq_l f7, 0x6(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 4
        psq_st f1, 0x2(r4), 0, 4
        psq_st f2, 0x4(r4), 0, 4
        psq_st f3, 0x6(r4), 0, 4
        add r4, r4, r8
        psq_l f0, 0x10(r6), 0, 3
        psq_l f4, 0x8(r3), 0, 4
        psq_l f1, 0x14(r6), 0, 3
        psq_l f5, 0xa(r3), 0, 4
        psq_l f2, 0x18(r6), 0, 3
        psq_l f6, 0xc(r3), 0, 4
        psq_l f3, 0x1c(r6), 0, 3
        psq_l f7, 0xe(r3), 0, 4
        ps_add f0, f0, f4
        ps_add f1, f1, f5
        ps_add f2, f2, f6
        ps_add f3, f3, f7
        psq_st f0, 0x0(r4), 0, 4
        psq_st f1, 0x2(r4), 0, 4
        psq_st f2, 0x4(r4), 0, 4
        psq_st f3, 0x6(r4), 0, 4
        add r4, r4, r8
        addi r3, r3, 0x10
        addi r6, r6, 0x20
        bdnz L15
L16:
        addi r9, r9, 0x1
        slwi r5, r5, 1
        cmpwi r9, 0x6
        blt L09
    }
}

/* One reference's prediction of the current macroblock into dst (two 8x8 chroma blocks, then the
 * 16x16 luma block) through the half-pel kernels selected by the motion vector; ofs[] receives
 * the macroblock's chroma and luma offsets in the frame. */
void mpvumc_OneReadMb(MPV mpv, Uint8 *dst, Sint32 *ofs, MPV_RFB *rfb, MPV_MV *mv) {
    Sint32 cpitch;
    Sint32 ypitch;
    Sint32 cpos;
    Sint32 ypos;
    MPVMC *mc = &mpv->mc;
    MPVUMC_MCFUNC fn_c;
    MPVUMC_MCFUNC fn_y;
    Sint32 yhx;
    Sint32 chx;
    Uint8 *src;
    Sint32 vy;
    Sint32 vx;
    Sint32 mby;
    Sint32 mbx;
    MPVUMC_MCFUNC(*tbl_c)[2];
    MPVUMC_MCFUNC(*tbl_y)[2];
    Sint32 mode;
    Sint32 mby8;
    Sint32 mby16;
    Sint32 mbx8;

    /* Matching workaround: the unsigned masks of yhx/chx, the second read of rfb->ypitch and the
     * operand order of the luma src2 sum. With `& 1`, with ypitch instead of the second read, or
     * with src + ypitch + yhx, 45, 91 and 8 words differ. Revisit when the original expressions are
     * known. */
    mby = mpv->mb_y;
    cpitch = rfb->cpitch;
    mbx = mpv->mb_x;
    mode = mpv->cond[3];
    ypitch = rfb->ypitch;
    mby8 = mby * 8;
    mbx8 = mbx * 8;
    ofs[0] = mbx8 + mby8 * cpitch;
    mby16 = mby * 16;
    ofs[1] = mbx8 * 2 + mby16 * rfb->ypitch;
    tbl_y = mpvumc_oneref_y[mode];
    tbl_c = mpvumc_oneref[mode];
    vx = mv->vec[0];
    vy = mv->vec[1];
    ypos = ofs[1] + (vx >> 1) + (vy >> 1) * ypitch;
    yhx = vx & 1U;
    fn_y = tbl_y[vy & 1][vx & 1];
    cpos = ofs[0] + ((vx / 2) >> 1) + ((vy / 2) >> 1) * cpitch;
    chx = (vx / 2) & 1U;
    fn_c = tbl_c[(vy / 2) & 1][(vx / 2) & 1];
    chx &= mode;
    yhx &= mode;
    mc->stride = cpitch;
    mc->dst = (Uint32 *)dst;
    src = rfb->pln[0] + cpos;
    mc->src = src;
    mc->src2 = src + cpitch + chx;
    fn_c(mc);
    mc->dst = (Uint32 *)(dst + 0x40);
    src = rfb->pln[1] + cpos;
    mc->src = src;
    mc->src2 = src + cpitch + chx;
    fn_c(mc);
    mc->stride = ypitch;
    mc->dst = (Uint32 *)(dst + 0x80);
    src = rfb->pln[2] + ypos;
    mc->src = src;
    mc->src2 = ypitch + yhx + src;
    fn_y(mc);
}

/* the output block pointers of the current macroblock in the output frame (two chroma blocks,
 * four luma blocks) */
#define MPVUMC_SET_OUT_BLOCKS(mpv, ob, cofs, yofs, ypitch)                                         \
    do {                                                                                           \
        (ob)->oi[0].p = (mpv)->out.pln[0] + (cofs);                                                \
        (ob)->oi[1].p = (mpv)->out.pln[1] + (cofs);                                                \
        (ob)->oi[2].p = (mpv)->out.pln[2] + (yofs);                                                \
        (ob)->oi[3].p = (Uint8 *)(ob)->oi[2].p + 8;                                                \
        (ob)->oi[4].p = (Uint8 *)(ob)->oi[2].p + (ypitch) * 8;                                     \
        (ob)->oi[5].p = (Uint8 *)(ob)->oi[4].p + 8;                                                \
    } while (0)

/* Bidirectional macroblock: the frm.rfb[0] prediction with the forward vector and the frm.rfb[1]
 * prediction with the backward vector, averaged and added to the IDCT output. */
void MPVUMC_BiDirect(MPV mpv) {
    Sint32 ofs[2];
    MPV_RFB *rfb = mpv->frm.rfb;
    MPVMC_WORK *wk = &mpv->mc.wk;
    MPVCMC_OUTBLK *ob;

    mpvumc_OneReadMb(mpv, wk->work, ofs, rfb, &mpv->fwd);
    mpvumc_OneReadMb(mpv, wk->work2, ofs, rfb + 1, &mpv->bwd);
    ob = &mpv->ob_rt;
    MPVUMC_SET_OUT_BLOCKS(mpv, ob, ofs[0], ofs[1], mpv->out.ypitch);
    mpvumc_BiMakeMb(wk, ob, mpv->cbp_code);
}

/* Backward-predicted macroblock: the frm.rfb[1] prediction plus the IDCT output. */
void MPVUMC_Backward(MPV mpv) {
    Sint32 ofs[2];
    MPVMC_WORK *wk = &mpv->mc.wk;
    MPVCMC_OUTBLK *ob;

    mpvumc_OneReadMb(mpv, wk->work, ofs, &mpv->frm.rfb[1], &mpv->bwd);
    ob = &mpv->ob_rt;
    MPVUMC_SET_OUT_BLOCKS(mpv, ob, ofs[0], ofs[1], mpv->out.ypitch);
    mpvumc_OneMakeMb(wk, ob, mpv->cbp_code);
}

/* Forward-predicted macroblock: the frm.rfb[0] prediction plus the IDCT output. */
void MPVUMC_Forward(MPV mpv) {
    Sint32 ofs[2];
    MPVMC_WORK *wk = &mpv->mc.wk;
    MPVCMC_OUTBLK *ob;

    mpvumc_OneReadMb(mpv, wk->work, ofs, &mpv->frm.rfb[0], &mpv->fwd);
    ob = &mpv->ob_rt;
    MPVUMC_SET_OUT_BLOCKS(mpv, ob, ofs[0], ofs[1], mpv->out.ypitch);
    mpvumc_OneMakeMb(wk, ob, mpv->cbp_code);
}

/* Stores the six IDCT blocks of an intra macroblock through ob as 8-bit pixels (GQR3: signed
 * 16-bit loads, saturating unsigned 8-bit stores); clip is not read. */
void mpvumc_OutputIntra6blk(Sint16 *blk, MPVCMC_OUTBLK *ob, Uint8 *clip) {
    asm {
        addi r4, r4, MPVCMC_OUTBLK.oi
            /* Matching workaround: addi, not li (the same encoding). With li MWCC schedules the
             * preceding addi after it (2 words). Revisit when the original instruction is known. */
        addi r5, 0, 0x6
L17:
        lwz r7, MPVCMC_REF.p(r4)
        lwz r6, MPVCMC_REF.n(r4)
        addi r4, r4, 0x8
        clrlwi. r0, r7, 27
        bne L19
        li r0, 0x2
        mtctr r0
L18:
        dcbz r0, r7
        psq_l f0, 0x0(r3), 0, 3
        psq_l f1, 0x4(r3), 0, 3
        psq_l f2, 0x8(r3), 0, 3
        psq_l f3, 0xc(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        dcbz r0, r7
        psq_l f0, 0x10(r3), 0, 3
        psq_l f1, 0x14(r3), 0, 3
        psq_l f2, 0x18(r3), 0, 3
        psq_l f3, 0x1c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        dcbz r0, r7
        psq_l f0, 0x20(r3), 0, 3
        psq_l f1, 0x24(r3), 0, 3
        psq_l f2, 0x28(r3), 0, 3
        psq_l f3, 0x2c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        dcbz r0, r7
        psq_l f0, 0x30(r3), 0, 3
        psq_l f1, 0x34(r3), 0, 3
        psq_l f2, 0x38(r3), 0, 3
        psq_l f3, 0x3c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        addi r3, r3, 0x40
        bdnz L18
        b L21
L19:
        li r0, 0x2
        mtctr r0
L20:
        psq_l f0, 0x0(r3), 0, 3
        psq_l f1, 0x4(r3), 0, 3
        psq_l f2, 0x8(r3), 0, 3
        psq_l f3, 0xc(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        psq_l f0, 0x10(r3), 0, 3
        psq_l f1, 0x14(r3), 0, 3
        psq_l f2, 0x18(r3), 0, 3
        psq_l f3, 0x1c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        psq_l f0, 0x20(r3), 0, 3
        psq_l f1, 0x24(r3), 0, 3
        psq_l f2, 0x28(r3), 0, 3
        psq_l f3, 0x2c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        psq_l f0, 0x30(r3), 0, 3
        psq_l f1, 0x34(r3), 0, 3
        psq_l f2, 0x38(r3), 0, 3
        psq_l f3, 0x3c(r3), 0, 3
        psq_st f0, 0x0(r7), 0, 3
        psq_st f1, 0x2(r7), 0, 3
        psq_st f2, 0x4(r7), 0, 3
        psq_st f3, 0x6(r7), 0, 3
        add r7, r7, r6
        addi r3, r3, 0x40
        bdnz L20
L21:
        subic. r5, r5, 0x1
        bgt L17
    }
}

/* Intra macroblock: the IDCT output stored into the output frame. */
void MPVUMC_Intra(MPV mpv) {
    Sint32 mbx;
    Sint32 mby;
    Sint32 x8;
    Sint32 y8;
    Sint32 cofs;
    Sint32 yofs;
    Sint32 ypitch;
    MPVCMC_OUTBLK *ob;

    /* Matching workaround: the nested assignments. Separate statements in any order tried give 16
     * or 17 differing words. Revisit when the original expressions are known. */
    x8 = (mbx = mpv->mb_x) * 8;
    y8 = (mby = mpv->mb_y) * 8;
    cofs = x8 + y8 * mpv->out.cpitch;
    yofs = mbx * 16 + mby * 16 * (ypitch = mpv->out.ypitch);
    ob = &mpv->ob_rt;
    MPVUMC_SET_OUT_BLOCKS(mpv, ob, cofs, yofs, ypitch);
    mpvumc_OutputIntra6blk(mpv->mcbuf[0], ob, mpv->clip_base);
}

/* GQR4: unsigned 8-bit loads and stores; GQR3: signed 16-bit loads, unsigned 8-bit stores; GQR5:
 * as GQR3 with scale -1 (loads doubled, stores halved). */
void MPVUMC_SetGqr(void) {
    asm {
        li r0, 0x4
        oris r0, r0, 0x4
        mtspr GQR4, r0
        li r0, 0x4
        oris r0, r0, 0x7
        mtspr GQR3, r0
        li r0, 0x3f04
        oris r0, r0, 0x3f07
        mtspr GQR5, r0
    }
}

/* Nothing to do at the end of a frame. */
void MPVUMC_EndOfFrame(MPV_OBJ *mpv) {}

/* The output frame planes in frm.buf from the picture size: luma first, then the two chroma
 * planes; the pitches are the macroblock-rounded widths rounded up to 32 bytes. */
void MPVUMC_InitOutRfb(MPV_OBJ *mpv) {
    Sint32 w = mpv->picatr.width;
    Sint32 h = mpv->picatr.height;
    Sint32 mbw;
    Sint32 mbh;
    Sint32 yw;
    Sint32 ypitch;
    Sint32 cpitch;
    Sint32 yh;
    Uint8 *buf = mpv->frm.buf;

    mbw = (w + 15) / 16;
    yw = mbw * 16;
    ypitch = (yw + 31) / 32 * 32;
    cpitch = (yw / 2 + 31) / 32 * 32;
    mpv->out.ypitch = ypitch;
    mpv->out.cpitch = cpitch;
    mbh = (h + 15) / 16;
    yh = mbh * 16;
    mpv->out.pln[2] = buf;
    mpv->out.pln[0] = mpv->out.pln[2] + yh * ypitch;
    mpv->out.pln[1] = mpv->out.pln[0] + yh / 2 * cpitch;
}

/* Nothing to release. */
void MPVUMC_Finish(void) {}

/* The half-pel kernel tables of both block sizes; with cond[3] set the four-point entry is
 * replaced by the vertical kernel. */
void MPVUMC_Init(void) {
    mpvumc_oneref[0][0][0] = MPVMC08_OneRef1p_TuneC;
    mpvumc_oneref[0][0][1] = MPVMC08_OneRefH2_TuneC;
    mpvumc_oneref[0][1][0] = MPVMC08_OneRefV2_TuneC;
    mpvumc_oneref[0][1][1] = MPVMC08_OneRef4p_TuneC;
    mpvumc_oneref[1][0][0] = MPVMC08_OneRef1p_TuneC;
    mpvumc_oneref[1][0][1] = MPVMC08_OneRefH2_TuneC;
    mpvumc_oneref[1][1][0] = MPVMC08_OneRefV2_TuneC;
    mpvumc_oneref[1][1][1] = MPVMC08_OneRefV2_TuneC;
    mpvumc_oneref_y[0][0][0] = MPVMC16_OneRef1p_TuneC;
    mpvumc_oneref_y[0][0][1] = MPVMC16_OneRefH2_TuneC;
    mpvumc_oneref_y[0][1][0] = MPVMC16_OneRefV2_TuneC;
    mpvumc_oneref_y[0][1][1] = MPVMC16_OneRef4p_TuneC;
    mpvumc_oneref_y[1][0][0] = MPVMC16_OneRef1p_TuneC;
    mpvumc_oneref_y[1][0][1] = MPVMC16_OneRefH2_TuneC;
    mpvumc_oneref_y[1][1][0] = MPVMC16_OneRefV2_TuneC;
    mpvumc_oneref_y[1][1][1] = MPVMC16_OneRefV2_TuneC;
}
