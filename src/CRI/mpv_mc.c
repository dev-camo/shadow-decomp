/* CRI Sofdec MPEG video: 8x8 block motion compensation, "tuned C" versions (mpv_mc.c). One
 * reference block is copied (1p), horizontally (H2) / vertically (V2) / four-point (4p) half-pel
 * averaged into the word-packed destination; the source alignment selects the load strategy.
 * mpv_mcy.c holds the 16x16 counterparts. The 1p copies are hand-written assembly inside the C
 * switch: they use update-form indexed loads (lfdux, lwzux), which MWCC does not emit from C, and
 * the fourth row of the 3/7 case prefetches one stride ahead where the other rows prefetch two.
 * Bodies from the RE4 GameCube decompilation of the same file, except the H2 body.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mpv.h"

/* four-point average of the pixel pairs (a, a+1) x (b, b+1), packed as four bytes */
#define MPVMC08_AVG4(p0, p1, p2, p3)                                                               \
    ((((p0) << 22) & 0xFF000000) | (((p1) << 14) & 0x00FF0000) | (((p2) << 6) & 0x0000FF00) |      \
     (((p3) >> 2) & 0x000000FF))

/* byte-wise rounded average of two packed words w and a, given x = w ^ a and the mask variables
 * m1 (0xFEFEFEFE) and m2 (0x01010101) */
#define MPVMC08_AVG2(w, a, x, m1, m2) (((w) & (a)) + (((x) & (m1)) >> 1) + ((x) & (m2)))

/* word and halfword of a reference row at byte offset n (any alignment) */
#define MPVMC08_W(p, n) (*(Uint32 *)((Uint8 *)(p) + (n)))
#define MPVMC08_H(p, n) (*(Uint16 *)((Uint8 *)(p) + (n)))

/* 8x8 block, half-pel in both directions: each output byte is the rounded average of a 2x2
 * reference neighbourhood, written as four packed bytes per word. */
void MPVMC08_OneRef4p_TuneC(MPVMC *mc) {
    /* three rotating pixel pairs (pixel k in pair k % 3) */
    Sint32 i;
    Sint32 stride;
    Uint8 *s0;
    Uint8 *s1;
    Uint32 *d;
    Uint8 a0, b0, a1, b1, a2, b2;
    Uint32 p0, p1, p2, p3, p4, p5, p6, p7;

    stride = mc->stride;
    s0 = mc->src;
    s1 = mc->src2;
    d = mc->dst;
    for (i = 0; i < 8; i++) {
        a0 = s0[0];
        b0 = s1[0];
        __dcbt(s1, stride);
        a1 = s0[1];
        b1 = s1[1];
        p0 = a0 + a1 + b0 + b1 + 2;
        a2 = s0[2];
        b2 = s1[2];
        p1 = a1 + a2 + b1 + b2 + 2;
        a0 = s0[3];
        b0 = s1[3];
        p2 = a2 + a0 + b2 + b0 + 2;
        a1 = s0[4];
        b1 = s1[4];
        p3 = a0 + a1 + b0 + b1 + 2;
        a2 = s0[5];
        b2 = s1[5];
        p4 = a1 + a2 + b1 + b2 + 2;
        a0 = s0[6];
        b0 = s1[6];
        p5 = a2 + a0 + b2 + b0 + 2;
        a1 = s0[7];
        b1 = s1[7];
        p6 = a0 + a1 + b0 + b1 + 2;
        a2 = s0[8];
        b2 = s1[8];
        p7 = a1 + a2 + b1 + b2 + 2;
        d[0] = MPVMC08_AVG4(p0, p1, p2, p3);
        d[1] = MPVMC08_AVG4(p4, p5, p6, p7);
        s0 += stride;
        s1 += stride;
        d += 2;
    }
}

/* 8x8 block, horizontal half-pel average. */
void MPVMC08_OneRefH2_TuneC(MPVMC *mc) {
    Sint32 i;
    Uint8 *s = mc->src;
    Uint32 *d = mc->dst;
    Sint32 stride = mc->stride;
    Uint32 w0, a0, w1, a1, w2;
    Uint32 m1 = 0xFEFEFEFE;
    Uint32 m2 = 0x01010101;

    switch ((Uint32)s & 3) {
    case 0:
        for (i = 0; i < 8; i++) {
            __dcbt(s, stride);
            w0 = MPVMC08_W(s, 0);
            w1 = MPVMC08_W(s, 4);
            a1 = s[8];
            a0 = (w0 << 8) | (w1 >> 24);
            a1 = (w1 << 8) | a1;
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w1, a1, w1 ^ a1, m1, m2);
            s += stride;
            d += 2;
        }
        break;
    case 1:
        s -= 1;
        for (i = 0; i < 8; i++) {
            __dcbt(s, stride);
            w1 = MPVMC08_W(s, 4);
            w0 = MPVMC08_W(s, 0);
            w2 = MPVMC08_H(s, 8);
            a0 = (w0 << 16) | (w1 >> 16);
            w0 = (w0 << 8) | (w1 >> 24);
            a1 = (w1 << 16) | w2;
            w1 = (w1 << 8) | (w2 >> 8);
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w1, a1, w1 ^ a1, m1, m2);
            s += stride;
            d += 2;
        }
        break;
    case 2:
        s -= 2;
        for (i = 0; i < 8; i++) {
            __dcbt(s, stride);
            w0 = MPVMC08_W(s, 0);
            w1 = MPVMC08_W(s, 4);
            w2 = MPVMC08_W(s, 8);
            a0 = (w0 << 24) | (w1 >> 8);
            w0 = (w0 << 16) | (w1 >> 16);
            a1 = (w1 << 24) | (w2 >> 8);
            w1 = (w1 << 16) | (w2 >> 16);
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w1, a1, w1 ^ a1, m1, m2);
            s += stride;
            d += 2;
        }
        break;
    case 3:
        s -= 3;
        for (i = 0; i < 8; i++) {
            __dcbt(s, stride);
            a0 = MPVMC08_W(s, 4);
            w0 = ((Uint32)s[3] << 24) | (a0 >> 8);
            a1 = MPVMC08_W(s, 8);
            w1 = (a0 << 24) | (a1 >> 8);
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w1, a1, w1 ^ a1, m1, m2);
            s += stride;
            d += 2;
        }
        break;
    }
}

/* 8x8 block, vertical half-pel: byte-wise average of each row with the next. */
void MPVMC08_OneRefV2_TuneC(MPVMC *mc) {
    Sint32 i;
    Uint32 *d;
    Uint8 *s0;
    Uint8 *s1;
    Sint32 stride;
    Uint32 w0, a0, w1, a1, w2, a2;
    Uint32 m1 = 0xFEFEFEFE;
    Uint32 m2 = 0x01010101;

    s0 = mc->src;
    s1 = mc->src2;
    d = mc->dst;
    stride = mc->stride;
    switch ((Uint32)s0 & 3) {
    case 0:
        for (i = 0; i < 8; i++) {
            w0 = MPVMC08_W(s0, 0);
            a0 = MPVMC08_W(s1, 0);
            w1 = MPVMC08_W(s0, 4);
            a1 = MPVMC08_W(s1, 4);
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w1, a1, w1 ^ a1, m1, m2);
            s0 += stride;
            s1 += stride;
            d += 2;
        }
        break;
    case 1:
        s0 -= 1;
        s1 -= 1;
        for (i = 0; i < 8; i++) {
            w1 = MPVMC08_W(s0, 4);
            a1 = MPVMC08_W(s1, 4);
            w0 = MPVMC08_W(s0, 0);
            w2 = s0[8];
            s0 += stride;
            a0 = MPVMC08_W(s1, 0);
            a2 = s1[8];
            s1 += stride;
            w0 = (w0 << 8) | (w1 >> 24);
            a0 = (a0 << 8) | (a1 >> 24);
            w2 = (w1 << 8) | w2;
            a2 = (a1 << 8) | a2;
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w2, a2, w2 ^ a2, m1, m2);
            d += 2;
        }
        break;
    case 2:
        s0 -= 2;
        s1 -= 2;
        for (i = 0; i < 8; i++) {
            w1 = MPVMC08_W(s0, 4);
            a1 = MPVMC08_W(s1, 4);
            w0 = MPVMC08_W(s0, 0);
            w2 = MPVMC08_H(s0, 8);
            s0 += stride;
            a0 = MPVMC08_W(s1, 0);
            a2 = MPVMC08_H(s1, 8);
            s1 += stride;
            w0 = (w0 << 16) | (w1 >> 16);
            a0 = (a0 << 16) | (a1 >> 16);
            w2 = (w1 << 16) | w2;
            a2 = (a1 << 16) | a2;
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w2, a2, w2 ^ a2, m1, m2);
            d += 2;
        }
        break;
    case 3:
        s0 -= 3;
        s1 -= 3;
        for (i = 0; i < 8; i++) {
            w1 = MPVMC08_W(s0, 4);
            a1 = MPVMC08_W(s1, 4);
            w0 = MPVMC08_W(s0, 0);
            w2 = MPVMC08_W(s0, 8);
            s0 += stride;
            a0 = MPVMC08_W(s1, 0);
            a2 = MPVMC08_W(s1, 8);
            s1 += stride;
            w0 = (w0 << 24) | (w1 >> 8);
            a0 = (a0 << 24) | (a1 >> 8);
            w2 = (w1 << 24) | (w2 >> 8);
            a2 = (a1 << 24) | (a2 >> 8);
            d[0] = MPVMC08_AVG2(w0, a0, w0 ^ a0, m1, m2);
            d[1] = MPVMC08_AVG2(w2, a2, w2 ^ a2, m1, m2);
            d += 2;
        }
        break;
    }
}

/* 8x8 block, full-pel: copies eight rows (hand-written assembly per source alignment; r3 holds
 * mc until the destination pointer is loaded). */
void MPVMC08_OneRef1p_TuneC(MPVMC *mc) {
    register Uint8 *s = mc->src;

    asm { dcbt r0, s }
    switch ((Uint32)s & 7) {
    case 0: {
        register Sint32 st;

        asm {
        lwz st, MPVMC.stride(r3)
        mr r8, s
        lwz r3, MPVMC.dst(r3)
        lfdux f1, r8, st
        lfdux f2, r8, st
        lfdux f3, r8, st
        lfdux f4, r8, st
        lfdux f5, r8, st
        lfdux f6, r8, st
        lfd f0, 0x0(s)
        lfdux f7, r8, st
        stfd f0, 0x0(r3)
        stfd f1, 0x8(r3)
        stfd f2, 0x10(r3)
        stfd f3, 0x18(r3)
        stfd f4, 0x20(r3)
        stfd f5, 0x28(r3)
        stfd f6, 0x30(r3)
        stfd f7, 0x38(r3)
        }
        break;
    }
    case 4: {
        register Sint32 st;
        register Uint8 *p;
        register Uint32 w;

        asm {
        lwz st, MPVMC.stride(r3)
        mr p, s
        lwz r3, MPVMC.dst(r3)
        lwzux r7, p, st
        lwz r6, 0x0(s)
        lwz w, 0x4(s)
        lwz r8, 0x4(p)
        stw r6, 0x0(r3)
        stw w, 0x4(r3)
        lwzux r6, p, st
        lwz w, 0x4(p)
        stw r7, 0x8(r3)
        stw r8, 0xc(r3)
        lwzux r7, p, st
        lwz r8, 0x4(p)
        stw r6, 0x10(r3)
        stw w, 0x14(r3)
        lwzux r6, p, st
        lwz w, 0x4(p)
        stw r7, 0x18(r3)
        stw r8, 0x1c(r3)
        lwzux r7, p, st
        lwz r8, 0x4(p)
        stw r6, 0x20(r3)
        stw w, 0x24(r3)
        lwzux r6, p, st
        lwz w, 0x4(p)
        stw r7, 0x28(r3)
        stw r8, 0x2c(r3)
        lwzux r7, p, st
        lwz r8, 0x4(p)
        stw r6, 0x30(r3)
        stw w, 0x34(r3)
        stw r7, 0x38(r3)
        stw r8, 0x3c(r3)
        }
        break;
    }
    case 2:
    case 6: {
        register Uint32 t0;
        register Uint32 st;

        asm {
        lwz st, MPVMC.stride(r3)
        mr r6, s
        li t0, 0x2
        lwz s, MPVMC.dst(r3)
        clrrwi r3, st, 1
        mtctr t0
row_pair:
        lhz st, 0x0(r6)
        lwz r7, 0x2(r6)
        lhz r8, 0x6(r6)
        slwi st, st, 16
        rlwimi st, r7, 16, 16, 31
        slwi t0, r7, 16
        stw st, 0x0(s)
        or r8, t0, r8
        add r6, r6, r3
        stw r8, 0x4(s)
        lwz r7, 0x2(r6)
        lhz st, 0x0(r6)
        lhz r8, 0x6(r6)
        slwi t0, r7, 16
        slwi st, st, 16
        add r6, r6, r3
        rlwimi st, r7, 16, 16, 31
        or r8, t0, r8
        stw st, 0x8(s)
        stw r8, 0xc(s)
        lwz r7, 0x2(r6)
        lhz st, 0x0(r6)
        lhz r8, 0x6(r6)
        slwi t0, r7, 16
        slwi st, st, 16
        add r6, r6, r3
        rlwimi st, r7, 16, 16, 31
        or r8, t0, r8
        stw st, 0x10(s)
        stw r8, 0x14(s)
        lwz r7, 0x2(r6)
        lhz st, 0x0(r6)
        lhz r8, 0x6(r6)
        slwi t0, r7, 16
        slwi st, st, 16
        add r6, r6, r3
        rlwimi st, r7, 16, 16, 31
        or r8, t0, r8
        stw st, 0x18(s)
        stw r8, 0x1c(s)
        addi s, s, 0x20
        bdnz row_pair
        }
        break;
    }
    case 1:
    case 5: {
        register Sint32 st;
        register Uint32 *d;

        asm {
        lwz st, MPVMC.stride(r3)
        subi s, s, 0x1
        lwz d, MPVMC.dst(r3)
        slwi r3, st, 1
        dcbt s, st
        lwz r6, 0x0(s)
        lwz r7, 0x4(s)
        lbz r8, 0x8(s)
        dcbt s, r3
        slwi r6, r6, 8
        lwzux r9, s, st
        rlwimi r6, r7, 8, 24, 31
        rlwimi r8, r7, 8, 0, 23
        lwz r7, 0x4(s)
        lbz r10, 0x8(s)
        stw r6, 0x0(d)
        stw r8, 0x4(d)
        dcbt s, r3
        slwi r9, r9, 8
        lwzux r6, s, st
        rlwimi r9, r7, 8, 24, 31
        rlwimi r10, r7, 8, 0, 23
        lwz r7, 0x4(s)
        lbz r8, 0x8(s)
        stw r9, 0x8(d)
        stw r10, 0xc(d)
        dcbt s, r3
        slwi r6, r6, 8
        lwzux r9, s, st
        rlwimi r6, r7, 8, 24, 31
        rlwimi r8, r7, 8, 0, 23
        lwz r7, 0x4(s)
        lbz r10, 0x8(s)
        stw r6, 0x10(d)
        stw r8, 0x14(d)
        dcbt s, r3
        slwi r9, r9, 8
        lwzux r6, s, st
        rlwimi r9, r7, 8, 24, 31
        rlwimi r10, r7, 8, 0, 23
        lwz r7, 0x4(s)
        lbz r8, 0x8(s)
        stw r9, 0x18(d)
        stw r10, 0x1c(d)
        dcbt s, r3
        slwi r6, r6, 8
        lwzux r9, s, st
        rlwimi r6, r7, 8, 24, 31
        rlwimi r8, r7, 8, 0, 23
        lwz r7, 0x4(s)
        lbz r10, 0x8(s)
        stw r6, 0x20(d)
        stw r8, 0x24(d)
        dcbt s, r3
        slwi r9, r9, 8
        lwzux r6, s, st
        rlwimi r9, r7, 8, 24, 31
        rlwimi r10, r7, 8, 0, 23
        lwz r7, 0x4(s)
        slwi r6, r6, 8
        lbz r8, 0x8(s)
        rlwimi r6, r7, 8, 24, 31
        rlwimi r8, r7, 8, 0, 23
        stw r9, 0x28(d)
        stw r10, 0x2c(d)
        lwzux r9, s, st
        lwz r7, 0x4(s)
        slwi r9, r9, 8
        lbz r10, 0x8(s)
        rlwimi r9, r7, 8, 24, 31
        rlwimi r10, r7, 8, 0, 23
        stw r6, 0x30(d)
        stw r8, 0x34(d)
        stw r9, 0x38(d)
        stw r10, 0x3c(d)
        }
        break;
    }
    case 3:
    case 7: {
        register Sint32 st;
        register Uint32 *d;

        asm {
        lwz st, MPVMC.stride(r3)
        subi s, s, 0x3
        lwz d, MPVMC.dst(r3)
        slwi r3, st, 1
        dcbt s, st
        lwz r6, 0x0(s)
        lwz r7, 0x4(s)
        lwz r8, 0x8(s)
        dcbt s, r3
        slwi r6, r6, 24
        lwzux r9, s, st
        rlwimi r6, r7, 24, 8, 31
        srwi r8, r8, 8
        lwz r10, 0x4(s)
        rlwimi r8, r7, 24, 0, 7
        lwz r11, 0x8(s)
        stw r6, 0x0(d)
        stw r8, 0x4(d)
        dcbt s, r3
        slwi r9, r9, 24
        lwzux r6, s, st
        srwi r11, r11, 8
        rlwimi r9, r10, 24, 8, 31
        lwz r7, 0x4(s)
        rlwimi r11, r10, 24, 0, 7
        lwz r8, 0x8(s)
        stw r9, 0x8(d)
        stw r11, 0xc(d)
        dcbt s, r3
        slwi r6, r6, 24
        lwzux r9, s, st
        srwi r8, r8, 8
        rlwimi r6, r7, 24, 8, 31
        lwz r10, 0x4(s)
        rlwimi r8, r7, 24, 0, 7
        lwz r11, 0x8(s)
        stw r6, 0x10(d)
        stw r8, 0x14(d)
        dcbt s, st
        slwi r9, r9, 24
        lwzux r6, s, st
        srwi r11, r11, 8
        rlwimi r9, r10, 24, 8, 31
        lwz r7, 0x4(s)
        rlwimi r11, r10, 24, 0, 7
        lwz r8, 0x8(s)
        stw r9, 0x18(d)
        stw r11, 0x1c(d)
        dcbt s, r3
        slwi r6, r6, 24
        lwzux r9, s, st
        srwi r8, r8, 8
        rlwimi r6, r7, 24, 8, 31
        lwz r10, 0x4(s)
        rlwimi r8, r7, 24, 0, 7
        lwz r11, 0x8(s)
        stw r6, 0x20(d)
        stw r8, 0x24(d)
        dcbt s, r3
        slwi r9, r9, 24
        lwzux r6, s, st
        srwi r11, r11, 8
        rlwimi r9, r10, 24, 8, 31
        lwz r8, 0x8(s)
        rlwimi r11, r10, 24, 0, 7
        lwz r7, 0x4(s)
        slwi r6, r6, 24
        srwi r8, r8, 8
        stw r9, 0x28(d)
        rlwimi r6, r7, 24, 8, 31
        rlwimi r8, r7, 24, 0, 7
        stw r11, 0x2c(d)
        lwzux r9, s, st
        lwz r11, 0x8(s)
        slwi r9, r9, 24
        lwz r10, 0x4(s)
        srwi r11, r11, 8
        stw r6, 0x30(d)
        rlwimi r9, r10, 24, 8, 31
        rlwimi r11, r10, 24, 0, 7
        stw r8, 0x34(d)
        stw r9, 0x38(d)
        stw r11, 0x3c(d)

        }
        break;
    }
    }
}

/* the generic (non-tuned) versions were dead-stripped; the table keeps their slots */
void (*const mpvmc_oneref1p_func_table[4])(MPVMC *mc) = {0, 0, 0, 0};

/* Fills the handle's 8x8 kernel slots (generic table, stripped to NULLs). */
void MPVMC08_Init(MPVMC *mc) {
    mc->oneref08[0] = mpvmc_oneref1p_func_table[0];
    mc->oneref08[1] = mpvmc_oneref1p_func_table[1];
    mc->oneref08[2] = mpvmc_oneref1p_func_table[2];
    mc->oneref08[3] = mpvmc_oneref1p_func_table[3];
}
