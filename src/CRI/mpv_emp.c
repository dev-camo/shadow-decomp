/* MPEG video: detect "empty" P/B pictures (a single slice whose macroblocks are all skipped). The
 * slice data is read with the MPVBIT macros (mpv_bit.h).
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mpv.h"
#include "CRI/mpv_bit.h"

/* macroblock_address_increment VLC: (code << 8) | length, indexed by the increment */
Sint16 mpvemp_mbai[36] = {
    0x0000, 0x0101, 0x0303, 0x0203, 0x0304, 0x0204, 0x0305, 0x0205, 0x0707, 0x0607, 0x0B08, 0x0A08,
    0x0908, 0x0808, 0x0708, 0x0608, 0x170A, 0x160A, 0x150A, 0x140A, 0x130A, 0x120A, 0x230B, 0x220B,
    0x210B, 0x200B, 0x1F0B, 0x1E0B, 0x1D0B, 0x1C0B, 0x1B0B, 0x1A0B, 0x190B, 0x180B, 0x0F0B, 0x080B,
};

/* Whether the P picture at `buf` (nbyte bytes, nmb macroblocks) codes nothing: one slice whose
 * macroblocks are all skipped or "forward, no coded blocks, zero vector". The video driver drops
 * such pictures without decoding (their reference frame is re-shown). */
Sint32 MPV_IsEmptyPpic(Sint8 *buf, Sint32 nbyte, Sint32 nmb) {
    Sint32 bitpos;
    Uint32 *ptr;
    Uint32 bbuf;
    Uint32 nbuf;
    Sint32 cnt;
    Uint32 val;
    Uint32 code;
    Sint32 len;
    Sint32 pos;
    Sint8 *q;
    Sint8 *p;
    Sint32 flg;

    MPVBIT_INIT(buf);
    /* slice_start_code 1 */
    MPVBIT_GET32(val);
    if (val != 0x101) {
        return 0;
    }
    /* quantiser_scale_code */
    MPVBIT_GET(val, 5);
    /* extra_bit_slice */
    MPVBIT_GET1(val);
    if (val != 0) {
        return 0;
    }
    /* macroblock_address_increment = 1 */
    MPVBIT_GET1(val);
    if (val == 0) {
        return 0;
    }
    /* macroblock_type "001" (MC, not coded) + zero motion vectors "1" "1" */
    MPVBIT_PEEK(val, 5);
    if (val != 7) {
        return 0;
    }
    MPVBIT_SKIP(5);
    /* macroblock_escape codes for the remaining nmb - 1 macroblocks */
    cnt = nmb - 1;
    for (;;) {
        MPVBIT_PEEK(val, 11);
        if (val != 8) {
            break;
        }
        MPVBIT_SKIP(11);
        cnt -= 33;
        if (cnt <= 33) {
            break;
        }
    }
    if (cnt <= 0 || cnt > 33) {
        return 0;
    }
    code = mpvemp_mbai[cnt];
    len = code & 0xFF;
    MPVBIT_GET(val, len);
    if (val != code >> 8) {
        return 0;
    }
    MPVBIT_PEEK(val, 5);
    if (val != 7) {
        return 0;
    }
    MPVBIT_SKIP(5);
    MPVBIT_BYTEPTR(q);
    pos = q - buf;
    if (pos > nbyte) {
        return 0;
    }
    /* the picture must be followed by a delimiter inside the buffer */
    for (;;) {
        p = MPV_SearchDelim(q, nbyte - pos, 0xCC);
        if (p == 0) {
            return 0;
        }
        flg = MPV_CheckDelim(p);
        if (flg & 0x04) {
            /* picture_start_code: only a following B picture keeps this one empty */
            if ((((p[5] & 3) << 1) | ((p[6] >> 7) & 1)) != 3) {
                return 0;
            }
            q = p + 1;
            pos = q - buf;
            continue;
        }
        if (flg & 0x40) {
            q = p + 1;
            pos = q - buf;
            continue;
        }
        if (flg & 0x80) {
            if (p + 4 - buf > nbyte) {
                return 0;
            }
            break;
        }
        if (flg & 0x08) {
            if (p[7] & 0x40) {
                if (p + 7 - buf > nbyte) {
                    return 0;
                }
                break;
            }
            return 0;
        }
        return 0;
    }
    return 1;
}

/* Whether the B picture at `buf` codes nothing: a single slice (start code 0x101, no extra slice
 * information) whose first macroblock and last macroblock are forward- or backward-only predicted,
 * not coded, with zero motion codes, and one address increment (with escapes) skipping all nmb - 2
 * macroblocks between them; the slice must end within nbyte. */
Sint32 MPV_IsEmptyBpic(Sint8 *buf, Sint32 nbyte, Sint32 nmb) {
    Sint32 bitpos;
    Uint32 *ptr;
    Uint32 bbuf;
    Uint32 nbuf;
    Sint32 cnt;
    Uint32 val;
    Uint32 code;
    Sint32 len;
    Sint32 pos;
    Sint8 *q;

    MPVBIT_INIT(buf);
    MPVBIT_GET32(val);
    if (val != 0x101) {
        return 0;
    }
    MPVBIT_GET(val, 5);
    MPVBIT_GET1(val);
    if (val != 0) {
        return 0;
    }
    MPVBIT_GET1(val);
    if (val == 0) {
        return 0;
    }
    /* macroblock_type backward not coded "010" or forward not coded "0010", each followed by the
     * zero horizontal and vertical motion codes "1" "1" (5 or 6 bits) */
    MPVBIT_PEEK(val, 6);
    switch (val) {
    case 0x16:
    case 0x17:
        MPVBIT_SKIP(5);
        break;
    case 0x0B:
        MPVBIT_SKIP(6);
        break;
    default:
        return 0;
    }
    cnt = nmb - 1;
    for (;;) {
        MPVBIT_PEEK(val, 11);
        if (val != 8) {
            break;
        }
        MPVBIT_SKIP(11);
        cnt -= 33;
        if (cnt <= 33) {
            break;
        }
    }
    if (cnt <= 0 || cnt > 33) {
        return 0;
    }
    code = mpvemp_mbai[cnt];
    len = code & 0xFF;
    MPVBIT_GET(val, len);
    if (val != code >> 8) {
        return 0;
    }
    MPVBIT_PEEK(val, 6);
    switch (val) {
    case 0x16:
    case 0x17:
        MPVBIT_SKIP(5);
        break;
    case 0x0B:
        MPVBIT_SKIP(6);
        break;
    default:
        return 0;
    }
    MPVBIT_BYTEPTR(q);
    pos = q - buf;
    if (pos > nbyte) {
        return 0;
    }
    return 1;
}
