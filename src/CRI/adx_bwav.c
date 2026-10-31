/* CRI ADXB RIFF WAVE support (adx_bwav.c): signature test, header parse ("fmt " / "data" chunks)
 * and the per-tick PCM copy steps for 4-, 8- and 16-bit WAV into the ADXB PCM ring (the 4-bit step
 * does no ADPCM decoding). Selected by ADXB_DecodeHeader / ADXB_ExecHndl for type 1.
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include <string.h>

#include "CRI/adx_b.h"

void ADXB_ExecOneWav4(ADXB adxb);
void ADXB_ExecOneWav8(ADXB adxb);
void ADXB_ExecOneWav16(ADXB adxb);
Sint32 ADX_DecodeInfoWav(Uint8 *buf, Sint32 bsize, Sint16 *hdrlen, Sint8 *x0c, Sint8 *bps,
                         Sint8 *x0f, Sint8 *nch, Sint32 *sfreq, Sint32 *nsmpl, Sint32 *fmt,
                         Sint16 *x9c);

/* chunk id accessors (inlined): the static id strings come first in .rodata */
static Char8 *wav_FmtId(void) {
    static Char8 *fmt_id = "fmt ";

    return fmt_id;
}

/* The "data" chunk id string (static, first in .rodata). */
static Char8 *wav_DataId(void) {
    static Char8 *data_id = "data";

    return data_id;
}

/* Dispatches on the sample width recorded at header time (x9c 2 = 4-bit, 1 = 8-bit, 0 = 16-bit). */
void ADXB_ExecOneWav(ADXB adxb) {
    if (adxb->x9c == 2) {
        ADXB_ExecOneWav4(adxb);
    } else if (adxb->x9c == 1) {
        ADXB_ExecOneWav8(adxb);
    } else if (adxb->x9c == 0) {
        ADXB_ExecOneWav16(adxb);
    }
}

/* "RIFF" .... "WAVE" signature test. */
Sint32 ADXB_CheckWav(Uint8 *buf) {
    if (memcmp(buf, "RIFF", 4) == 0 && memcmp(buf + 8, "WAVE", 4) == 0) {
        return 1;
    }
    return 0;
}

/* Copy step for 4-bit WAV (x9c 2): no ADPCM decoding; each output sample is a little-endian 16-bit
 * value assembled from two input bytes (stereo: bytes 0/2 for the left and 1/3 for the right
 * channel of each 4-byte group). Reports the bytes consumed and samples written. */
void ADXB_ExecOneWav4(ADXB adxb) {
    Uint8 *inbuf;
    Sint16 *out0;
    Sint16 *out1;
    Sint32 i;
    Sint32 n;

    inbuf = (Uint8 *)adxb->inbuf;
    if (adxb->stat == ADXB_STAT_DECODE && ADXPD_GetStat(adxb->pd) == ADXPD_STAT_STOP) {
        adxb->getwr_func(adxb->getwr_obj, &adxb->wr_pos, &adxb->wr_nsmpl, &adxb->wr_x70);
        n = adxb->pcmbuf_nsmpl - adxb->wr_pos;
        if (n > adxb->wr_nsmpl) {
            n = adxb->wr_nsmpl;
        }
        if (n > adxb->inbuf_nsmpl) {
            n = adxb->inbuf_nsmpl;
        }
        out0 = adxb->pcmbuf + adxb->wr_pos;
        if (adxb->nch == 2) {
            out1 = adxb->pcmbuf + (adxb->pcmbuf_chofst + adxb->wr_pos);
            for (i = 0; i < n; i++) {
                out0[i] = inbuf[i * 4] | (inbuf[i * 4 + 2] << 8);
                out1[i] = inbuf[i * 4 + 1] | (inbuf[i * 4 + 3] << 8);
            }
        } else {
            for (i = 0; i < n; i++) {
                out0[i] = inbuf[i * 2] | (inbuf[i * 2 + 1] << 8);
            }
        }
        adxb->dec_nsmpl = n;
        adxb->dec_nbyte = adxb->nch * (n * 2);
        adxb->stat = ADXB_STAT_WRITE;
    }
    if (adxb->stat == ADXB_STAT_WRITE) {
        adxb->addwr_func(adxb->addwr_obj, adxb->dec_nbyte, adxb->dec_nsmpl);
        adxb->stat = ADXB_STAT_DONE;
    }
}

/* Decode step for 8-bit unsigned PCM: samples biased to signed and scaled to 16 bits into the PCM
 * ring (deinterleaving stereo); reports bytes consumed / samples produced. */
void ADXB_ExecOneWav8(ADXB adxb) {
    Uint8 *inbuf;
    Sint16 *out0;
    Sint16 *out1;
    Sint32 i;
    Sint32 n;

    inbuf = (Uint8 *)adxb->inbuf;
    if (adxb->stat == ADXB_STAT_DECODE && ADXPD_GetStat(adxb->pd) == ADXPD_STAT_STOP) {
        adxb->getwr_func(adxb->getwr_obj, &adxb->wr_pos, &adxb->wr_nsmpl, &adxb->wr_x70);
        n = adxb->pcmbuf_nsmpl - adxb->wr_pos;
        if (n > adxb->wr_nsmpl) {
            n = adxb->wr_nsmpl;
        }
        if (n > adxb->inbuf_nsmpl) {
            n = adxb->inbuf_nsmpl;
        }
        out0 = adxb->pcmbuf + adxb->wr_pos;
        if (adxb->nch == 2) {
            out1 = adxb->pcmbuf + (adxb->pcmbuf_chofst + adxb->wr_pos);
            for (i = 0; i < n; i++) {
                out0[i] = (Uint8)(inbuf[i * 2] - 0x80) << 8;
                out1[i] = (Uint8)(inbuf[i * 2 + 1] - 0x80) << 8;
            }
        } else {
            for (i = 0; i < n; i++) {
                out0[i] = (Uint8)(inbuf[i] - 0x80) << 8;
            }
        }
        adxb->dec_nsmpl = n;
        adxb->dec_nbyte = n * adxb->nch;
        adxb->stat = ADXB_STAT_WRITE;
    }
    if (adxb->stat == ADXB_STAT_WRITE) {
        adxb->addwr_func(adxb->addwr_obj, adxb->dec_nbyte, adxb->dec_nsmpl);
        adxb->stat = ADXB_STAT_DONE;
    }
}

/* Decode step for 16-bit little-endian PCM: byte-swaps into the PCM ring (deinterleaving
 * stereo). */
void ADXB_ExecOneWav16(ADXB adxb) {
    Uint16 *inbuf;
    Uint16 *out0;
    Uint16 *out1;
    Sint32 i;
    Sint32 n;

    inbuf = (Uint16 *)adxb->inbuf;
    if (adxb->stat == ADXB_STAT_DECODE && ADXPD_GetStat(adxb->pd) == ADXPD_STAT_STOP) {
        adxb->getwr_func(adxb->getwr_obj, &adxb->wr_pos, &adxb->wr_nsmpl, &adxb->wr_x70);
        n = adxb->pcmbuf_nsmpl - adxb->wr_pos;
        if (n > adxb->wr_nsmpl) {
            n = adxb->wr_nsmpl;
        }
        if (n > adxb->inbuf_nsmpl) {
            n = adxb->inbuf_nsmpl;
        }
        out0 = (Uint16 *)adxb->pcmbuf + adxb->wr_pos;
        if (adxb->nch == 2) {
            out1 = (Uint16 *)adxb->pcmbuf + (adxb->pcmbuf_chofst + adxb->wr_pos);
            for (i = 0; i < n; i++) {
                out0[i] = SWAP16(inbuf[i * 2]);
                out1[i] = SWAP16(inbuf[i * 2 + 1]);
            }
        } else {
            for (i = 0; i < n; i++) {
                out0[i] = SWAP16(inbuf[i]);
            }
        }
        adxb->dec_nsmpl = n;
        adxb->dec_nbyte = adxb->nch * (n * 2);
        adxb->stat = ADXB_STAT_WRITE;
    }
    if (adxb->stat == ADXB_STAT_WRITE) {
        adxb->addwr_func(adxb->addwr_obj, adxb->dec_nbyte, adxb->dec_nsmpl);
        adxb->stat = ADXB_STAT_DONE;
    }
}

/* Container-specific header decode for ADXB_DecodeHeader: fills the handle from ADX_DecodeInfoWav,
 * clears the loop info, type ADXB_TYPE_WAV; returns the header length (0 on failure). */
Sint32 ADXB_DecodeHeaderWav(ADXB adxb, void *buf, Sint32 bsize) {
    Sint16 hdrlen;

    adxb->x02 = 1;
    if (ADX_DecodeInfoWav(buf, bsize, &hdrlen, &adxb->x0c, &adxb->bps, &adxb->x0f, &adxb->nch,
                          &adxb->sfreq, &adxb->total_nsmpl, &adxb->fmt, &adxb->x9c) < 0) {
        return 0;
    }
    adxb->x1c = 0;
    adxb->x26 = 0;
    adxb->x24 = 0;
    adxb->x34 = 0;
    adxb->x30 = 0;
    adxb->x2c = 0;
    adxb->x28 = 0;
    adxb->x20 = 0;
    adxb->out_nch = adxb->nch;
    adxb->x54 = adxb->x0f;
    adxb->out_fmt = adxb->fmt;
    adxb->pcmbuf = adxb->x3c;
    adxb->pcmbuf_nsmpl = adxb->x40;
    adxb->pcmbuf_chofst = adxb->x44;
    adxb->x8c = 0;
    adxb->x88 = 0;
    adxb->x98 = ADXB_TYPE_WAV;
    return hdrlen;
}

/* Finds the "fmt " and "data" chunks (4-byte aligned) in the header bytes: PCM only, rate, channels
 * (1..2), bits (16/8, or 4-bit ADPCM as x9c 2), block align and the sample count from the data
 * size. */
Sint32 ADX_DecodeInfoWav(Uint8 *buf, Sint32 bsize, Sint16 *hdrlen, Sint8 *x0c, Sint8 *bps,
                         Sint8 *x0f, Sint8 *nch, Sint32 *sfreq, Sint32 *nsmpl, Sint32 *fmt,
                         Sint16 *x9c) {
    Sint32 i;
    Uint8 *p;
    Sint32 dsize;

    for (i = 0; i < bsize; i++) {
        if (memcmp(buf + i, wav_FmtId(), 4) == 0) {
            break;
        }
    }
    if (i == bsize) {
        return -1;
    }
    if (i % 4 != 0) {
        return -1;
    }
    p = (Uint8 *)(i + (Sint32)buf);
    if ((Sint16)SWAP16(*(Uint16 *)(p + 8)) > 1) {
        return -1;
    }
    for (i = 0; i < bsize; i++) {
        if (memcmp(buf + i, wav_DataId(), 4) == 0) {
            break;
        }
    }
    if (i == bsize) {
        return -1;
    }
    dsize = SWAP32(*(Uint32 *)&buf[i + 4]);
    *hdrlen = i + 8;
    *x0c = -1;
    *sfreq = SWAP32(*(Uint32 *)(p + 0xC));
    *nch = SWAP16(*(Uint16 *)(p + 0xA));
    *bps = SWAP16(*(Uint16 *)(p + 0x16));
    *x0f = SWAP16(*(Uint16 *)(p + 0x14));
    *nsmpl = dsize / *x0f;
    *fmt = 1;
    if (*bps == 16) {
        *x9c = 0;
    } else if (*bps == 8) {
        *x9c = 1;
    } else if (*bps == 4) {
        *x0f = *nch * 2;
        *fmt = 4;
        *nsmpl = (dsize / 2) / *nch;
        *bps = 16;
        *x9c = 2;
    }
    if (*bps == 0) {
        return -1;
    }
    if (*x0f == 0) {
        return -1;
    }
    if (*nch <= 0 || *nch > 2) {
        return -1;
    }
    if (*sfreq == 0) {
        return -1;
    }
    return 0;
}
