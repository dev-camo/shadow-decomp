/* CRI Sofdec MPV header decoder (mpv_hdec.c): sequence, GOP, picture and slice headers and user
 * data read from a stream joint (extensions are skipped). Header bits are read with the MPVBIT
 * macros (mpv_bit.h); after a header the consumed bytes are split off the chunk and returned to
 * the free side, and the rest is pushed back to the data side.
 * Bodies from the RE4 GameCube decompilation of the same file. This version also numbers the
 * pictures of a GOP (picture attribute x68), ends the header pass at a sequence end code and
 * decodes slices until the last macroblock of the picture.
 */
#include <stdlib.h>
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/mpv.h"
#include "CRI/mpv_bit.h"
#include "CRI/sj.h"
#include "CRI/uty.h"

/* header types (MPV_OBJ.hdrtype, the MPV_SetUsrSj id) */
#define MPVHDEC_HDR_SEQ 1
#define MPVHDEC_HDR_GOP 2
#define MPVHDEC_HDR_PIC 3

typedef void (*MPVHDEC_MCFUNC)(MPV mpv);
typedef void (*MPVHDEC_SKIPFUNC)(MPV mpv, Sint32 n);

/* slice macroblock decoders by picture type */
Sint32 (*const dec_mbs_func[5])(MPV mpv, SJ sj) = {
    NULL, MPVDEC_DecIpicMb, MPVDEC_DecPpicMb, MPVDEC_DecBpicMb, MPVDEC_DecDpicMb,
};

/* motion compensation functions by [cond[6] != 3][picture type], the intra ones also by cond[4]
 * (MPVHDEC_Init) */
static MPVHDEC_MCFUNC mc_bidirect_func[2][5];
MPVHDEC_MCFUNC mc_backward_func[2][5];
MPVHDEC_MCFUNC mc_forward_func[2][5];
static MPVHDEC_MCFUNC mc_intra_func[2][2][5];
static MPVHDEC_SKIPFUNC skip_func[2][5];

/* Stand-in for a function the linker stripped from the game; its body is unknown. MWCC lays out
 * .bss by first reference, so this never-called function refers to the tables in storage order
 * before mpvhdec_DecPscSj does. Storing the addresses keeps those references from being optimized
 * away.
 */
static void mpvhdec_BssOrder(void **out) {
    out[0] = &mc_bidirect_func;
    out[1] = &mc_backward_func;
    out[2] = &mc_forward_func;
    out[3] = &mc_intra_func;
    out[4] = &skip_func;
}

/* Byte position of the first bit not yet read after `nbit` bits of the window (rounded up to a
 * whole byte); the window words were loaded from the two words before ptr. MPVBIT_BYTEPTR
 * (mpv_bit.h) computes the same address but compiles to a different register assignment here.
 */
#define MPVHDEC_BYTEPOS(q, nbit) (q) = (Uint8 *)(ptr - 2) + (((nbit) + 7) >> 3)

/* Consumes the header bytes up to the reader position; uses the caller's reader locals `ptr` and
 * `bitpos` and its byte pointer `q`. */
#define MPVHDEC_FLUSH(mpv, sj)                                                                     \
    {                                                                                              \
        SJCK rest;                                                                                 \
        MPVHDEC_BYTEPOS(q, bitpos);                                                                \
        SJ_SplitChunk(&(mpv)->ck, q - (mpv)->ck.data, &(mpv)->ck, &rest);                          \
        SJ_PutChunk(sj, SJ_CK_FREE, &(mpv)->ck);                                                   \
        SJ_UngetChunk(sj, SJ_CK_DATA, &rest);                                                      \
    }

/* Slice header: quantiser scale and extra information bits, then the picture type's macroblock
 * decoder runs on the slice data, which starts at bit `bitofs` of its first byte.
 */
static void mpvhdec_DecSlice(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 *ptr;
    Uint32 bbuf;
    Uint32 nbuf;
    Uint32 val;
    Sint32 row;
    Uint8 *q;
    SJCK rest;

    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVBIT_INIT(mpv->ck.data);
    MPVBIT_GET32(val);
    row = (val & 0xFF) - 1;
    mpv->mb_addr = row * mpv->picatr.mb_width - 1;
    mpv->mb_y = row;
    mpv->mb_x = -1;
    MPVBIT_GET(mpv->qscale, 5);
    MPVDEC_ResetMv(&mpv->fwd);
    MPVDEC_ResetMv(&mpv->bwd);
    MPVDEC_ResetDc(mpv);
    for (;;) {
        val = bbuf >> 31;
        if (val == 0) {
            MPVBIT_SKIP(1);
            break;
        }
        MPVBIT_SKIP(9);
        MPVHDEC_BYTEPOS(q, bitpos);
        if (mpv->ck.len <= q - mpv->ck.data) {
            return;
        }
    }
    mpv->bitofs = bitpos & 7;
    MPVHDEC_BYTEPOS(q, bitpos - mpv->bitofs);
    SJ_SplitChunk(&mpv->ck, q - mpv->ck.data, &mpv->ck, &rest);
    SJ_PutChunk(sj, SJ_CK_FREE, &mpv->ck);
    SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
    mpv->dec_mbs_func(mpv, sj);
}

/* Moves `nbyte` bytes of a stream joint from side `id` to the other one; returns the bytes moved.
 */
Sint32 MPV_MoveChunk(SJ sj, Sint32 id, Sint32 nbyte) {
    SJCK ck;
    Sint32 dst;

    if (id == SJ_CK_FREE) {
        dst = SJ_CK_DATA;
    } else {
        dst = SJ_CK_FREE;
    }
    SJ_GetChunk(sj, id, nbyte, &ck);
    SJ_PutChunk(sj, dst, &ck);
    return ck.len;
}

/* Discards data up to the next start code; returns its class (MPV_CheckDelim), 0 if none is found
 * before the data ends. The same body as MPV_GoNextDelimSj, which is defined after its callers in
 * this file and so is not inlined into them.
 */
static inline Sint32 mpvhdec_GoNextDelim(SJ sj) {
    SJCK ck;
    SJCK rest;
    Sint32 delim;
    Uint8 *p;

    for (;;) {
        SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &ck);
        if (ck.len < 4) {
            SJ_UngetChunk(sj, SJ_CK_DATA, &ck);
            delim = 0;
            break;
        }
        p = (Uint8 *)MPV_SearchDelim((Sint8 *)ck.data, ck.len, -1);
        if (p == NULL) {
            SJ_SplitChunk(&ck, ck.len - 3, &ck, &rest);
            SJ_PutChunk(sj, SJ_CK_FREE, &ck);
            SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
            continue;
        }
        delim = MPV_CheckDelim(p);
        SJ_SplitChunk(&ck, p - ck.data, &ck, &rest);
        SJ_PutChunk(sj, SJ_CK_FREE, &ck);
        SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
        break;
    }
    return delim;
}

/* Counts a finished picture, then skips to the next start code of a class in `mask`. Returns 0
 * when one is found, otherwise -2 (cond[1] == 0; inferred: single picture mode) or -3.
 */
static inline Sint32 mpvhdec_NextDelim(MPV mpv, SJ sj, Sint32 mask) {
    Sint32 ret;
    Sint32 delim;
    Sint32 c1;

    c1 = mpv->cond[1];
    if (mpv->pic_done != 0) {
        mpv->pic_done = 0;
        mpv->npic++;
        mpv->errinf.x0c++;
        if (c1 == 0) {
            return -2;
        }
        mpv->errinf.x10++;
    }
    if (c1 == 0) {
        ret = -2;
    } else {
        ret = -3;
    }
    for (;;) {
        delim = mpvhdec_GoNextDelim(sj);
        if (delim == 0) {
            break;
        }
        if (delim & mask) {
            ret = 0;
            break;
        }
        if (MPV_MoveChunk(sj, SJ_CK_DATA, 4) != 4) {
            break;
        }
    }
    return ret;
}

/* Decodes the slices of the picture whose headers MPV_DecodePicAtrSj has read, until a start code
 * other than a slice or the last macroblock of the picture.
 */
Sint32 MPVHDEC_DecPicture(MPV mpv, SJ sj) {
    SJCK ck;
    Sint32 ret;

    mpv->x1344 = mpv->cond[7];
    mpv->x13a0 = 0;
    do {
        ret = mpvhdec_NextDelim(mpv, sj, -1);
        if (ret != 0) {
            return MPVERR_SetCode(mpv, ret);
        }
        SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &ck);
        SJ_UngetChunk(sj, SJ_CK_DATA, &ck);
        if (ck.len < 4) {
            break;
        }
        if ((MPV_CheckDelim(ck.data) & MPV_DLM_SLICE) == 0) {
            break;
        }
        mpvhdec_DecSlice(mpv, sj);
    } while (mpv->mb_addr < mpv->mb_last);
    if (mpv->mb_addr != mpv->mb_last) {
        mpv->errinf.x0c++;
    }
    if (mpv->x13a0 != 0) {
        mpv->errinf.x0c += mpv->x13a0;
    }
    return 0;
}

/* Discards data up to the next start code; returns its class, 0 if the data ends first. */
Sint32 MPV_GoNextDelimSj(SJ sj) {
    SJCK ck;
    SJCK rest;
    Sint32 delim;
    Uint8 *p;

    for (;;) {
        SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &ck);
        if (ck.len < 4) {
            SJ_UngetChunk(sj, SJ_CK_DATA, &ck);
            delim = 0;
            break;
        }
        p = (Uint8 *)MPV_SearchDelim((Sint8 *)ck.data, ck.len, -1);
        if (p == NULL) {
            SJ_SplitChunk(&ck, ck.len - 3, &ck, &rest);
            SJ_PutChunk(sj, SJ_CK_FREE, &ck);
            SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
            continue;
        }
        delim = MPV_CheckDelim(p);
        SJ_SplitChunk(&ck, p - ck.data, &ck, &rest);
        SJ_PutChunk(sj, SJ_CK_FREE, &ck);
        SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
        break;
    }
    return delim;
}

/* Sequence user data of the Sofdec encoder: "IDCPREC n" selects 8 or 11 bit intra DC precision
 * (the DC size tables and the intra block decoder), "STCCODE a b c" records three codes; code 8
 * is not supported (-1).
 */
Sint32 mpvhdec_DecSeqUdsc(MPV mpv, Char8 *buf, Sint32 len) {
    Sint32 ret;
    Char8 *p;
    Sint32 i;

    ret = 0;
    for (i = 0; i < len - 4; i++) {
        p = &buf[i + 4]; /* past the start code */
        if (strncmp(p, "IDCPREC", 7) == 0) {
            if (atoi(p + 16) == 0) {
                mpv->dcprec = 0;
            } else {
                mpv->dcprec = 3;
            }
        }
        if (strncmp(p, "STCCODE", 7) == 0) {
            mpv->stc[0] = atoi(p + 16);
            mpv->stc[1] = atoi(p + 24);
            mpv->stc[2] = atoi(p + 32);
        }
        if (MPV_CheckDelim(p) != 0) {
            break;
        }
    }
    if (mpv->dcprec == 0) {
        mpv->intra_func = MPVABDEC_IntraBlock;
        mpv->dctbl_y = mpvvlc_y_dcsiz;
        mpv->dctbl_c = mpvvlc_c_dcsiz;
    } else {
        mpv->intra_func = MPVABDEC_IntraBlockDc11;
        mpv->dctbl_y = mpvvlc2_y_dcsiz;
        mpv->dctbl_c = mpvvlc2_c_dcsiz;
    }
    if (mpv->stc[0] == 8) {
        ret = -1;
    } else {
        if (mpv->dcprec == 0) {
            mpv->intra_func = MPVABDEC_IntraBlock;
        } else {
            mpv->intra_func = MPVABDEC_IntraBlockDc11;
        }
        mpv->nintra_func = MPVABDEC_NintraBlock;
    }
    return ret;
}

/* A user data block up to the next start code: parsed as encoder data after a sequence header,
 * copied into the user data stream joint of the current header type (MPV_SetUsrSj), and after a
 * picture header also into the picture user data buffer (MPV_SetPicUsrBuf).
 */
Sint32 mpvhdec_AnalyUd(MPV mpv, Uint8 *buf, Sint32 len) {
    SJCK ck;
    SJCK ck2;
    Sint32 ret;
    Sint32 ret2;
    Sint32 type;
    Sint32 n;
    SJ sj;
    Sint32 siz;
    int i; /* int: n is a separate copy of i in the original, which a Sint32 i does not give */

    ret2 = 0;
    ret = 0;
    type = mpv->hdrtype;
    for (i = 4; i < len - 3; i++) {
        if (MPV_CheckDelim(buf + i) != 0) {
            break;
        }
    }
    if (i == len - 3) {
        ret = -1;
    }
    n = i;
    if (type == MPVHDEC_HDR_SEQ) {
        ret2 = mpvhdec_DecSeqUdsc(mpv, (Char8 *)buf, i);
    }
    sj = mpv->usr[type].sj;
    if (sj != NULL) {
        SJ_GetChunk(sj, SJ_CK_FREE, n, &ck);
        memcpy(ck.data, buf, ck.len);
        SJ_PutChunk(sj, SJ_CK_DATA, &ck);
        if (ck.len < n) {
            SJ_GetChunk(sj, SJ_CK_FREE, n - ck.len, &ck2);
            memcpy(ck2.data, buf + ck.len, ck2.len);
            SJ_PutChunk(sj, SJ_CK_DATA, &ck2);
        }
        if (mpv->usr[type].func != NULL) {
            mpv->usr[type].func(mpv->usr[type].obj, type);
        }
    }
    if (type == MPVHDEC_HDR_PIC && mpv->picusr_buf != NULL) {
        siz = mpv->picusr_bufsiz;
        if (n < siz) {
            siz = n;
        }
        mpv->picusr_len = siz;
        memcpy(mpv->picusr_buf, buf, mpv->picusr_len);
    }
    if (ret2 != 0) {
        return ret2;
    }
    return ret;
}

/* Picture header: temporal reference, picture type, vbv_delay and the forward/backward f_codes;
 * selects the macroblock decoder, skip and motion compensation functions for the picture type,
 * then skips the extra information bytes.
 */
Sint32 mpvhdec_DecPscSj(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 bbuf;
    Uint32 *ptr;
    Uint32 nbuf;
    Uint32 val;
    Sint32 type;
    Sint32 c6;
    Sint32 c4;
    Sint32 r_size;
    Uint8 *q;

    mpv->hdrtype = MPVHDEC_HDR_PIC;
    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVBIT_INIT(mpv->ck.data);
    MPVBIT_GET32(val);
    MPVBIT_GET(mpv->picatr.temp_ref, 10);
    MPVBIT_GET(val, 3);
    mpv->picatr.pic_type = val;
    MPVBIT_GET(mpv->vbv_delay, 16);
    type = mpv->picatr.pic_type;
    if (type == 1 || type == 2) {
        mpv->x1398++;
        mpv->x139c = 0;
        mpv->picatr.x68 = mpv->x1398 << 16;
    } else {
        mpv->x139c++;
        mpv->picatr.x68 = ((mpv->x1398 - 1) << 16) | mpv->x139c;
    }
    if (type == 2 || type == 3) {
        MPVBIT_GET1(mpv->fwd.full_pel);
        MPVBIT_GET(r_size, 3);
        mpv->fwd.r_size = --r_size;
        mpv->fwd.shift = 27 - r_size;
        mpv->fwd.f = 1 << r_size;
    }
    if (type == 3) {
        MPVBIT_GET1(mpv->bwd.full_pel);
        MPVBIT_GET(r_size, 3);
        mpv->bwd.r_size = --r_size;
        mpv->bwd.shift = 27 - r_size;
        mpv->bwd.f = 1 << r_size;
    }
    c6 = (mpv->cond[6] == 3) ? 0 : 1;
    c4 = mpv->cond[4];
    mpv->intra_blocks = MPVCDEC_IntraBlocks;
    mpv->nintra_blocks = MPVCDEC_NintraBlocks;
    mpv->dec_mbs_func = dec_mbs_func[type];
    mpv->skip_func = skip_func[c6][type];
    mpv->mc_intra_func = mc_intra_func[c4][c6][type];
    mpv->mc_func[1] = mc_backward_func[c6][type];
    mpv->mc_func[2] = mc_forward_func[c6][type];
    mpv->mc_func[3] = mc_bidirect_func[c6][type];
    mpv->mc_func[0] = mpv->mc_func[2];
    for (;;) {
        val = bbuf >> 31;
        if (val == 0) {
            MPVBIT_SKIP(1);
            break;
        }
        MPVBIT_SKIP(9);
        MPVHDEC_BYTEPOS(q, bitpos);
        if (mpv->ck.len <= q - mpv->ck.data) {
            return -3;
        }
    }
    MPVHDEC_FLUSH(mpv, sj);
    return 0;
}

/* GOP header: the time code (drop flag, hours, minutes, seconds, pictures), closed_gop and
 * broken_link.
 */
static Sint32 mpvhdec_DecGscSj(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 bbuf;
    Uint32 *ptr;
    Uint32 nbuf;
    Uint32 val;
    Uint32 tc;
    Uint8 *q;

    mpv->hdrtype = MPVHDEC_HDR_GOP;
    mpv->picatr.ngop++;
    mpv->x1398 = 0;
    mpv->x139c = 0;
    mpv->picatr.x68 = 0;
    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVBIT_INIT(mpv->ck.data);
    MPVBIT_GET32(val);
    MPVBIT_GET(tc, 25);
    mpv->picatr.tc_pic = tc & 0x3F;
    mpv->picatr.tc_sec = (tc >> 6) & 0x3F;
    mpv->picatr.tc_min = (tc >> 13) & 0x3F;
    mpv->picatr.tc_hour = (tc >> 19) & 0x1F;
    mpv->picatr.tc_drop = tc >> 24;
    MPVBIT_GET1(mpv->linkflg1);
    MPVBIT_GET1(mpv->linkflg2);
    MPVHDEC_FLUSH(mpv, sj);
    return 0;
}

/* Sequence header: picture size and macroblock counts, aspect ratio, picture rate, bit rate, vbv
 * buffer size, constrained flag and the intra / non-intra quantiser matrices (the defaults when
 * absent).
 */
Sint32 mpvhdec_DecShcSj(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 bbuf;
    Uint32 *ptr;
    Uint32 nbuf;
    Uint32 val;
    Sint32 i;
    Uint8 *q;

    mpv->hdrtype = MPVHDEC_HDR_SEQ;
    mpv->picatr.nseq++;
    mpv->x1398 = 0;
    mpv->x139c = 0;
    mpv->picatr.x68 = 0;
    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVBIT_INIT(mpv->ck.data);
    MPVBIT_GET32(val);
    MPVBIT_GET(mpv->picatr.width, 12);
    MPVBIT_GET(mpv->picatr.height, 12);
    MPVBIT_GET(mpv->aspect, 4);
    MPVBIT_GET(val, 4);
    mpv->picatr.frame_rate = val;
    MPVBIT_GET(mpv->bitrate, 18);
    MPVBIT_SKIP(1);
    MPVBIT_GET(mpv->vbv_size, 10);
    MPVBIT_GET1(mpv->constrained);
    MPVBIT_GET1(val);
    if (val != 0) {
        for (i = 0; i < 64; i++) {
            MPVBIT_GET(val, 8);
            mpv->intra_iqm[mpvbdec_zigzag[i]] = val;
        }
    } else {
        UTY_MemcpyDword((Uint32 *)mpv->intra_iqm, (Uint32 *)mpvbdec_dfl_iqm, 16);
    }
    MPVBIT_GET1(val);
    if (val != 0) {
        for (i = 0; i < 64; i++) {
            MPVBIT_GET(val, 8);
            mpv->nintra_iqm[mpvbdec_zigzag[i]] = val;
        }
    } else {
        UTY_MemsetDword(mpv->nintra_iqm, 0x10101010, 16);
    }
    mpv->picatr.mb_width = (mpv->picatr.width + 15) >> 4;
    mpv->picatr.mb_height = (mpv->picatr.height + 15) >> 4;
    mpv->mb_last = mpv->picatr.mb_height * mpv->picatr.mb_width - 1;
    mpv->picatr.bitrate = mpv->bitrate;
    mpv->picatr.vbv_size = mpv->vbv_size;
    mpv->picatr.aspect = mpv->aspect;
    mpv->picatr.constrained = mpv->constrained;
    MPVHDEC_FLUSH(mpv, sj);
    return 0;
}

/* Decodes the headers in a memory chunk up to the first slice; *used receives the bytes consumed.
 */
Sint32 MPV_DecodePicAtr(MPV mpv, SJCK *ck, Sint32 *used) {
    Sint32 ret;
    SJ sj;

    sj = SJMEM_Create(ck->data, ck->len);
    if (sj == NULL) {
        return -1;
    }
    ret = MPV_DecodePicAtrSj(mpv, sj);
    *used = ck->len - SJ_GetNumData(sj, SJ_CK_DATA);
    SJ_Destroy(sj);
    return ret;
}

/* MPEG-1 (1) or MPEG-2 (2), decided once from the start code after the first sequence header. */
static inline Sint32 mpvhdec_GetM2vMode(MPV mpv, Sint8 *data, Sint32 len) {
    Sint8 *p;
    Sint32 delim;

    if (mpv->m2v_mode != 0) {
        return mpv->m2v_mode;
    }
    p = MPV_SearchDelim(data, len, MPV_DLM_SEQ);
    if (p == NULL) {
        return mpv->m2v_mode;
    }
    p += 4;
    p = MPV_SearchDelim(p, len - (p - data), -1);
    if (p == NULL) {
        return mpv->m2v_mode;
    }
    delim = MPV_CheckDelim(p);
    if (delim & MPV_DLM_EXT) {
        mpv->m2v_mode = 2;
    } else if (delim != 0) {
        mpv->m2v_mode = 1;
    }
    return mpv->m2v_mode;
}

/* Places the bit reader at `buf` without loading the window. The word pointer is assigned inside
 * the bit position expression; written as two statements, the position computation in
 * MPV_DecodePicAtrSj (where both skip helpers are inlined) is allocated and scheduled differently
 * from the original.
 */
#define MPVHDEC_SETPOS(buf)                                                                        \
    bitpos = ((Uint32)(buf) - (Uint32)(ptr = (Uint32 *)((Uint32)(buf) & ~3))) << 3

/* byte position after the start code at the reader position */
#define MPVHDEC_AFTER_SC(q) (q) = (Uint8 *)(ptr + 1) + ((bitpos + 7) >> 3)

/* Skips an extension start code and its data up to the next start code. */
static inline void mpvhdec_SkipExt(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 *ptr;
    Uint8 *q;
    SJCK rest;

    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVHDEC_SETPOS(mpv->ck.data);
    MPVHDEC_AFTER_SC(q);
    SJ_SplitChunk(&mpv->ck, q - mpv->ck.data, &mpv->ck, &rest);
    SJ_PutChunk(sj, SJ_CK_FREE, &mpv->ck);
    SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
    mpvhdec_GoNextDelim(sj);
}

/* Analyses a user data block (mpvhdec_AnalyUd) and skips it. */
static inline void mpvhdec_SkipUd(MPV mpv, SJ sj) {
    Sint32 bitpos;
    Uint32 *ptr;
    Uint8 *q;
    SJCK rest;

    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &mpv->ck);
    MPVHDEC_SETPOS(mpv->ck.data);
    mpvhdec_AnalyUd(mpv, mpv->ck.data, mpv->ck.len);
    MPVHDEC_AFTER_SC(q);
    SJ_SplitChunk(&mpv->ck, q - mpv->ck.data, &mpv->ck, &rest);
    SJ_PutChunk(sj, SJ_CK_FREE, &mpv->ck);
    SJ_UngetChunk(sj, SJ_CK_DATA, &rest);
    mpvhdec_GoNextDelim(sj);
}

/* Header pass of one picture: reads sequence, GOP and picture headers and user data and skips
 * extensions until the first slice, filling the picture attributes. A sequence end code ends it
 * with -2. The handle is taken untyped and converted, as in MPV_DecodeFrmSj (mpv_frm.c): with an
 * MPV parameter the handle and the stream joint get each other's registers.
 */
Sint32 MPV_DecodePicAtrSj(void *hn, SJ sj) {
    MPV_OBJ *mpv = hn;
    SJCK ck;
    SJCK ck2;
    Sint8 *data;
    Sint32 len;
    Sint32 delim;
    Sint32 ret;

    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(NULL, 0xFF03020C);
    }
    mpv->picusr_len = 0;
    SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &ck);
    SJ_UngetChunk(sj, SJ_CK_DATA, &ck);
    data = (Sint8 *)ck.data;
    len = ck.len;
    if (mpvhdec_GetM2vMode(mpv, data, len) == 2) {
        return MPVM2V_DecodePicAtr(mpv, sj);
    }
    for (;;) {
        ret = mpvhdec_NextDelim(mpv, sj, -1);
        if (ret != 0) {
            return MPVERR_SetCode(mpv, ret);
        }
        SJ_GetChunk(sj, SJ_CK_DATA, 0x7FFFFFFF, &ck2);
        SJ_UngetChunk(sj, SJ_CK_DATA, &ck2);
        if (ck2.len < 4) {
            delim = 0;
        } else {
            delim = MPV_CheckDelim(ck2.data);
        }
        if (delim == 0 || (delim & (MPV_DLM_SLICE | MPV_DLM_SLICE1))) {
            break;
        }
        if (delim & MPV_DLM_END) {
            ret = -2;
            break;
        }
        switch (delim) {
        case MPV_DLM_SEQ:
            mpvhdec_DecShcSj(mpv, sj);
            break;
        case MPV_DLM_GOP:
            mpvhdec_DecGscSj(mpv, sj);
            break;
        case MPV_DLM_PIC:
            mpvhdec_DecPscSj(mpv, sj);
            break;
        case MPV_DLM_EXT:
            mpvhdec_SkipExt(mpv, sj);
            break;
        case MPV_DLM_UD:
            mpvhdec_SkipUd(mpv, sj);
            break;
        }
    }
    return ret;
}

/* The user data of the last picture header (buffer and length). */
void MPV_GetPicUsr(MPV mpv, Uint8 **buf, Sint32 *len) {
    if (buf != NULL) {
        *buf = mpv->picusr_buf;
    }
    if (len != NULL) {
        *len = mpv->picusr_len;
    }
}

/* Buffer that receives the user data of each picture header. */
void MPV_SetPicUsrBuf(MPV mpv, Uint8 *buf, Sint32 bufsiz) {
    mpv->picusr_buf = buf;
    mpv->picusr_bufsiz = bufsiz;
    mpv->picusr_len = 0;
}

/* Stream joint and notification callback that receive the user data of header type `id`. */
void MPV_SetUsrSj(MPV mpv, Sint32 id, SJ sj, void (*func)(void *obj, Sint32 id), void *obj) {
    MPV_USRSJ *usr = &mpv->usr[id];

    usr->sj = sj;
    usr->func = func;
    usr->obj = obj;
}

/* Library init: the skip, intra, forward, backward and bidirectional motion compensation function
 * tables by picture type (only the mpv_umc.c functions are installed).
 */
void MPVHDEC_Init(void) {
    memset(skip_func, 0, sizeof(skip_func));
    memset(mc_intra_func, 0, sizeof(mc_intra_func));
    memset(mc_forward_func, 0, sizeof(mc_forward_func));
    memset(mc_backward_func, 0, sizeof(mc_backward_func));
    memset(mc_bidirect_func, 0, sizeof(mc_bidirect_func));
    skip_func[0][1] = MPVUMC_PpicSkipped;
    skip_func[0][2] = MPVUMC_PpicSkipped;
    skip_func[0][3] = MPVUMC_BpicSkipped;
    mc_intra_func[0][0][1] = MPVUMC_Intra;
    mc_intra_func[0][0][2] = MPVUMC_Intra;
    mc_intra_func[0][0][3] = MPVUMC_Intra;
    mc_intra_func[0][0][4] = MPVUMC_Intra;
    mc_intra_func[1][0][1] = MPVUMC_Intra;
    mc_intra_func[1][0][2] = MPVUMC_Intra;
    mc_intra_func[1][0][3] = MPVUMC_Intra;
    mc_intra_func[1][0][4] = MPVUMC_Intra;
    mc_forward_func[0][2] = MPVUMC_Forward;
    mc_forward_func[0][3] = MPVUMC_Forward;
    mc_backward_func[0][3] = MPVUMC_Backward;
    mc_bidirect_func[0][3] = MPVUMC_BiDirect;
}
