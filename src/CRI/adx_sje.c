/* CRI ADX stream-joint encoder (adx_sje.c): encodes 16-bit PCM read from one stream joint per
 * channel into ADX (4-bit ADPCM, 18-byte blocks of 32 samples) written to an output stream joint:
 * header, block prediction/quantisation through the predictor filter objects (with an IIR decoder
 * model for the quantisation feedback), scale scrambling with the key sequence and the end code.
 * The zero-initialised scalars take their declaration place in .bss; the filter tables follow in
 * first-reference order (the unreferenced table clear helper), the handle table last.
 *
 * Source from the RE4 GameCube decompilation (ADXENC Ver.1.09). This build adds its own reference
 * count around ADXSJE_Init/ADXSJE_Finish; RE4's version/build-string functions and the data words
 * pointing at them have no counterpart here and are left out.
 */
#include <stddef.h>
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_dcd.h"
#include "CRI/adx_sje.h"
#include "CRI/sj.h"

#define ADXSJE_MAX_OBJ 8
#define ADXSJE_MAX_FLT 16
#define ADXSJE_MAX_NCH 2
#define ADXSJE_BLK_NSMPL 32

/* decoder model: the two-tap IIR the decoder runs on the dequantised residual */
typedef struct {
    Sint8 used; /* 0x00 */
    Sint8 x01;  /* 0x01 */
    Sint16 x02; /* 0x02 */
    Sint16 c1;  /* 0x04 */
    Sint16 c2;  /* 0x06 */
    Sint16 h1;  /* 0x08 */
    Sint16 h2;  /* 0x0A */
} ADXSJE_IIRFLT;

/* prediction filter of one channel */
typedef struct {
    Sint8 used;                   /* 0x00 */
    Sint8 x01;                    /* 0x01 */
    Sint16 x02;                   /* 0x02 */
    Sint16 c1;                    /* 0x04 */
    Sint16 c2;                    /* 0x06 */
    Sint16 h1;                    /* 0x08 */
    Sint16 h2;                    /* 0x0A */
    Sint32 x0c;                   /* 0x0C */
    Sint32 x10;                   /* 0x10 */
    Sint32 nsmpl;                 /* 0x14 */
    Sint16 buf[ADXSJE_BLK_NSMPL]; /* 0x18 prediction residuals */
    Sint8 q[ADXSJE_BLK_NSMPL];    /* 0x58 quantised residuals */
    Sint32 maxabs;                /* 0x78 */
    Sint16 scale;                 /* 0x7C */
    Float64 invscale;             /* 0x80 */
    ADXSJE_IIRFLT *iir;           /* 0x88 */
    Sint32 x8c;                   /* 0x8C */
} ADXSJE_PRDFLT;

struct ADXSJE_OBJ {
    Sint8 used;                                   /* 0x00 */
    Sint8 stat;                                   /* 0x01 1: header, 2: encoding, 3: done */
    Sint8 nch;                                    /* 0x02 */
    Sint8 stopflg;                                /* 0x03 */
    SJ sji[ADXSJE_MAX_NCH];                       /* 0x04 PCM inputs */
    SJ sjo;                                       /* 0x0C encoded output */
    Sint32 x10[6];                                /* 0x010 */
    Sint32 x28;                                   /* 0x028 */
    Sint32 nbyte;                                 /* 0x2C bytes written */
    Sint32 nsmpl;                                 /* 0x30 samples encoded */
    Sint32 x34;                                   /* 0x034 */
    Sint32 x38;                                   /* 0x038 */
    Sint32 total;                                 /* 0x3C samples to encode */
    Sint32 x40;                                   /* 0x040 */
    Sint32 blksmpl;                               /* 0x44 samples per block */
    Sint32 hdrlen;                                /* 0x48 */
    Sint32 fmt;                                   /* 0x4C */
    Sint32 blksz;                                 /* 0x50 */
    Sint32 bps;                                   /* 0x54 */
    Sint32 nch32;                                 /* 0x58 */
    Sint32 sfreq;                                 /* 0x5C */
    Sint32 x60;                                   /* 0x060 */
    Sint32 cutoff;                                /* 0x64 */
    Sint32 x68;                                   /* 0x068 */
    Sint32 nloop;                                 /* 0x6C */
    Sint32 lptype;                                /* 0x70 */
    Sint32 lpstart;                               /* 0x74 */
    Sint32 lpstartbyte;                           /* 0x78 */
    Sint32 lpend;                                 /* 0x7C */
    ADXSJE_PRDFLT *prd[ADXSJE_MAX_NCH];           /* 0x80 */
    Sint16 hist1[ADXSJE_MAX_NCH];                 /* 0x88 decoder history handed to the block */
    Sint16 hist2[ADXSJE_MAX_NCH];                 /* 0x8C */
    Sint16 pcm[ADXSJE_MAX_NCH][ADXSJE_BLK_NSMPL]; /* 0x90 */
    Sint16 res[ADXSJE_MAX_NCH][ADXSJE_BLK_NSMPL]; /* 0x110 residuals */
    Sint16 scl[ADXSJE_MAX_NCH][ADXSJE_BLK_NSMPL]; /* 0x190 scaled residuals */
    Sint16 deq[ADXSJE_MAX_NCH][ADXSJE_BLK_NSMPL]; /* 0x210 dequantised */
    Sint16 scale[ADXSJE_MAX_NCH];                 /* 0x290 */
    Sint16 pad294[2];                             /* 0x294 */
    Float64 invscale[ADXSJE_MAX_NCH];             /* 0x298 */
    Uint8 data[ADXSJE_MAX_NCH][16];               /* 0x2A8 encoded block bodies */
    Sint16 first[ADXSJE_MAX_NCH];                 /* 0x2C8 first samples (decoder history seed) */
    Sint16 first2[ADXSJE_MAX_NCH];                /* 0x2CC */
    Sint16 key;                                   /* 0x2D0 scale scrambling sequence */
    Sint16 keymul;                                /* 0x2D2 */
    Sint16 keyadd;                                /* 0x2D4 */
    Sint8 ainf_flg;                               /* 0x2D6 */
    Uint8 ainf_id[16];                            /* 0x2D7 */
    Sint8 pad2e7;                                 /* 0x2E7 */
    Sint16 ainf_vol;                              /* 0x2E8 */
    Sint16 ainf_pan[2];                           /* 0x2EA */
    Sint8 cinf_flg;                               /* 0x2EE */
    Sint8 pad2ef;                                 /* 0x2EF */
    void *cinf;                                   /* 0x2F0 */
    Sint32 cinf_len;                              /* 0x2F4 */
};
typedef struct ADXSJE_OBJ ADXSJE_OBJ;

Sint32 AdxGainDataMax = 0x1000;
Char8 *cri_str = "(c)CRI";

static Sint32 skg_init_count = 0;
/* the SKG error callback; unreferenced in this game but part of the original data */
static void (*skg_err_func)(void *obj, const Char8 *msg) = 0;
static void *skg_err_obj = 0;
/* volatile like the other CRI reference counts (adx_crs.c); the count is reloaded after each
 * update. */
static volatile Sint32 adxsje_init_cnt = 0;
static ADXSJE_PRDFLT adxsje_prdflt_obj[ADXSJE_MAX_FLT];
static ADXSJE_IIRFLT adxsje_iirflt_obj[ADXSJE_MAX_FLT];
static ADXSJE_OBJ adxsje_obj[ADXSJE_MAX_OBJ];

/* Unreferenced, so the linker strips it (body from RE4, not verified); its references place the
 * filter tables before the handle table in .bss. */
static void adxsje_flt_init(void) {
    memset(adxsje_prdflt_obj, 0, sizeof(adxsje_prdflt_obj));
    memset(adxsje_iirflt_obj, 0, sizeof(adxsje_iirflt_obj));
}

/* clamps: the arguments are evaluated twice */
#define ADXSJE_MAX(x, lo) (((x) < (lo)) ? (lo) : (x))
#define ADXSJE_MIN(x, hi) (((x) > (hi)) ? (hi) : (x))
#define ADXSJE_CLIP16(x) ADXSJE_MIN(ADXSJE_MAX((x), -0x8000), 0x7FFF)
#define ADXSJE_MAX2(x, lo) (((x) > (lo)) ? (x) : (lo))
#define ADXSJE_MIN2(x, hi) (((x) < (hi)) ? (x) : (hi))
#define ADXSJE_ABS(x) (((x) < 0) ? -(x) : (x))

/* quantisation step of the 4-bit residual (32767 / 7) */
#define ADXSJE_QSTEP 4681
#define ADXSJE_QHALF 2340

/* Prediction coefficients of the decoder-model IIR. */
static void adxsje_iirflt_set_coef(ADXSJE_IIRFLT *iir, Sint16 c1, Sint16 c2) {
    iir->c1 = c1;
    iir->c2 = c2;
}

/* Prediction coefficients of the encoder predictor. */
static void adxsje_prdflt_set_coef(ADXSJE_PRDFLT *prd, Sint16 c1, Sint16 c2) {
    prd->c1 = c1;
    prd->c2 = c2;
}

/* Per-channel prediction history of the handle (copied from the decoder-model IIR). */
static void adxsje_set_hist(ADXSJE sje, Sint32 ch, Sint16 h1, Sint16 h2) {
    sje->hist1[ch] = h1;
    sje->hist2[ch] = h2;
}

/* Prediction history of the decoder-model IIR. */
static void adxsje_iirflt_set_hist(ADXSJE_IIRFLT *iir, Sint16 h1, Sint16 h2) {
    iir->h1 = h1;
    iir->h2 = h2;
}

/* Prediction history of the encoder predictor. */
static void adxsje_prdflt_set_hist(ADXSJE_PRDFLT *prd, Sint16 h1, Sint16 h2) {
    prd->h1 = h1;
    prd->h2 = h2;
}

Sint32 adxsje_write68(void *src, Sint32 unit, Sint32 n, SJ sj);
Sint32 adxsje_calc_rsig(ADXSJE sje, Sint32 ch);
void adxsje_set_rsig(ADXSJE sje, Sint32 ch);
Sint32 adxsje_output_sdata(ADXSJE sje);
Sint32 adxsje_encode_data(ADXSJE sje);
Sint32 adxsje_write_end_code(ADXSJE sje);
static Sint32 adxsje_output_header(ADXSJE sje, SJ sjo);
static void adxsje_encode_exec(ADXSJE sje);

/* Runs ADXSJE_ExecHndl on every live encoder (ADXT_InsertHdrSfa runs it once, synchronously). */
void ADXSJE_ExecServer(void) {
    Sint32 i;

    for (i = 0; i < ADXSJE_MAX_OBJ; i++) {
        if (adxsje_obj[i].used == 1) {
            ADXSJE_ExecHndl(&adxsje_obj[i]);
        }
    }
}

/* header stage: wait for the first sample of every input (the decoder history seed), write the
 * header and program the filter coefficients. An inlined helper, which gives its counters the
 * original registers relative to the loops' stepping pointers.
 * Matching constraint (COMPILER-DIFF): the per-channel fields are reached through `Uint8 *` copies
 * of the handle stepped by 4/2 plus the member offset (the adx_tsvr ExecHndl idiom: `mr rIV, sje` +
 * offset loads), declared in the order that gives the original register assignment.
 * Indexing sje->sji[ch], sje->first[ch] and sje->prd[i] directly gives a different loop shape
 * (verified: 95% in any declaration order). */
static inline void adxsje_header_exec(ADXSJE sje) {
    Sint32 n;
    Sint16 c1;
    Sint16 c2;
    SJCK ck;
    ADXSJE_PRDFLT *prd;
    Uint8 *p3;
    Uint8 *p2;
    Sint32 i;
    Uint8 *p;
    SJ sjo;
    Sint32 ch;

    sjo = sje->sjo;
    p = (Uint8 *)sje;
    p2 = (Uint8 *)sje;
    for (ch = 0; ch < sje->nch32; ch++) {
        SJ_GetChunk(*(SJ *)(p + offsetof(ADXSJE_OBJ, sji)), SJ_CK_DATA, 2, &ck);
        if (ck.len == 0) {
            break;
        }
        *(Sint16 *)(p2 + offsetof(ADXSJE_OBJ, first)) =
            *(Sint16 *)(p2 + offsetof(ADXSJE_OBJ, first2)) = *(Sint16 *)ck.data;
        SJ_UngetChunk(*(SJ *)(p + offsetof(ADXSJE_OBJ, sji)), SJ_CK_DATA, &ck);
        p += 4;
        p2 += 2;
    }
    if (ch < sje->nch32) {
        return;
    }
    for (ch = 0; ch < sje->nch32; ch++) {
        sje->hist1[ch] = sje->first[ch];
        sje->hist2[ch] = sje->first2[ch];
    }
    n = adxsje_output_header(sje, sjo);
    if (n == 0) {
        return;
    }
    sje->nbyte += n;
    p3 = (Uint8 *)sje;
    for (i = 0; i < sje->nch32; i++) {
        prd = *(ADXSJE_PRDFLT **)(p3 + offsetof(ADXSJE_OBJ, prd));
        ADX_GetCoefficient((Sint16)sje->cutoff, sje->sfreq, &c1, &c2);
        adxsje_prdflt_set_coef(prd, c1, c2);
        adxsje_iirflt_set_coef(prd->iir, c1, c2);
        p3 += 4;
    }
    sje->stat = 2;
}

/* server callback signature: the object arrives as `void *` and is copied into a typed local, which
 * the original also keeps (matching constraint: a typed parameter changes the register order) */
void ADXSJE_ExecHndl(void *obj) {
    ADXSJE sje = obj;

    if (sje->stat == 1) {
        adxsje_header_exec(sje);
    } else if (sje->stat == 2) {
        adxsje_encode_exec(sje);
    }
}

/* Encoding stage (stat 2): encodes blocks while input is available and the sample total is not
 * reached, then writes the end code and finishes (stat 3). */
static void adxsje_encode_exec(ADXSJE sje) {
    Sint32 n;

    for (;;) {
        if (sje->stopflg == 0) {
            do {
                n = adxsje_encode_data(sje);
                if (n == 0) {
                    return;
                }
                sje->nbyte += n;
            } while (sje->nsmpl < sje->total);
            sje->stopflg = 1;
        } else {
            if (adxsje_write_end_code(sje) > 0) {
                sje->stat = 3;
            }
            return;
        }
    }
}

/* SFA configuration: channel count, rate, total samples, fixed 0x11C header length. */
void ADXSJE_SetConfigSfa(ADXSJE sje, Sint32 nch, Sint32 sfreq, Sint32 nsmpl) {
    sje->nch32 = nch;
    sje->sfreq = sfreq;
    sje->total = nsmpl;
    sje->x60 = nsmpl;
    sje->hdrlen = 0x11C;
}

/* Requests the end code after the current block. */
void ADXSJE_Stop(ADXSJE sje) {
    sje->stopflg = 1;
}

/* x68 leading silent samples are pushed into every input first */
void ADXSJE_Start(ADXSJE sje) {
    Sint32 ch;
    Sint32 n;
    SJCK ck;

    for (ch = 0; ch < sje->nch32; ch++) {
        n = sje->x68 << 1;
        if (n <= 0) {
            continue;
        }
        SJ_GetChunk(sje->sji[ch], SJ_CK_FREE, n, &ck);
        if (ck.len != n) {
            SJ_UngetChunk(sje->sji[ch], SJ_CK_FREE, &ck);
            for (;;) {
            }
        }
        memset(ck.data, 0, ck.len);
        SJ_PutChunk(sje->sji[ch], SJ_CK_DATA, &ck);
    }
    sje->x28 = 0;
    sje->nbyte = 0;
    sje->nsmpl = 0;
    sje->stopflg = 0;
    sje->stat = 1;
}

/* First free decoder-model IIR object of the 16. */
static ADXSJE_IIRFLT *adxsje_iirflt_create(void) {
    ADXSJE_IIRFLT *iir;
    Sint32 i;

    for (i = 0; i < ADXSJE_MAX_FLT; i++) {
        iir = &adxsje_iirflt_obj[i];
        if (iir->used == 0) {
            break;
        }
    }
    return (i < ADXSJE_MAX_FLT) ? iir : 0;
}

/* Takes a free predictor object, a decoder-model IIR (adxsje_iirflt_create never marks one used, so
 * predictors can share it) and a block of `nsmpl` residuals. */
static ADXSJE_PRDFLT *adxsje_prdflt_create(Sint32 nsmpl) {
    ADXSJE_PRDFLT *prd;
    ADXSJE_IIRFLT *iir;
    Sint32 i;

    iir = adxsje_iirflt_create();
    if (iir == 0) {
        return 0;
    }
    for (i = 0; i < ADXSJE_MAX_FLT; i++) {
        prd = &adxsje_prdflt_obj[i];
        if (prd->used == 0) {
            break;
        }
    }
    prd->nsmpl = nsmpl;
    prd->iir = iir;
    if (i < ADXSJE_MAX_FLT) {
        prd->used = 1;
    } else {
        prd = 0;
    }
    return prd;
}

/* Frees a predictor object. */
static void adxsje_prdflt_destroy(ADXSJE_PRDFLT *prd) {
    if (prd == 0) {
        return;
    }
    prd->used = 0;
    memset(prd, 0, sizeof(ADXSJE_PRDFLT));
}

/* Frees the predictors and the handle slot. */
void ADXSJE_Destroy(ADXSJE sje) {
    if (sje == 0) {
        return;
    }
    ADXCRS_Lock();
    if (sje->prd[0] != 0) {
        adxsje_prdflt_destroy(sje->prd[0]);
    }
    if (sje->prd[1] != 0) {
        adxsje_prdflt_destroy(sje->prd[1]);
    }
    memset(sje, 0, 4);
    ADXCRS_Unlock();
}

/* Takes an encoder slot reading `nch` PCM stream joints and writing ADX to `sjo`: defaults 44.1
 * kHz, 4-bit 18-byte blocks (32 samples), 500 Hz cut-off, no loop, no AINF, unbounded length. */
ADXSJE ADXSJE_Create(Sint32 nch, SJ *sji, SJ sjo) {
    ADXSJE sje;
    Sint32 i;

    for (i = 0; i < ADXSJE_MAX_OBJ; i++) {
        if (adxsje_obj[i].used == 0) {
            break;
        }
    }
    if (i == ADXSJE_MAX_OBJ) {
        return 0;
    }
    sje = &adxsje_obj[i];
    sje->nch = nch;
    for (i = 0; i < nch; i++) {
        sje->sji[i] = sji[i];
    }
    sje->sjo = sjo;
    sje->stat = 0;
    sje->x28 = 0;
    sje->nbyte = 0;
    sje->nsmpl = 0;
    sje->x34 = 0x7FFF0000;
    sje->hdrlen = ADX_CalcHdrInfoLen(0, 0, 4, 4);
    sje->fmt = 3;
    sje->nch32 = nch;
    sje->sfreq = 44100;
    sje->bps = 4;
    sje->blksz = 18;
    sje->blksmpl = (sje->blksz - 2) * 8 / sje->bps;
    sje->x60 = 0x7FFF0000;
    sje->cutoff = 500;
    sje->x68 = 0;
    sje->nloop = 0;
    sje->lptype = 0;
    sje->lpstart = 0;
    sje->lpstartbyte = 0;
    sje->lpend = 0;
    sje->x38 = 0;
    sje->total = 0;
    sje->x40 = ((sje->x60 - 1) / sje->blksmpl + 1) * sje->blksmpl;
    sje->prd[0] = adxsje_prdflt_create(sje->blksmpl);
    sje->prd[1] = adxsje_prdflt_create(sje->blksmpl);
    sje->first[0] = sje->first2[0] = 0;
    sje->first[1] = sje->first2[1] = 0;
    sje->ainf_flg = 0;
    sje->ainf_vol = 0;
    sje->ainf_pan[0] = -128;
    sje->ainf_pan[1] = -128;
    memset(sje->ainf_id, 0, 16);
    sje->used = 1;
    return sje;
}

/* Clears the handles (and releases the shared SKG counter). */
void ADXSJE_Finish(void) {
    adxsje_init_cnt--;
    if (adxsje_init_cnt == 0) {
        skg_init_count--;
        memset(adxsje_obj, 0, sizeof(adxsje_obj));
    }
}

void ADXSJE_Init(void) {
    if (adxsje_init_cnt == 0) {
        skg_init_count++;
        memset(adxsje_obj, 0, sizeof(adxsje_obj));
    }
    adxsje_init_cnt++;
}

/* Copies len bytes from src into the output stream joint if there is room. */
static void adxsje_put(SJ sj, void *src, Sint32 len) {
    SJCK ck;

    SJ_GetChunk(sj, SJ_CK_FREE, len, &ck);
    if (ck.len < len) {
        SJ_UngetChunk(sj, SJ_CK_FREE, &ck);
    } else {
        memcpy(ck.data, src, len);
        SJ_PutChunk(sj, SJ_CK_DATA, &ck);
    }
}

/* Writes one 16-bit word into the output stream joint if there is room. */
static void adxsje_put16(SJ sj, void *src) {
    SJCK ck;

    SJ_GetChunk(sj, SJ_CK_FREE, 2, &ck);
    if (ck.len < 2) {
        SJ_UngetChunk(sj, SJ_CK_FREE, &ck);
    } else {
        /* Matching constraint: the write cursor `dp` and its (unused) post-increment keep the
         * ck.data load ahead of the value load; storing through ck.data directly, or without the
         * increment, evaluates the value first (verified: 94%). */
        Sint16 *dp = (Sint16 *)ck.data;
        *dp++ = *(Sint16 *)src;
        SJ_PutChunk(sj, SJ_CK_DATA, &ck);
    }
}

#define ADXSJE_WRITE(p, unit, n)                                                                   \
    if (adxsje_write68((p), (unit), (n), sjo) != (n)) {                                            \
        goto err;                                                                                  \
    }

/* the ADX header: fixed part, loop records, the AINF and CINF chunks, zero padding and the
 * copyright; returns its size (plus the 4 bytes of the id/offset) or 0 when there is no room */
static Sint32 adxsje_output_header(ADXSJE sje, SJ sjo) {
    SJCK ck;
    Sint32 v32;
    Uint16 sig;
    Sint16 v16;
    Sint16 hdr;
    Sint8 v8;
    Sint32 padlen;
    Sint32 copylen;
    Sint32 room;
    Sint32 len;
    Sint32 i;

    copylen = strlen(cri_str);
    SJ_GetChunk(sjo, SJ_CK_FREE, 0x7FFFFFFF, &ck);
    room = ck.len;
    SJ_UngetChunk(sjo, SJ_CK_FREE, &ck);
    if (room < sje->hdrlen + 4) {
        return 0;
    }
    sig = 0x8000;
    ADXSJE_WRITE(&sig, 2, 1);
    hdr = sje->hdrlen;
    ADXSJE_WRITE(&hdr, 2, 1);
    v8 = sje->fmt;
    ADXSJE_WRITE(&v8, 1, 1);
    v8 = sje->blksz;
    ADXSJE_WRITE(&v8, 1, 1);
    v8 = sje->bps;
    ADXSJE_WRITE(&v8, 1, 1);
    v8 = sje->nch32;
    ADXSJE_WRITE(&v8, 1, 1);
    v32 = sje->sfreq;
    ADXSJE_WRITE(&v32, 4, 1);
    v32 = sje->x60;
    ADXSJE_WRITE(&v32, 4, 1);
    v16 = sje->cutoff;
    ADXSJE_WRITE(&v16, 2, 1);
    v8 = 4;
    ADXSJE_WRITE(&v8, 1, 1);
    if (sje->key == 0) {
        v8 = 0;
    } else {
        v8 = 8;
    }
    ADXSJE_WRITE(&v8, 1, 1);
    v32 = 0;
    ADXSJE_WRITE(&v32, 4, 1);
    v16 = sje->first[0];
    ADXSJE_WRITE(&v16, 2, 1);
    v16 = sje->first2[0];
    ADXSJE_WRITE(&v16, 2, 1);
    v16 = sje->first[1];
    ADXSJE_WRITE(&v16, 2, 1);
    v16 = sje->first2[1];
    ADXSJE_WRITE(&v16, 2, 1);
    len = 0x1C;
    if (sje->nloop > 0) {
        v16 = sje->x68;
        ADXSJE_WRITE(&v16, 2, 1);
        v16 = sje->nloop;
        ADXSJE_WRITE(&v16, 2, 1);
        i = 0;
        len = 0x20;
        for (; i < sje->nloop; i++) {
            v16 = i;
            ADXSJE_WRITE(&v16, 2, 1);
            v16 = 1;
            ADXSJE_WRITE(&v16, 2, 1);
            v32 = sje->lptype;
            ADXSJE_WRITE(&v32, 4, 1);
            v32 = sje->lpstart;
            ADXSJE_WRITE(&v32, 4, 1);
            v32 = sje->lpstartbyte;
            ADXSJE_WRITE(&v32, 4, 1);
            v32 = sje->lpend;
            ADXSJE_WRITE(&v32, 4, 1);
            len += 0x14;
        }
    }
    if (sje->ainf_flg == 1) {
        if (sje->nloop == 0) {
            v32 = 0;
            ADXSJE_WRITE(&v32, 4, 1);
            len += 4;
        }
        v32 = 0x41494E46;
        ADXSJE_WRITE(&v32, 4, 1);
        v32 = 0x18;
        ADXSJE_WRITE(&v32, 4, 1);
        ADXSJE_WRITE(sje->ainf_id, 1, sizeof(sje->ainf_id));
        v16 = sje->ainf_vol;
        ADXSJE_WRITE(&v16, 2, 1);
        v16 = 0;
        ADXSJE_WRITE(&v16, 2, 1);
        v16 = sje->ainf_pan[0];
        ADXSJE_WRITE(&v16, 2, 1);
        v16 = sje->ainf_pan[1];
        ADXSJE_WRITE(&v16, 2, 1);
        len += 0x20;
    }
    if (sje->cinf_flg == 1) {
        if (sje->nloop == 0) {
            v32 = 0;
            ADXSJE_WRITE(&v32, 4, 1);
            len += 4;
        }
        v32 = 0x43494E46;
        ADXSJE_WRITE(&v32, 4, 1);
        v32 = sje->cinf_len;
        ADXSJE_WRITE(&v32, 4, 1);
        len += 8;
        if (sje->cinf_len != 0 && sje->cinf != 0) {
            ADXSJE_WRITE(sje->cinf, 1, sje->cinf_len);
            len += sje->cinf_len;
        }
    }
    v8 = 0;
    padlen = sje->hdrlen - copylen;
    while (len < padlen) {
        ADXSJE_WRITE(&v8, 1, 1);
        len++;
    }
    ADXSJE_WRITE(cri_str, 1, copylen);
    len += copylen;
    return len + 4;
err:
    return 0;
}

/* the end code block (0x8001, its length) padded to the sector end when the file loops;
 * returns its size or 0 when the output has no room yet */
Sint32 adxsje_write_end_code(ADXSJE sje) {
    Sint32 n;
    Sint32 padlen;
    SJ sjo;
    Sint32 nbyte;
    Sint32 i;
    Uint16 v;
    Sint8 b;

    sjo = sje->sjo;
    nbyte = sje->nbyte;
    if (sje->nloop <= 0) {
        padlen = sje->blksz;
    } else {
        padlen = (nbyte + sje->blksz + 0x7FF) / 0x800 * 0x800 - nbyte;
    }
    n = padlen - 4;
    if (SJ_GetNumData(sjo, SJ_CK_FREE) < padlen) {
        return 0;
    }
    v = 0x8001;
    adxsje_put16(sjo, &v);
    v = n;
    adxsje_put16(sjo, &v);
    b = 0;
    for (i = 0; i < n; i++) {
        adxsje_put(sjo, &b, 1);
    }
    return padlen;
}

/* read n samples per channel into the block buffers; 0 when an input has less than n */
static Sint32 adxsje_read_pcm(ADXSJE sje, Sint16 **bufs, Sint32 n) {
    SJ *sji = sje->sji; /* declared first for the original register order (matching constraint) */
    Sint32 cnt = 0;
    Sint32 ch;
    SJCK ck;

    for (ch = 0; ch < sje->nch32; ch++) {
        cnt = SJ_GetNumData(sji[ch], SJ_CK_DATA) / sizeof(Sint16);
        if (cnt < n) {
            return 0;
        }
    }
    for (ch = 0; ch < sje->nch32; ch++) {
        cnt = 0;
        while (cnt < n) {
            SJ_GetChunk(sji[ch], SJ_CK_DATA, n * 2, &ck);
            memcpy(bufs[ch] + cnt, ck.data, ck.len);
            cnt += (Sint16)(ck.len / sizeof(Sint16));
            SJ_PutChunk(sji[ch], SJ_CK_FREE, &ck);
        }
    }
    return cnt;
}

/* predict, quantise and pack one block per channel. Matching constraints: a separate helper so its
 * `ch` gets the original register, and `iir = prd->iir` places the iir load right after prd. */
static void adxsje_encode_blocks(ADXSJE sje) {
    Sint32 ch;
    ADXSJE_PRDFLT *prd;
    ADXSJE_IIRFLT *iir;

    for (ch = 0; ch < sje->nch32; ch++) {
        adxsje_calc_rsig(sje, ch);
        prd = sje->prd[ch];
        iir = prd->iir;
        sje->scale[ch] = prd->scale;
        sje->invscale[ch] = prd->invscale;
        adxsje_set_hist(sje, ch, iir->h1, iir->h2);
        adxsje_set_rsig(sje, ch);
    }
}

/* zero the tail of a short last block (a helper for the same reason) */
static void adxsje_pad_pcm(ADXSJE sje, Sint16 **bufs, Sint32 n) {
    Sint32 ch;

    for (ch = 0; ch < sje->nch32; ch++) {
        if (bufs[ch] != 0) {
            memset(bufs[ch] + n, 0, (sje->blksmpl - n) * 2);
        }
    }
}

/* encode as many blocks as the output has room for and the inputs supply; returns the bytes
 * written */
Sint32 adxsje_encode_data(ADXSJE sje) {
    Sint32 nbyte;
    SJ sjo;
    Sint32 n;
    Sint32 cnt;
    Sint32 nenc;
    Sint16 *bufs[ADXSJE_MAX_NCH];

    sjo = sje->sjo;
    nbyte = 0;
    do {
        if (SJ_GetNumData(sjo, SJ_CK_FREE) / 18 / sje->nch32 <= 0) {
            break;
        }
        bufs[0] = sje->pcm[0];
        bufs[1] = sje->pcm[1];
        /* matching constraint: `?:` rather than if/else gives the original register order */
        n = (sje->blksmpl < sje->total - sje->nsmpl) ? sje->blksmpl : sje->total - sje->nsmpl;
        cnt = adxsje_read_pcm(sje, bufs, n);
        if (cnt == 0) {
            nenc = 0;
        } else {
            if (n < sje->blksmpl) {
                adxsje_pad_pcm(sje, bufs, n);
            }
            sje->nsmpl += sje->blksmpl;
            adxsje_encode_blocks(sje);
            nenc = sje->blksmpl;
        }
        if (nenc == 0) {
            break;
        }
        nbyte += adxsje_output_sdata(sje);
    } while (sje->nsmpl < sje->total);
    return nbyte;
}

/* one encoded block per channel: the scale (minus one, scrambled with the key sequence, zero for a
 * silent block) and the 16 body bytes; returns the bytes written */
Sint32 adxsje_output_sdata(ADXSJE sje) {
    Sint32 ch;
    Sint32 nbyte;
    SJ sjo;
    Sint32 v;
    Sint16 s;
    Sint8 b;

    ch = 0;
    nbyte = 0;
    sjo = sje->sjo;
    for (; ch < sje->nch32; ch++) {
        v = (Sint16)(sje->scale[ch] - 1) ^ sje->key;
        sje->key = sje->key * sje->keymul + sje->keyadd;
        sje->key &= 0x7FFF;
        if (((Sint32 *)sje->data[ch])[0] == 0 && ((Sint32 *)sje->data[ch])[1] == 0 &&
            ((Sint32 *)sje->data[ch])[2] == 0 && ((Sint32 *)sje->data[ch])[3] == 0) {
            v = 0;
        }
        s = v;
        b = s >> 8;
        adxsje_put(sjo, &b, 1);
        b = s;
        adxsje_put(sjo, &b, 1);
        adxsje_put(sjo, sje->data[ch], 16);
        nbyte += 18;
    }
    return nbyte;
}

/* pack the block of channel ch: the residuals scaled, quantised to bps bits and written as nibbles
 * into the block body. The body bytes are indexed (`dst[++n]`, n from -1): the strength reducer's
 * pointer is then a temporary initialised `addi p, dst, -1` off the dying `dst`, and the shift
 * amount is an expression (a variable `sft` ranks as an own local below the tail temporaries) */
void adxsje_set_rsig(ADXSJE sje, Sint32 ch) {
    Sint16 *res;
    Sint16 *scl;
    Sint16 *deq;
    Sint32 i;
    Sint32 nib;
    Sint32 nibs;
    ADXSJE_PRDFLT *prd;
    Sint8 *dst;
    Sint32 n;
    Sint32 v;
    Sint32 t;
    Sint32 q;
    Sint8 byte;

    res = sje->res[ch];
    scl = sje->scl[ch];
    deq = sje->deq[ch];
    dst = (Sint8 *)sje->data[ch];
    n = -1;
    nibs = 8 / sje->bps;
    prd = sje->prd[ch];
    nib = 0;
    for (i = 0; i < sje->blksmpl; i++) {
        res[i] = (prd == 0) ? 0 : prd->buf[i];
        t = (Sint32)((Float64)res[i] * prd->invscale);
        v = ADXSJE_CLIP16(t);
        scl[i] = v;
        if (scl[i] < 0) {
            q = (v - ADXSJE_QHALF) / ADXSJE_QSTEP;
        } else {
            q = (v + ADXSJE_QHALF) / ADXSJE_QSTEP;
        }
        q = ADXSJE_MIN2(ADXSJE_MAX(q, -8), 7);
        t = (Sint32)((Float64)(q * prd->scale) * prd->invscale);
        deq[i] = ADXSJE_CLIP16(t);
        if (i % nibs == 0) {
            nib = 1;
            dst[++n] = 0;
        }
        byte = dst[n];
        dst[n] = byte | (Sint8)((((Uint8)(q << (8 - sje->bps))) >> (8 - sje->bps))
                                << ((nibs - nib) * sje->bps));
        nib++;
    }
}

/* one sample through the prediction filter: the residual (clipped to 16 bits), its peak and the
 * history; the first sample of a block resets the residual buffer and the peak */
static void adxsje_prdflt_exec(ADXSJE_PRDFLT *prd, Sint16 s, Sint32 i) {
    Sint32 j;
    Sint32 pred;
    Sint16 v;

    if (i == 0) {
        for (j = 0; j < prd->nsmpl; j++) {
            prd->buf[j] = 0;
        }
        prd->maxabs = 0;
    }
    if (prd == 0) {
        return;
    }
    pred = s - ((prd->c1 * prd->h1) >> 12) - ((prd->c2 * prd->h2) >> 12);
    v = ADXSJE_CLIP16(pred);
    prd->buf[i] = v;
    if (ADXSJE_ABS(v) > prd->maxabs) {
        prd->maxabs = ADXSJE_ABS(v);
    }
    prd->h2 = prd->h1;
    prd->h1 = s;
}

/* the decoder model: dequantised residual plus the prediction, clipped, into the history */
static void adxsje_iirflt_exec(ADXSJE_IIRFLT *iir, Sint16 d) {
    Sint32 t;
    Sint16 h;

    if (iir == 0) {
        return;
    }
    t = d + ((iir->c1 * iir->h1 + iir->c2 * iir->h2) >> 12);
    h = ADXSJE_MIN2(ADXSJE_MAX2(t, -0x8000), 0x7FFF);
    iir->h2 = iir->h1;
    iir->h1 = h;
}

/* the block of channel ch: residuals and their scale, then a second pass with the decoder model
 * so the quantisation errors feed back into the prediction */
Sint32 adxsje_calc_rsig(ADXSJE sje, Sint32 ch) {
    Sint16 *pcm = sje->pcm[ch];
    Sint32 i;
    ADXSJE_PRDFLT *prd = sje->prd[ch];
    ADXSJE_IIRFLT *iir = prd->iir;
    Sint32 scale;
    Sint32 v;
    Sint32 q;
    Sint32 d;
    Sint32 t;
    Sint16 c;
    Sint16 smp;

    adxsje_prdflt_set_hist(prd, sje->hist1[ch], sje->hist2[ch]);
    for (i = 0; i < sje->blksmpl; i++) {
        adxsje_prdflt_exec(prd, pcm[i], i);
    }
    scale = (prd->maxabs - 1) / 7 + 1;
    prd->scale = ADXSJE_MAX2(ADXSJE_MIN2(scale, AdxGainDataMax), 1);
    if (prd->maxabs == 0) {
        prd->invscale = 32767.0;
    } else {
        prd->invscale = 32767.0 / (Float64)prd->maxabs;
    }
    adxsje_iirflt_set_hist(prd->iir, sje->hist1[ch], sje->hist2[ch]);
    /* re-derived before the second pass: the redefinition (a range-split copy of the same address
     * temporary, coalesced away) is what schedules the preheader's `li i, 0` above the pool `addi`
     * and lets the peephole fold the 0x4330 constant's `lfd` onto its `lis` */
    pcm = sje->pcm[ch];
    for (i = 0; i < sje->blksmpl; pcm++, i++) {
        smp = *pcm;
        adxsje_prdflt_set_hist(prd, iir->h1, iir->h2);
        adxsje_prdflt_exec(prd, smp, i);
        v = (prd == 0) ? 0 : prd->buf[i];
        t = (Sint32)((Float64)(Sint16)v * prd->invscale);
        c = ADXSJE_CLIP16(t);
        if (c < 0) {
            q = (c - ADXSJE_QHALF) / ADXSJE_QSTEP;
        } else {
            q = (c + ADXSJE_QHALF) / ADXSJE_QSTEP;
        }
        q = ADXSJE_MIN2(ADXSJE_MAX(q, -8), 7);
        prd->q[i] = q;
        d = q * prd->scale;
        c = ADXSJE_CLIP16(d);
        adxsje_iirflt_exec(prd->iir, c);
    }
    return 0;
}

/* write n items of unit bytes from src to the stream joint; returns n or 0 when there is no room */
Sint32 adxsje_write68(void *src, Sint32 unit, Sint32 n, SJ sj) {
    SJCK ck;
    Sint32 nbyte = unit * n;
    Sint32 i;

    SJ_GetChunk(sj, SJ_CK_FREE, nbyte, &ck);
    if (ck.len < nbyte) {
        SJ_UngetChunk(sj, SJ_CK_FREE, &ck);
        return 0;
    }
    if (unit == 4) {
        Uint32 *s = (Uint32 *)src;
        Uint32 *d = (Uint32 *)ck.data;
        for (i = 0; i < n; i++) {
            *d++ = *s++;
        }
    } else if (unit == 2) {
        Sint16 *s = (Sint16 *)src;
        Sint16 *d = (Sint16 *)ck.data;
        for (i = 0; i < n; i++) {
            *d++ = *s++;
        }
    } else if (unit == 1) {
        memcpy(ck.data, src, (Uint16)n);
    } else {
        for (;;) {
        }
    }
    SJ_PutChunk(sj, SJ_CK_DATA, &ck);
    return n;
}
