/* CRI ADX stream-joint decoder (adx_sjd.c, ADXSJD): feeds an ADXB block decoder from an input
 * stream joint and writes the PCM into one ring-buffer stream joint per channel.
 *
 * Bodies from the RE4 GameCube decompilation, in the newer source order of the Street Fighter III
 * 3rd Strike adx_sjd.c (Init/Finish first). The file is compiled with deferred inlining, so the
 * functions are emitted in reverse order and .bss is laid out in reverse declaration order. This
 * version reference-counts Init/Finish, adds ADXSJD_GetDecDtLen and checks the leading padding
 * and ADXB_GetIgnoreHdrErr in adxsjd_decode_prep; ADXSJD_EntryPl2SetSfreqFunc is not in it.
 */
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_b.h"
#include "CRI/adx_dcd.h"
#include "CRI/adx_sjd.h"
#include "layout_check.h"

#define ADXSJD_MAX_OBJ 16
#define ADXSJD_MAX_NCH 2

/* ADXSJD_OBJ.stat */
#define ADXSJD_STAT_STOP 0
#define ADXSJD_STAT_PREP 1
#define ADXSJD_STAT_DECODE 2
#define ADXSJD_STAT_END 3
#define ADXSJD_STAT_ERROR 4

/* container types (ADXB_GetFormat) whose input is passed through undecoded: AHX and the
 * unnamed types 11, 12, 15 and 20 */
#define ADXSJD_IS_RAW_FMT(fmt)                                                                     \
    ((fmt) == ADXB_TYPE_AHX || (fmt) == 11 || (fmt) == 12 || (fmt) == 20 || (fmt) == 15)

struct ADXSJD_OBJ {
    Sint8 used;                 /* 0x00 */
    Sint8 stat;                 /* 0x01 */
    Sint8 nch;                  /* 0x02 */
    Sint8 au;                   /* 0x03 AU stream: the input must hold data before a block starts */
    ADXB adxb;                  /* 0x04 */
    SJ sji;                     /* 0x08 input */
    SJ sjo[ADXSJD_MAX_NCH];     /* 0x0C outputs */
    SJCK inck;                  /* 0x14 input chunk handed to the decoder */
    SJCK outck[ADXSJD_MAX_NCH]; /* 0x1C output chunks being written */
    Sint32 dec_nsmpl;           /* 0x2C samples decoded */
    Sint32 dec_nbyte;           /* 0x30 input bytes consumed */
    Sint32 decpos;              /* 0x34 decode position in samples */
    Sint32 maxdecsmpl;          /* 0x38 */
    Sint32 trap_nsmpl;          /* 0x3C (-1: none) */
    Sint32 trap_cnt;            /* 0x40 */
    Sint32 trap_dtlen;          /* 0x44 */
    void (*trapfn)(void *obj);  /* 0x48 */
    void *trapobj;              /* 0x4C */
    void (*outfn)(void *obj, Sint32 ch, Uint8 *data, Sint32 len); /* 0x50 */
    void *outobj;                                                 /* 0x54 */
    Uint8 spsdinf[0x40];                                          /* 0x58 SPSD header copy */
    Sint32 hdrlen;                                                /* 0x98 */
    Sint32 lnksw;      /* 0x9C linked (concatenated) files */
    Sint32 pad_nsmpl;  /* 0xA0 silence samples to insert before the output */
    Sint32 skip_nsmpl; /* 0xA4 output samples to discard */
};
CHECK_SIZEOF(struct ADXSJD_OBJ, 0xA8);
typedef struct ADXSJD_OBJ ADXSJD_OBJ;

ADXSJD_OBJ adxsjd_obj[ADXSJD_MAX_OBJ];
void (*pl2setsfreqfunc)(ADXB adxb, Sint32 sfreq);
/* volatile like the other CRI reference counts (adx_crs.c); the count is reloaded after each
 * update. */
static volatile Sint32 adxsjd_init_cnt;

static void adxsjd_reset(ADXSJD sjd);
static void adxsjd_skip_out(ADXSJD sjd);
static void adxsjd_decode(ADXSJD sjd);
static void adxsjd_rawexec_end(ADXSJD sjd);
static void adxsjd_decexec_end(ADXSJD sjd);
static void adxsjd_pad_out(ADXSJD sjd);

void adxsjd_decexec_start(ADXSJD sjd);
Sint16 *adxsjd_get_wr(void *obj, Sint32 *pos, Sint32 *nsmpl, Sint32 *trap);
void adxsjd_decode_prep(ADXSJD sjd);

/* Library init (reference counted): the first user brings up the ADXB decoder and clears the
 * slots. */
void ADXSJD_Init(void) {
    if (adxsjd_init_cnt == 0) {
        ADXB_Init();
        memset(adxsjd_obj, 0, sizeof(adxsjd_obj));
    }
    adxsjd_init_cnt++;
}

/* Clears all decoder slots (library shutdown). */
void ADXSJD_Finish(void) {
    adxsjd_init_cnt--;
    if (adxsjd_init_cnt == 0) {
        memset(adxsjd_obj, 0, sizeof(adxsjd_obj));
    }
}

/* Takes a free adxsjd_obj slot; the ADXB writes into the ring buffer of the first output stream
 * joint (`sjo[0]`, its size and extra area in samples), the other channels are `chofst` samples
 * apart. */
ADXSJD ADXSJD_Create(SJ sji, Sint32 nch, SJ *sjo) {
    ADXSJD sjd;
    Sint32 i;
    void *buf;
    SJ sjo0;
    Sint32 bsize;
    Sint32 xsize;

    sjo0 = sjo[0];
    for (i = 0; i < ADXSJD_MAX_OBJ; i++) {
        if (adxsjd_obj[i].used == 0) {
            break;
        }
    }
    if (i == ADXSJD_MAX_OBJ) {
        return 0;
    }
    sjd = &adxsjd_obj[i];
    buf = SJRBF_GetBufPtr(sjo0);
    bsize = SJRBF_GetBufSize(sjo0) / 2;
    xsize = SJRBF_GetXtrSize(sjo0) / 2;
    sjd->adxb = ADXB_Create(nch, buf, bsize, bsize + xsize);
    if (sjd->adxb == 0) {
        return 0;
    }
    ADXB_EntryGetWrFunc(sjd->adxb, adxsjd_get_wr, sjd);
    sjd->sji = sji;
    sjd->nch = nch;
    for (i = 0; i < nch; i++) {
        sjd->sjo[i] = sjo[i];
    }
    sjd->stat = ADXSJD_STAT_STOP;
    adxsjd_reset(sjd);
    sjd->trapfn = 0;
    sjd->trapobj = 0;
    sjd->outfn = 0;
    sjd->outobj = 0;
    sjd->used = 1;
    return sjd;
}

/* Destroys the block decoder and clears the slot. */
void ADXSJD_Destroy(ADXSJD sjd) {
    ADXB adxb;

    if (sjd == 0) {
        return;
    }
    adxb = sjd->adxb;
    if (adxb != 0) {
        sjd->adxb = 0;
        ADXB_Destroy(adxb);
    }
    ADXCRS_Lock();
    memset(sjd, 0, sizeof(ADXSJD_OBJ));
    ADXCRS_Unlock();
}

/* Decoder state (ADXSJD_STAT_STOP/PREP/DECODE/END/ERROR). */
Sint32 ADXSJD_GetStat(ADXSJD sjd) {
    return sjd->stat;
}

/* Changes the input stream joint (ADXT swaps between the file stream and a memory stream). */
void ADXSJD_SetInSj(ADXSJD sjd, SJ sji) {
    sjd->sji = sji;
    ADXB_SetAhxInSj(sjd->adxb, sji);
}

/* Caps the samples decoded per server tick (ADXT: obufsize/svrfreq derived), also for the AHX
 * path. */
void ADXSJD_SetMaxDecSmpl(ADXSJD sjd, Sint32 nsmpl) {
    sjd->maxdecsmpl = nsmpl;
    ADXB_SetAhxDecSmpl(sjd->adxb, nsmpl);
}

/* Tells the AHX decoder that no more input will arrive (flushes its last block). */
void ADXSJD_TermSupply(ADXSJD sjd) {
    ADXB_AhxTermSupply(sjd->adxb);
}

/* Resets the per-play counters and enters PREP: the next tick parses the header at the input. */
void ADXSJD_Start(ADXSJD sjd) {
    adxsjd_reset(sjd);
    sjd->stat = ADXSJD_STAT_PREP;
}

/* per-play state */
static void adxsjd_reset(ADXSJD sjd) {
    sjd->hdrlen = 0;
    sjd->dec_nsmpl = 0;
    sjd->dec_nbyte = 0;
    sjd->decpos = 0;
    sjd->maxdecsmpl = 0x7FFFFFFF;
    sjd->trap_nsmpl = -1;
    sjd->trap_cnt = 0;
    sjd->trap_dtlen = 0;
    sjd->au = 0;
    sjd->pad_nsmpl = 0;
    sjd->skip_nsmpl = 0;
}

/* Stops the block decoder and the SJD (stat STOP). */
void ADXSJD_Stop(ADXSJD sjd) {
    ADXB_Stop(sjd->adxb);
    sjd->stat = ADXSJD_STAT_STOP;
}

/* analyse the header at the start of the input. */
void adxsjd_decode_prep(ADXSJD sjd) {
    SJCK ck;
    SJCK ck2;
    ADXB adxb;
    SJ sji;
    Sint32 hdrlen;
    Sint32 i;
    Sint32 fmt;

    sji = sjd->sji;
    adxb = sjd->adxb;
    SJ_GetChunk(sji, SJ_CK_DATA, 0xC800, &ck);
    for (i = 0; i < ck.len; i++) {
        if (((Sint8 *)ck.data)[i] != 0) {
            break;
        }
    }
    if (i % 2 == 1) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
        if (ADXB_GetIgnoreHdrErr() == 0) {
            ADXERR_CallErrFunc2("E04102501 adxsjd_decode_prep: ", "The data alignment is illegal.");
        }
        sjd->stat = ADXSJD_STAT_ERROR;
        return;
    }
    SJ_SplitChunk(&ck, i, &ck2, &ck);
    SJ_PutChunk(sji, SJ_CK_FREE, &ck2);
    if (ck.len < 16) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
        return;
    }
    hdrlen = ADXB_DecodeHeader(adxb, ck.data, ck.len);
    if (hdrlen == 0 || hdrlen > ck.len) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
        return;
    }
    if (hdrlen < 0) {
        if (adxb->x9a != 0) {
            ADXB_SetDefPrm(adxb);
            hdrlen = 0;
        } else {
            SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
            if (ADXB_GetIgnoreHdrErr() == 0) {
                ADXERR_CallErrFunc2("E03010901 ADXB_DecodeHeader: ",
                                    "Can not decode this file format.");
            }
            sjd->stat = ADXSJD_STAT_ERROR;
            return;
        }
    }
    sjd->hdrlen = hdrlen;
    if (ADXB_GetFormat(adxb) == ADXB_TYPE_AU) {
        sjd->au = 1;
    }
    if (ADXB_GetFormat(adxb) == ADXB_TYPE_SPSD) {
        memcpy(sjd->spsdinf, ck.data, (ck.len < 0x40) ? ck.len : 0x40);
    }
    fmt = ADXB_GetFormat(adxb);
    if (ADXSJD_IS_RAW_FMT(fmt)) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
    } else {
        SJ_SplitChunk(&ck, hdrlen, &ck, &ck2);
        SJ_PutChunk(sji, SJ_CK_FREE, &ck);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
    }
    if (adxb->xdc != 0 && pl2setsfreqfunc != 0) {
        pl2setsfreqfunc(adxb, adxb->sfreq);
    }
    sjd->stat = ADXSJD_STAT_DECODE;
}

/* ADXB write callback: where and how much PCM may be written. */
Sint16 *adxsjd_get_wr(void *obj, Sint32 *pos, Sint32 *nsmpl, Sint32 *trap) {
    ADXSJD s = obj;
    SJ sjo0;
    Sint32 i;
    Sint32 n;

    sjo0 = s->sjo[0];
    for (i = 0; i < ADXB_GetNumChan(s->adxb); i++) {
        SJ_GetChunk(s->sjo[i], SJ_CK_FREE, 0x4000, &s->outck[i]);
    }
    *pos = (s->outck[0].data - (Uint8 *)SJRBF_GetBufPtr(sjo0)) / 2;
    n = s->maxdecsmpl;
    if (s->outck[0].len / 2 < n) {
        n = s->outck[0].len / 2;
    }
    *nsmpl = n;
    if (s->trap_nsmpl >= 0) {
        *trap = s->trap_nsmpl - s->trap_cnt;
    } else {
        *trap = 0x1FFFFFFF;
    }
    return ADXB_GetPcmBuf(s->adxb);
}

/* start decoding the next block: hand the input chunk to the decoder, or finish at the footer. */
void adxsjd_decexec_start(ADXSJD sjd) {
    SJCK ck2;
    Sint16 ftrlen;
    ADXB adxb;
    SJ sji;
    Sint32 blksmpl;
    Sint32 len;
    Sint32 i;
    Sint32 n;

    adxb = sjd->adxb;
    sji = sjd->sji;
    if (sjd->trap_nsmpl >= 0 && sjd->trap_cnt >= sjd->trap_nsmpl && sjd->trapfn != 0) {
        sjd->trapfn(sjd->trapobj);
    }
    if (sjd->au == 1) {
        if (SJ_GetNumData(sji, SJ_CK_DATA) == 0) {
            sjd->stat = ADXSJD_STAT_END;
            return;
        }
    }
    SJ_GetChunk(sji, SJ_CK_DATA, 0x7FFFFFFF, &sjd->inck);
    if (ADXB_GetFormat(adxb) == ADXB_TYPE_ADX && sjd->inck.len >= 4 &&
        (Uint16) * (Sint16 *)sjd->inck.data == 0x8001) {
        sjd->stat = ADXSJD_STAT_END;
        if (ADX_DecodeFooter(sjd->inck.data, sjd->inck.len, &ftrlen) == 0) {
            if (ftrlen > sjd->inck.len) {
                SJ_UngetChunk(sji, SJ_CK_DATA, &sjd->inck);
                return;
            }
            SJ_SplitChunk(&sjd->inck, ftrlen, &sjd->inck, &ck2);
            SJ_PutChunk(sji, SJ_CK_FREE, &sjd->inck);
            SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
        }
        if (sjd->lnksw != 0) {
            /* skip the zero padding up to the next linked file */
            for (;;) {
                SJ_GetChunk(sji, SJ_CK_DATA, 0x7FFFFFFF, &sjd->inck);
                if (sjd->inck.len == 0) {
                    return;
                }
                len = sjd->inck.len;
                for (i = 0; i < len; i++) {
                    if (((Sint8 *)sjd->inck.data)[i] != 0) {
                        break;
                    }
                }
                SJ_SplitChunk(&sjd->inck, i, &sjd->inck, &ck2);
                SJ_PutChunk(sji, SJ_CK_FREE, &sjd->inck);
                SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
                if (i < len) {
                    break;
                }
            }
        }
        return;
    }
    if (sjd->decpos >= ADXB_GetTotalNumSmpl(sjd->adxb)) {
        sjd->stat = ADXSJD_STAT_END;
        SJ_UngetChunk(sji, SJ_CK_DATA, &sjd->inck);
        return;
    }
    blksmpl = ADXB_GetBlkSmpl(sjd->adxb);
    n = SJ_GetNumData(sjd->sjo[0], SJ_CK_FREE);
    if (n / 2 < blksmpl) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &sjd->inck);
        return;
    }
    if (ADXB_GetFormat(adxb) == ADXB_TYPE_AHX) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &sjd->inck);
    }
    ADXB_EntryData(adxb, (Sint16 *)sjd->inck.data, sjd->inck.len);
    ADXB_Start(adxb);
}

/* One tick of one decoder: insert pending silence, decode, then drop pending skip samples. */
void ADXSJD_ExecHndl(ADXSJD sjd) {
    adxsjd_pad_out(sjd);
    adxsjd_decode(sjd);
    adxsjd_skip_out(sjd);
}

/* discard the requested samples from the output */
static void adxsjd_skip_out(ADXSJD sjd) {
    SJCK ck;
    Sint32 i;
    Sint32 nbyte;
    Sint32 nsmpl;
    Sint32 n;

    if (sjd->skip_nsmpl <= 0) {
        return;
    }
    ADXCRS_Lock();
    nbyte = sjd->skip_nsmpl * 2;
    for (i = 0; i < sjd->nch; i++) {
        SJ_GetChunk(sjd->sjo[i], SJ_CK_DATA, 0x7FFFFFFF, &ck);
        n = ck.len;
        if (nbyte < n) {
            n = nbyte;
        }
        nbyte = n;
        SJ_UngetChunk(sjd->sjo[i], SJ_CK_DATA, &ck);
    }
    nsmpl = nbyte / 2;
    nbyte = nsmpl * 2;
    if (nbyte > 0) {
        for (i = 0; i < sjd->nch; i++) {
            SJ_GetChunk(sjd->sjo[i], SJ_CK_DATA, nbyte, &ck);
            SJ_PutChunk(sjd->sjo[i], SJ_CK_FREE, &ck);
        }
        sjd->skip_nsmpl -= nsmpl;
    }
    ADXCRS_Unlock();
}

/* State step: DECODE starts a block when the ADXB is idle, runs it, collects a finished block and
 * accounts raw (undecoded) formats; PREP parses the header. */
static void adxsjd_decode(ADXSJD sjd) {
    ADXB adxb;
    Sint32 stat;
    Sint16 fmt;

    stat = sjd->stat;
    if (stat == ADXSJD_STAT_DECODE) {
        adxb = sjd->adxb;
        if (ADXB_GetStat(adxb) == ADXB_STAT_STOP) {
            adxsjd_decexec_start(sjd);
        }
        ADXB_ExecHndl(adxb);
        if (ADXB_GetStat(adxb) == ADXB_STAT_DONE) {
            adxsjd_decexec_end(sjd);
        }
        fmt = adxb->x98;
        if (fmt == ADXB_TYPE_AHX || fmt == 20 || (Uint16)(fmt - 11) <= 1 || fmt == 15) {
            adxsjd_rawexec_end(sjd);
        }
    } else if (stat == ADXSJD_STAT_PREP) {
        adxsjd_decode_prep(sjd);
    }
}

/* raw formats: the decoder consumed the block without output */
static void adxsjd_rawexec_end(ADXSJD sjd) {
    ADXB adxb;
    Sint32 rem;
    Sint32 nbyte;
    Sint32 nsmpl;

    adxb = sjd->adxb;
    rem = ADXB_GetTotalNumSmpl(adxb);
    nbyte = ADXB_GetDecDtLen(adxb);
    nsmpl = ADXB_GetDecNumSmpl(adxb);
    rem -= sjd->decpos;
    if (nsmpl < rem) {
        rem = nsmpl;
    }
    sjd->dec_nsmpl += rem;
    sjd->dec_nbyte += nbyte;
    sjd->decpos += rem;
}

/* a block was decoded: hand the input to the free side and the PCM to the data side */
static void adxsjd_decexec_end(ADXSJD sjd) {
    SJCK ck1;
    SJCK ck2;
    Sint32 rem;
    ADXB adxb;
    Sint32 nbyte;
    SJ sji;
    Sint32 nsmpl;
    Sint32 i;

    adxb = sjd->adxb;
    sji = sjd->sji;
    rem = ADXB_GetTotalNumSmpl(adxb);
    nbyte = ADXB_GetDecDtLen(adxb);
    nsmpl = ADXB_GetDecNumSmpl(adxb);
    rem -= sjd->decpos;
    if (nsmpl < rem) {
        rem = nsmpl;
    }
    SJ_SplitChunk(&sjd->inck, nbyte, &ck1, &ck2);
    SJ_PutChunk(sji, SJ_CK_FREE, &ck1);
    SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
    for (i = 0; i < ADXB_GetNumChan(sjd->adxb); i++) {
        SJ_SplitChunk(&sjd->outck[i], rem * 2, &ck1, &ck2);
        if (sjd->outfn != 0) {
            sjd->outfn(sjd->outobj, i, ck1.data, ck1.len);
        }
        SJ_PutChunk(sjd->sjo[i], SJ_CK_DATA, &ck1);
        SJ_UngetChunk(sjd->sjo[i], SJ_CK_FREE, &ck2);
    }
    sjd->dec_nsmpl += rem;
    sjd->dec_nbyte += nbyte;
    sjd->decpos += rem;
    sjd->trap_cnt += rem;
    sjd->trap_dtlen += nbyte;
    ADXB_Reset(adxb);
}

/* insert the requested silence in front of the output */
static void adxsjd_pad_out(ADXSJD sjd) {
    SJCK ck;
    Sint32 i;
    Sint32 nbyte;
    Sint32 nsmpl;
    Sint32 n;

    if (sjd->pad_nsmpl <= 0) {
        return;
    }
    ADXCRS_Lock();
    nbyte = sjd->pad_nsmpl * 2;
    for (i = 0; i < sjd->nch; i++) {
        SJ_GetChunk(sjd->sjo[i], SJ_CK_FREE, 0x7FFFFFFF, &ck);
        n = ck.len;
        if (nbyte < n) {
            n = nbyte;
        }
        nbyte = n;
        SJ_UngetChunk(sjd->sjo[i], SJ_CK_FREE, &ck);
    }
    nsmpl = nbyte / 2;
    nbyte = nsmpl * 2;
    if (nbyte > 0) {
        for (i = 0; i < sjd->nch; i++) {
            SJ_GetChunk(sjd->sjo[i], SJ_CK_FREE, nbyte, &ck);
            memset(ck.data, 0, nbyte);
            SJ_PutChunk(sjd->sjo[i], SJ_CK_DATA, &ck);
        }
        sjd->pad_nsmpl -= nsmpl;
    }
    ADXCRS_Unlock();
}

/* Server tick: runs ADXSJD_ExecHndl on every live decoder (called from ADXT's main server). */
void ADXSJD_ExecServer(void) {
    Sint32 i;

    for (i = 0; i < ADXSJD_MAX_OBJ; i++) {
        if (adxsjd_obj[i].used == 1) {
            ADXSJD_ExecHndl(&adxsjd_obj[i]);
        }
    }
}

/* Input bytes the decoder has consumed. */
Sint32 ADXSJD_GetDecDtLen(ADXSJD sjd) {
    return sjd->dec_nbyte;
}

/* Total samples decoded since ADXSJD_Start. */
Sint32 ADXSJD_GetDecNumSmpl(ADXSJD sjd) {
    return sjd->dec_nsmpl;
}

/* Overrides the decode position in samples (ADXT sets it to the loop start after a loop jump). */
void ADXSJD_SetDecPos(ADXSJD sjd, Sint32 pos) {
    sjd->decpos = pos;
}

/* Enables linked playback: after an ADX footer the decoder skips the zero padding and continues
 * with the next concatenated file instead of ending. */
void ADXSJD_SetLnkSw(ADXSJD sjd, Sint32 sw) {
    sjd->lnksw = sw;
}

/* Registers the trap callback (`fn(obj)`) run by adxsjd_decexec_start when trap_cnt reaches
 * trap_nsmpl. */
void ADXSJD_EntryTrapFunc(ADXSJD sjd, void (*fn)(void *obj), void *obj) {
    sjd->trapfn = fn;
    sjd->trapobj = obj;
}

/* Sets the sample count at which the trap callback fires before the next block starts (-1: no
 * trap). ADXT uses it for the loop end and for the linked-file boundary. */
void ADXSJD_SetTrapNumSmpl(ADXSJD sjd, Sint32 nsmpl) {
    sjd->trap_nsmpl = nsmpl;
}

/* Presets the trap's decoded-sample counter. */
void ADXSJD_SetTrapCnt(ADXSJD sjd, Sint32 cnt) {
    sjd->trap_cnt = cnt;
}

/* Presets the input-byte counter of the trap (used by ADXT to align the loop-end trap to a file
 * position). */
void ADXSJD_SetTrapDtLen(ADXSJD sjd, Sint32 len) {
    sjd->trap_dtlen = len;
}

/* Container type id of the current stream (ADXB_TYPE_*). */
Sint32 ADXSJD_GetFormat(ADXSJD sjd) {
    return ADXB_GetFormat(sjd->adxb);
}

/* Sampling rate in Hz of the current stream. */
Sint32 ADXSJD_GetSfreq(ADXSJD sjd) {
    return ADXB_GetSfreq(sjd->adxb);
}

/* Output channel count (2 when a Pro Logic II encoder upmixes mono). */
Sint32 ADXSJD_GetNumChan(ADXSJD sjd) {
    return ADXB_GetNumChan(sjd->adxb);
}

/* Output bits per sample. */
Sint32 ADXSJD_GetOutBps(ADXSJD sjd) {
    return ADXB_GetOutBps(sjd->adxb);
}

/* Samples per block of the current stream (32 for ADX). */
Sint32 ADXSJD_GetBlkSmpl(ADXSJD sjd) {
    return ADXB_GetBlkSmpl(sjd->adxb);
}

/* Total samples per channel of the current stream. */
Sint32 ADXSJD_GetTotalNumSmpl(ADXSJD sjd) {
    return ADXB_GetTotalNumSmpl(sjd->adxb);
}

/* Loop count from the ADX header. */
Sint32 ADXSJD_GetNumLoop(ADXSJD sjd) {
    return ADXB_GetNumLoop(sjd->adxb);
}

/* Loop start sample position. */
Sint32 ADXSJD_GetLpStartPos(ADXSJD sjd) {
    return ADXB_GetLpStartPos(sjd->adxb);
}

/* Loop start byte offset in the stream; 0 for a 0 handle. */
Sint32 ADXSJD_GetLpStartOfst(ADXSJD sjd) {
    if (sjd == 0) {
        return 0;
    }
    return ADXB_GetLpStartOfst(sjd->adxb);
}

/* Loop end sample position of the stream. */
Sint32 ADXSJD_GetLpEndPos(ADXSJD sjd) {
    return ADXB_GetLpEndPos(sjd->adxb);
}

/* Loop end byte offset of the stream (ADXB_GetLpEndOfst). */
Sint32 ADXSJD_GetLpEndOfst(ADXSJD sjd) {
    return ADXB_GetLpEndOfst(sjd->adxb);
}

/* Default output volume from the stream's AINF chunk once decoding has begun; 0 otherwise. */
Sint16 ADXSJD_GetDefOutVol(ADXSJD sjd) {
    if (ADXB_GetAinfLen(sjd->adxb) > 0 &&
        (sjd->stat == ADXSJD_STAT_DECODE || sjd->stat == ADXSJD_STAT_END)) {
        return ADXB_GetDefOutVol(sjd->adxb);
    }
    return 0;
}

/* Default pan of channel `ch` from the stream's AINF chunk, only once the header has been decoded
 * (stat DECODE/END); -128 otherwise. */
Sint16 ADXSJD_GetDefPan(ADXSJD sjd, Sint32 ch) {
    if (ADXB_GetAinfLen(sjd->adxb) > 0 &&
        (sjd->stat == ADXSJD_STAT_DECODE || sjd->stat == ADXSJD_STAT_END)) {
        return ADXB_GetDefPan(sjd->adxb, ch);
    }
    return -128;
}

/* The 64-byte header copy kept for SPSD streams (spsdinf). */
void *ADXSJD_GetSpsdInfo(ADXSJD sjd) {
    return sjd->spsdinf;
}

/* Saves the block decoder's ADPCM history/key snapshot. */
void ADXSJD_TakeSnapshot(ADXSJD sjd) {
    ADXB_TakeSnapshot(sjd->adxb);
}

/* Restores the block decoder's ADPCM history/key snapshot (see ADXB_RestoreSnapshot); ADXT uses it
 * to resume after a loop/seek. */
void ADXSJD_RestoreSnapshot(ADXSJD sjd) {
    ADXB_RestoreSnapshot(sjd->adxb);
}
