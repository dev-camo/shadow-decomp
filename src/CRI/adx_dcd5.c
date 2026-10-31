/* CRI ADX 4-bit decoder (adx_dcd5.c): 18-byte frames of a 2-byte scale and 32 nibbles, second-order
 * prediction with the coefficients c1/c2 (12-bit fixed point); the scale header is descrambled with
 * the running key *scl (key = key * smul + sadd). Stereo frames are interleaved (L, R).
 *
 * Names and structure follow the RE4 GameCube decompilation of the same file.
 *
 * Not yet matching: ADX_DecodeSte4AsSte (115 of 152 words differ) and ADX_DecodeSte4AsMono (137 of
 * 188) differ in register assignment and instruction scheduling; in the first the original needs
 * one callee-saved register fewer: it widens c1 and c2 in place and keeps the scale arguments in
 * volatile registers. */
#include "CRI/adx_dcd.h"

/* Clamps a prediction result to the 16-bit range. The outer range test is required: the single
 * if/else-if clamp changes the code of all three decoders. */
#define ADX_CLAMP(v)                                                                               \
    do {                                                                                           \
        if ((v) > 0x7FFF || (v) < -0x8000) {                                                       \
            if ((v) < -0x8000) {                                                                   \
                (v) = -0x8000;                                                                     \
            } else if ((v) > 0x7FFF) {                                                             \
                (v) = 0x7FFF;                                                                      \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Signed value of each nibble. */
const Sint32 AdxQtbl[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1,
};

/* Decodes `nfrm` interleaved stereo 4-bit ADX frames; with adx_decode_output_mono_flag set, both
 * channels get the L+R mix instead. */
Sint32 ADX_DecodeSte4(Sint8 *src, Sint32 nfrm, Sint16 *outl, Sint16 *histl, Sint16 *outr,
                      Sint16 *histr, Sint16 c1, Sint16 c2, Sint16 *scl, Sint16 smul, Sint16 sadd) {
    if (adx_decode_output_mono_flag == 0) {
        return ADX_DecodeSte4AsSte(src, nfrm, outl, histl, outr, histr, c1, c2, scl, smul, sadd);
    }
    return ADX_DecodeSte4AsMono(src, nfrm, outl, histl, outr, histr, c1, c2, scl, smul, sadd);
}

/* Stereo output: each channel is predicted from its own two previous samples. Returns nfrm, or
 * twice the pairs decoded when a frame's scale word has the end flag. */
Sint32 ADX_DecodeSte4AsSte(Sint8 *src, Sint32 nfrm, Sint16 *outl, Sint16 *histl, Sint16 *outr,
                           Sint16 *histr, Sint16 c1, Sint16 c2, Sint16 *scl, Sint16 smul,
                           Sint16 sadd) {
    Sint32 l2;
    Sint32 rr2;
    Sint32 rr1;
    Sint32 l1;
    Sint32 i;
    Sint32 d;
    Sint32 dr;
    Sint16 sc_l;
    Sint16 sc_r;
    Sint32 s;
    Sint32 t;
    Sint32 nblk;
    Sint32 key;
    Sint32 j;

    nblk = nfrm / 2;
    l1 = histl[0];
    l2 = histl[1];
    rr1 = histr[0];
    rr2 = histr[1];
    for (i = 0; i < nblk; i++) {
        s = *(Sint16 *)src;
        if (s & 0x8000) {
            return i * 2;
        }
        key = *scl;
        sc_l = ((s ^ key) & 0x1FFF) + 1;
        key = sadd + key * smul;
        *scl = key;
        *scl &= 0x7FFF;
        s = *(Sint16 *)(src + 0x12);
        if (s & 0x8000) {
            return i * 2;
        }
        key = *scl;
        sc_r = ((s ^ key) & 0x1FFF) + 1;
        key = sadd + key * smul;
        *scl = key;
        *scl &= 0x7FFF;
        src += 2;
        for (j = 0; j < 16; j++) {
            d = src[0];
            dr = src[0x12];
            src++;
            l2 = (d >> 4) * sc_l + ((c1 * l1 + c2 * l2) >> 12);
            ADX_CLAMP(l2);
            t = (dr >> 4) * sc_r + ((c1 * rr1 + c2 * rr2) >> 12);
            ADX_CLAMP(t);
            d = AdxQtbl[d & 0xF];
            outl[0] = l2;
            dr = AdxQtbl[dr & 0xF];
            outr[0] = t;
            l1 = d * sc_l + ((c1 * l2 + c2 * l1) >> 12);
            ADX_CLAMP(l1);
            rr1 = dr * sc_r + ((c1 * t + c2 * rr1) >> 12);
            ADX_CLAMP(rr1);
            outl[1] = l1;
            rr2 = t;
            outl += 2;
            outr[1] = rr1;
            outr += 2;
        }
        src += 0x12;
    }
    histl[0] = l1;
    histl[1] = l2;
    histr[0] = rr1;
    histr[1] = rr2;
    return nfrm;
}

/* Mono output of a stereo stream: both channels are predicted as above and each output sample is
 * the L+R mix times 0.7, written to both output buffers. */
Sint32 ADX_DecodeSte4AsMono(Sint8 *src, Sint32 nfrm, Sint16 *outl, Sint16 *histl, Sint16 *outr,
                            Sint16 *histr, Sint16 c1, Sint16 c2, Sint16 *scl, Sint16 smul,
                            Sint16 sadd) {
    Sint32 l2;
    Sint32 rr2;
    Sint32 rr1;
    Sint32 l1;
    Sint32 i;
    Sint32 d;
    Sint32 dr;
    Sint16 sc_l;
    Sint16 sc_r;
    Sint32 s;
    Sint32 t;
    Sint32 nblk;
    Sint32 key;
    Sint32 j;

    nblk = nfrm / 2;
    l1 = histl[0];
    l2 = histl[1];
    rr1 = histr[0];
    rr2 = histr[1];
    for (i = 0; i < nblk; i++) {
        s = *(Sint16 *)src;
        if (s & 0x8000) {
            return i * 2;
        }
        key = *scl;
        sc_l = ((s ^ key) & 0x1FFF) + 1;
        key = sadd + key * smul;
        *scl = key;
        *scl &= 0x7FFF;
        s = *(Sint16 *)(src + 0x12);
        if (s & 0x8000) {
            return i * 2;
        }
        key = *scl;
        sc_r = ((s ^ key) & 0x1FFF) + 1;
        key = sadd + key * smul;
        *scl = key;
        *scl &= 0x7FFF;
        src += 2;
        for (j = 0; j < 16; j++) {
            d = src[0];
            dr = src[0x12];
            src++;
            l2 = (d >> 4) * sc_l + ((c1 * l1 + c2 * l2) >> 12);
            ADX_CLAMP(l2);
            t = (dr >> 4) * sc_r + ((c1 * rr1 + c2 * rr2) >> 12);
            ADX_CLAMP(t);
            rr2 = t;
            t = (l2 + t) * 7 / 10;
            ADX_CLAMP(t);
            d = AdxQtbl[d & 0xF];
            outr[0] = t;
            dr = AdxQtbl[dr & 0xF];
            outl[0] = t;
            l1 = d * sc_l + ((c1 * l2 + c2 * l1) >> 12);
            ADX_CLAMP(l1);
            rr1 = dr * sc_r + ((c1 * rr2 + c2 * rr1) >> 12);
            ADX_CLAMP(rr1);
            t = (l1 + rr1) * 7 / 10;
            ADX_CLAMP(t);
            outr[1] = t;
            outr += 2;
            outl[1] = t;
            outl += 2;
        }
        src += 0x12;
    }
    histl[0] = l1;
    histl[1] = l2;
    histr[0] = rr1;
    histr[1] = rr2;
    return nfrm;
}

/* Decodes `nfrm` 4-bit mono ADX frames. Returns the number of frames decoded (stops early at a
 * frame whose scale word has the end flag). */
Sint32 ADX_DecodeMono4(Sint8 *src, Sint32 nfrm, Sint16 *out, Sint16 *hist, Sint16 c1, Sint16 c2,
                       Sint16 *scl, Sint16 smul, Sint16 sadd) {
    Sint32 i;
    Sint32 l2;
    Sint32 l1;
    Sint32 j;
    Sint32 s;
    Sint32 key;
    Sint16 sc;
    Sint32 d;
    Sint32 t;
    Sint32 q;

    l1 = hist[0];
    l2 = hist[1];
    for (i = 0; i < nfrm; i++) {
        s = *(Sint16 *)src;
        if (s & 0x8000) {
            return i;
        }
        key = *scl;
        sc = ((s ^ key) & 0x1FFF) + 1;
        key = sadd + key * smul;
        *scl = key;
        *scl &= 0x7FFF;
        src += 2;
        for (j = 0; j < 16; j++) {
            d = src[0];
            src++;
            t = (d >> 4) * sc + ((c1 * l1 + c2 * l2) >> 12);
            ADX_CLAMP(t);
            q = AdxQtbl[d & 0xF];
            out[0] = t;
            l1 = q * sc + ((c1 * t + c2 * l1) >> 12);
            ADX_CLAMP(l1);
            out[1] = l1;
            l2 = t;
            out += 2;
        }
    }
    hist[0] = l1;
    hist[1] = l2;
    return nfrm;
}
