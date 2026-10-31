/* CRI ADX renderer for the Nintendo AX sound library (AXRNA Ver.1.06): a handle owns up to two AX
 * voices fed from ARAM ring buffers (RNARES + SJRBF); PCM data is DMA'd from the input stream
 * joints.
 *
 * Source from the RE4 GameCube decompilation of the same file. Here the handle's play_cur field
 * (name inferred) remembers the last play offset so that AXRNA_SetPlaySw(1) resumes one block in
 * when playback stopped inside the first block, and stopping does not reset the play position or
 * the ring buffers. AXRNA_SetSrcType, AXRNA_GetPlaySw and AXRNA_GetTransSw are linker-stripped
 * here (the last two are inlined); they are kept so that MWCC lays out .bss and the literals as
 * the original does, together with the order-only stand-in axrna_OrderStandIn for another
 * stripped function.
 *
 * Not yet matching: AXRNA_SetPlaySw differs in 3 words, all in the additions that form the
 * voice's end and current addresses (an operand order and a register number). */
#include "CRI/cri_xpt.h"
#include <string.h>
#include <dolphin/os.h>
#include <dolphin/ar.h>
#include <dolphin/ax.h>
#include <dolphin/mix.h>
#include "layout_check.h"
#include "CRI/sj.h"
#include "CRI/adx_rna.h"

#define AXRNA_MAX_OBJ 16
#define AXRNA_MAX_NCH 2
#define AXRNA_BUF_NSMPL 0x1000     /* ring buffer length in 16-bit samples */
#define AXRNA_ZERO_DAT_SIZE 0x1000 /* zero source of the silence fill, in bytes */
#define AXRNA_DEF_SFREQ 48000
#define AXRNA_DMA_ALIGN 32
#define AXRNA_PAN_MAX 15
#define AXRNA_VOL_MIN -999

/* AX voice parameter values (not in include/dolphin/ax.h) */
#define AX_PB_STATE_STOP 0
#define AX_PB_STATE_RUN 1
#define AX_PB_FORMAT_PCM16 10

/* AXRNA_OBJ.sw bits */
#define AXRNA_SW_TRANS 0x01
#define AXRNA_SW_PLAY 0x02
#define AXRNA_GET_PLAY_SW(rna) (((rna)->sw >> 1) & 1)
#define AXRNA_GET_TRANS_SW(rna) ((rna)->sw & 1)

#define AXRNA_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define AXRNA_MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct AXRNA_OBJ {
    Sint8 used;                      /* 0x00 */
    Uint8 sw;                        /* 0x01 AXRNA_SW_* bits */
    Sint8 maxnch;                    /* 0x02 */
    Sint8 nch;                       /* 0x03 */
    Sint32 play_pos;                 /* 0x04 last observed play position (-1: none) */
    AXVPB *voice[AXRNA_MAX_NCH];     /* 0x08 */
    RNARES res[AXRNA_MAX_NCH];       /* 0x10 */
    Sint32 buf[AXRNA_MAX_NCH];       /* 0x18 ARAM address of the ring buffer (16-bit units) */
    Sint32 bufsize;                  /* 0x20 */
    Sint32 sfreq;                    /* 0x24 */
    Uint32 arq_owner[AXRNA_MAX_NCH]; /* 0x28 */
    SJ sj[AXRNA_MAX_NCH];            /* 0x30 input stream joints */
    SJ sjrbf[AXRNA_MAX_NCH];         /* 0x38 ring buffer stream joints */
    SJCK trans_ck[AXRNA_MAX_NCH];    /* 0x40 data chunk being transferred */
    SJCK free_ck[AXRNA_MAX_NCH];     /* 0x50 ring buffer chunk being written */
    volatile Sint32 trans_busy[AXRNA_MAX_NCH]; /* 0x60 */
    Sint32 trans_smpl;                         /* 0x68 */
    Sint32 trans_total;                        /* 0x6C */
    volatile Sint32 flash_busy[AXRNA_MAX_NCH]; /* 0x70 */
    Sint32 flash_smpl;                         /* 0x78 */
    Sint32 flash_total;                        /* 0x7C */
    Sint32 bps;                                /* 0x80 */
    Sint32 outvol;                             /* 0x84 */
    Sint32 outpan[AXRNA_MAX_NCH];              /* 0x88 */
    Sint32 span;                               /* 0x90 */
    Sint32 auxa;                               /* 0x94 */
    Sint32 auxb;                               /* 0x98 */
    Sint32 fader;                              /* 0x9C */
    Sint16 adjsfreq_fg;                        /* 0xA0 */
    Sint16 adjsfreq_state;                     /* 0xA2 */
    Sint32 src_type;                           /* 0xA4 */
    ARQRequest arq[AXRNA_MAX_NCH];             /* 0xA8 */
    Sint32 play_cur; /* 0xE8 last play offset read by axrna_update_play (name inferred) */
} AXRNA_OBJ;
CHECK_SIZEOF(AXRNA_OBJ, 0xEC);

void axrna_end_flash(u32 req);
void axrna_end_trans(u32 req);
void axrna_update_play(AXRNA rna);
void axrna_voice_drop(void *p);

/* Matching workaround: as in adx_sugc.c, the pointer is volatile and AXRNA_Init reads it without
 * using the value. */
static const Char8 *const volatile axrna_build = "\nAXRNA Ver.1.06 Build:Feb 28 2005 15:48:38\n";

Sint32 axrna_def_src_type = AX_SRC_TYPE_LINEAR;
Sint32 axrna_pan_tbl[2 * AXRNA_PAN_MAX + 1] = {
    0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x21, 0x25, 0x29, 0x2D, 0x31, 0x35, 0x39, 0x40,
    0x44, 0x48, 0x4C, 0x51, 0x55, 0x59, 0x5D, 0x62, 0x66, 0x6A, 0x6E, 0x73, 0x77, 0x7B, 0x7F,
};

static Uint32 axrna_init_cnt = 0;
static void *axrna_zero_dat = NULL;
Sint32 axrna_def_adjsfreq_fg = 0;
static Sint32 axrna_foo_cnt = 0;
Sint32 axrna_update_pos = 0;
static Sint32 axrna_update_hist[32];
Uint8 axrna_zero_dat_real[AXRNA_ZERO_DAT_SIZE + 2 * AXRNA_DMA_ALIGN];
AXRNA_OBJ axrna_obj[AXRNA_MAX_OBJ];

/* Per-handle switch for the 32028.5 Hz DSP rate correction applied by AXRNA_SetSfreq. */
void AXRNA_SetAdjsfreqFlg(AXRNA rna, Sint32 flg) {
    if (rna == NULL) {
        return;
    }
    rna->adjsfreq_fg = flg;
}

/* Sets the AX sample-rate converter type for the voices and marks it changed (adjsfreq_state). */
void AXRNA_SetSrcType(AXRNA rna, Sint32 type) {
    if (rna == NULL) {
        return;
    }
    rna->src_type = type;
    rna->adjsfreq_state = 1;
}

/* Matching workaround: order-only stand-in for a stripped function (never called, discarded by
 * the linker): referencing the statics and the "OFF" / "ON " literals here puts them in the
 * original .bss and literal order. The body is not the original's. Passing the first literal to
 * RNAERR_CallErrFunc keeps the array alive; storing both literals through a parameter instead
 * changes the literal order (AXRNA_SetTransSw and AXRNA_Create then differ). Revisit when the
 * stripped body is found. */
static void axrna_OrderStandIn(void) {
    const Char8 *sw_str[2] = {"OFF", "ON "};

    axrna_foo_cnt = 0;
    axrna_update_hist[0] = 0;
    axrna_zero_dat_real[0] = 0;
    RNAERR_CallErrFunc((Char8 *)sw_str[0]);
}

/* Whether the voices are running (sw bit 1). */
Sint32 AXRNA_GetPlaySw(AXRNA rna) {
    if (rna == NULL) {
        return -1;
    }
    return AXRNA_GET_PLAY_SW(rna);
}

/* Whether PCM is being transferred into ARAM (sw bit 0). */
Sint32 AXRNA_GetTransSw(AXRNA rna) {
    if (rna == NULL) {
        return -1;
    }
    return AXRNA_GET_TRANS_SW(rna);
}

/* Not implemented on AX: returns 0 samples discarded. */
Sint32 AXRNA_DiscardData(AXRNA rna, Sint32 nsmpl) {
    return 0;
}

/* Not used on AX. */
Sint32 AXRNA_SetStmHdInfo(AXRNA rna, void *hdinfo) {
    return 0;
}

/* Records the PCM bit depth (always 16 here). */
void AXRNA_SetBitPerSmpl(AXRNA rna, Sint32 bps) {
    if (rna == NULL) {
        return;
    }
    rna->bps = bps;
}

/* Pan of one channel, -15 (left) .. 15 (right), mapped through axrna_pan_tbl to the MIX 0..0x7F
 * pan. */
void AXRNA_SetOutPan(AXRNA rna, Sint32 ch, Sint32 pan) {
    Sint32 p;

    if (rna == NULL) {
        return;
    }
    if (ch >= rna->maxnch) {
        return;
    }
    p = AXRNA_MIN(pan, AXRNA_PAN_MAX);
    p = AXRNA_MAX(p, -AXRNA_PAN_MAX);
    if (p == rna->outpan[ch]) {
        return;
    }
    rna->outpan[ch] = p;
    GCRNA_LockCs();
    if (rna->voice[ch] != NULL) {
        MIXSetPan(rna->voice[ch], axrna_pan_tbl[p + AXRNA_PAN_MAX]);
    }
    GCRNA_UnlockCs();
}

/* Output volume in 1/10 dB (0 .. -999) applied as the MIX input level of every voice. */
void AXRNA_SetOutVol(AXRNA rna, Sint32 vol) {
    Sint32 v;
    Sint32 i;

    if (rna == NULL) {
        return;
    }
    v = AXRNA_MIN(vol, 0);
    v = AXRNA_MAX(v, AXRNA_VOL_MIN);
    if (v == rna->outvol) {
        return;
    }
    rna->outvol = v;
    for (i = 0; i < rna->maxnch; i++) {
        GCRNA_LockCs();
        if (rna->voice[i] != NULL) {
            MIXSetInput(rna->voice[i], v);
        }
        GCRNA_UnlockCs();
    }
}

/* Programs the voices' SRC ratio for `sfreq` Hz; with adjsfreq_fg the rate is scaled by 1124/1125
 * (the DSP runs at 32028.5 Hz) and 32 kHz sources switch to the no-SRC type. */
void AXRNA_SetSfreq(AXRNA rna, Sint32 sfreq) {
    AXPBSRC src;
    Sint32 adj;
    Sint32 i;

    if (rna == NULL) {
        return;
    }
    rna->sfreq = sfreq;
    /* the AX DSP runs at 32028.5 Hz: sfreq * 1124 / 1125, rounded up */
    adj = (sfreq * 1124 + 1124) / 1125;
    for (i = 0; i < rna->maxnch; i++) {
        GCRNA_LockCs();
        if (rna->voice[i] != NULL) {
            if (rna->adjsfreq_fg == 1) {
                Uint32 f = adj;

                if (sfreq == 32000 && rna->adjsfreq_state == 0) {
                    AXRNA_SetSrcType(rna, AX_SRC_TYPE_NONE);
                }
                src.ratioHi = f / 32000;
                src.ratioLo = (f << 8) / 125;
            } else {
                src.ratioHi = sfreq / 32000;
                src.ratioLo = ((sfreq << 8) / 125) & 0xFFFF;
            }
            src.currentAddressFrac = 0;
            src.last_samples[0] = 0;
            src.last_samples[1] = 0;
            src.last_samples[2] = 0;
            src.last_samples[3] = 0;
            AXSetVoiceSrcType(rna->voice[i], rna->src_type);
            AXSetVoiceSrc(rna->voice[i], &src);
        }
        GCRNA_UnlockCs();
    }
}

/* Number of channels actually streamed (<= maxnch). */
void AXRNA_SetNumChan(AXRNA rna, Sint32 nch) {
    if (rna == NULL) {
        return;
    }
    rna->nch = nch;
}

static void AXRNA_ExecHndl(AXRNA rna);

/* Renderer server: runs AXRNA_ExecHndl on every live handle. */
void AXRNA_ExecServer(void) {
    Uint32 i;

    for (i = 0; i < AXRNA_MAX_OBJ; i++) {
        if (axrna_obj[i].used == 1) {
            AXRNA_ExecHndl(&axrna_obj[i]);
        }
    }
}

/* Moves decoded PCM from the input stream joints into the ARAM rings: takes the largest 32-byte
 * multiple that fits both the free ring space and the available data, DMAs it (ARQ, high priority)
 * and spins until axrna_end_trans releases it. Stops at the first channel with nothing to move. */
static void axrna_exec_trans(AXRNA rna) {
    SJCK data;
    SJCK data_rest;
    SJCK free;
    SJCK free_rest;
    Sint32 i;
    Sint32 n;

    for (i = 0; i < rna->nch; i++) {
        if (rna->voice[i] == NULL) {
            continue;
        }
        if (rna->trans_busy[i] != 0) {
            continue;
        }
        SJ_GetChunk(rna->sjrbf[i], SJ_CK_FREE, 0x2000, &free);
        SJ_GetChunk(rna->sj[i], SJ_CK_DATA, free.len, &data);
        n = AXRNA_MIN(data.len, free.len);
        n = (n / AXRNA_DMA_ALIGN) * AXRNA_DMA_ALIGN;
        SJ_SplitChunk(&free, n, &free, &free_rest);
        SJ_UngetChunk(rna->sjrbf[i], SJ_CK_FREE, &free_rest);
        SJ_SplitChunk(&data, n, &data, &data_rest);
        SJ_UngetChunk(rna->sj[i], SJ_CK_DATA, &data_rest);
        if (n == 0) {
            return;
        }
        if (data.len != free.len) {
            for (;;) {
            }
        }
        rna->trans_ck[i] = data;
        rna->free_ck[i] = free;
        rna->trans_smpl = n / sizeof(Sint16);
        DCFlushRange(rna->trans_ck[i].data, rna->trans_ck[i].len);
        rna->trans_busy[i] = 1;
        ARQPostRequest(&rna->arq[i], rna->arq_owner[i], ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH,
                       (u32)data.data, (u32)free.data, n, axrna_end_trans);
        while (rna->trans_busy[i] != 0) {
        }
    }
}

/* After the transfer stops (stream end) fills the ring with one buffer length of zeros so the
 * looping voice plays silence instead of stale data. */
static void axrna_exec_flash(AXRNA rna) {
    SJCK zero;
    SJCK zero_rest;
    Sint32 i;
    Sint32 n;

    if (rna->flash_total < rna->bufsize) {
        for (i = 0; i < rna->nch; i++) {
            if (rna->flash_busy[i] != 0) {
                continue;
            }
            SJ_GetChunk(rna->sjrbf[i], SJ_CK_FREE, 0x2000, &zero);
            n = (zero.len / AXRNA_DMA_ALIGN) * AXRNA_DMA_ALIGN;
            SJ_SplitChunk(&zero, n, &zero, &zero_rest);
            SJ_UngetChunk(rna->sjrbf[i], SJ_CK_FREE, &zero_rest);
            if (n == 0) {
                return;
            }
            rna->free_ck[i] = zero;
            rna->flash_smpl = n / sizeof(Sint16);
            DCFlushRange(axrna_zero_dat, AXRNA_ZERO_DAT_SIZE);
            rna->flash_busy[i] = 1;
            ARQPostRequest(&rna->arq[i], rna->arq_owner[i], ARQ_TYPE_MRAM_TO_ARAM,
                           ARQ_PRIORITY_HIGH, (u32)axrna_zero_dat, (u32)zero.data, n,
                           axrna_end_flash);
            while (rna->flash_busy[i] != 0) {
            }
        }
    }
}

/* One handle per server tick: account the samples the DSP consumed, then either transfer new PCM or
 * flush zeros once the transfer switch is off. */
static void AXRNA_ExecHndl(AXRNA rna) {
    if (rna == NULL) {
        return;
    }
    if (AXRNA_GetPlaySw(rna) == 1) {
        axrna_update_play(rna);
    }
    if (AXRNA_GetTransSw(rna) == 1) {
        axrna_exec_trans(rna);
    } else if (AXRNA_GetPlaySw(rna) == 1) {
        axrna_exec_flash(rna);
    }
}

/* ARQ completion callback of a zero-fill DMA: publishes the ring chunk as data and clears
 * flash_busy. */
void axrna_end_flash(u32 req) {
    Sint32 id;
    AXRNA rna;
    Sint32 ch;

    id = ((ARQRequest *)req)->owner & 0x7FFFFFFF;
    rna = &axrna_obj[id / AXRNA_MAX_NCH];
    ch = id % AXRNA_MAX_NCH;
    if (rna->flash_busy[ch] == 1) {
        SJ_PutChunk(rna->sjrbf[ch], SJ_CK_DATA, &rna->free_ck[ch]);
        rna->flash_busy[ch] = 0;
        if (ch == rna->nch - 1) {
            rna->flash_total += rna->flash_smpl;
        }
    }
}

/* ARQ completion callback of a PCM DMA: frees the input chunk, publishes the ARAM chunk as data and
 * clears trans_busy; totals the samples on the last channel. */
void axrna_end_trans(u32 req) {
    Sint32 id;
    AXRNA rna;
    Sint32 ch;

    id = ((ARQRequest *)req)->owner & 0x7FFFFFFF;
    rna = &axrna_obj[id / AXRNA_MAX_NCH];
    ch = id % AXRNA_MAX_NCH;
    if (rna->trans_busy[ch] == 1) {
        SJ_PutChunk(rna->sj[ch], SJ_CK_FREE, &rna->trans_ck[ch]);
        SJ_PutChunk(rna->sjrbf[ch], SJ_CK_DATA, &rna->free_ck[ch]);
        rna->trans_busy[ch] = 0;
        if (ch == rna->nch - 1) {
            rna->trans_total += rna->trans_smpl;
        }
    }
}

/* Reads the last voice's current ARAM address, converts the advance since the previous tick (in
 * whole 0x800-sample steps, wrapping at the 0x1000-sample ring) into freed ring space on every
 * channel and records it in axrna_update_hist. Halts if the DSP address left the ring. */
void axrna_update_play(AXRNA rna) {
    SJCK ck;
    Sint32 nch = rna->nch;
    AXVPB *vpb = rna->voice[nch - 1];
    Sint32 prev = rna->play_pos;
    Sint32 cur;
    Sint32 diff;
    Sint32 i;
    Sint32 nbyte;

    if (vpb == NULL) {
        return;
    }
    /* currentAddressHi and currentAddressLo are read as one word (a single lwz in the original) */
    cur = *(Sint32 *)&vpb->pb.addr.currentAddressHi - rna->buf[nch - 1];
    rna->play_cur = cur;
    axrna_update_hist[axrna_update_pos++] = cur;
    if (axrna_update_pos == 32) {
        axrna_update_pos = 0;
    }
    if (cur < 0 || cur > rna->bufsize) {
        for (;;) {
        }
    }
    if (prev == -1) {
        if (cur == 0) {
            diff = 0;
        } else {
            rna->play_pos = 0;
            prev = 0;
        }
    }
    if (prev != -1) {
        if (cur > prev) {
            diff = cur - prev;
        } else {
            diff = AXRNA_BUF_NSMPL - (prev - cur);
        }
    }
    diff = (diff / 0x800) * 0x800;
    if (diff <= 0) {
        return;
    }
    nbyte = diff * sizeof(Sint16);
    for (i = 0; i < rna->nch; i++) {
        SJ_GetChunk(rna->sjrbf[i], SJ_CK_DATA, nbyte, &ck);
        SJ_PutChunk(rna->sjrbf[i], SJ_CK_FREE, &ck);
    }
    rna->play_pos += diff;
    if (rna->play_pos >= AXRNA_BUF_NSMPL) {
        rna->play_pos -= AXRNA_BUF_NSMPL;
    }
}

/* Free samples in the ARAM ring of the last channel. */
Sint32 AXRNA_GetNumRoom(AXRNA rna) {
    if (rna == NULL) {
        return -1;
    }
    return SJ_GetNumData(rna->sjrbf[rna->nch - 1], SJ_CK_FREE) / sizeof(Sint16);
}

/* Samples queued in ARAM and not yet consumed by the DSP, minus the zero fill. */
Sint32 AXRNA_GetNumData(AXRNA rna) {
    Sint32 n;

    if (rna == NULL) {
        return -1;
    }
    n = AXRNA_BUF_NSMPL - SJ_GetNumData(rna->sjrbf[rna->nch - 1], SJ_CK_FREE) / sizeof(Sint16);
    n -= rna->flash_total;
    if (n < 0) {
        n = 0;
    }
    return n;
}

/* 1: points every voice at its ARAM ring (PCM16, looping over the whole buffer) and runs it;
 * 0: stops the voices. */
void AXRNA_SetPlaySw(AXRNA rna, Sint32 sw) {
    AXPBADDR addr;
    Sint32 last;
    Sint32 loop;
    Sint32 cur;
    Sint32 i;
    Sint32 start;

    if (rna == NULL) {
        return;
    }
    if (sw == AXRNA_GetPlaySw(rna)) {
        return;
    }
    GCRNA_LockCs();
    if (sw == 1) {
        /* resume one block in when playback last stopped inside the first block */
        if (rna->play_cur == 0 || rna->play_cur > 0x800) {
            start = 0;
        } else {
            start = 0x800;
        }
        for (i = 0; i < rna->nch; i++) {
            if (rna->voice[i] != NULL) {
                last = rna->bufsize - 1;
                loop = rna->buf[i];
                cur = rna->buf[i] + start;
                last += loop;
                addr.loopFlag = 1;
                addr.format = AX_PB_FORMAT_PCM16;
                addr.loopAddressHi = loop >> 16;
                addr.loopAddressLo = loop;
                addr.endAddressHi = last >> 16;
                addr.endAddressLo = last;
                addr.currentAddressHi = cur >> 16;
                addr.currentAddressLo = cur;
                AXSetVoiceAddr(rna->voice[i], &addr);
                AXSetVoiceState(rna->voice[i], AX_PB_STATE_RUN);
            }
        }
        rna->sw |= AXRNA_SW_PLAY;
    } else if (sw == 0) {
        for (i = 0; i < rna->nch; i++) {
            if (rna->voice[i] != NULL) {
                AXSetVoiceState(rna->voice[i], AX_PB_STATE_STOP);
            }
        }
        rna->sw &= AXRNA_SW_TRANS;
    } else {
        RNAERR_CallErrFunc("E1070309:Illigal parameter(sw).\n");
    }
    GCRNA_UnlockCs();
}

/* 1: resets the rings, chunk records and ARQ requests and enables the PCM transfer; 0: waits (up to
 * 200 x 100000 spins per channel) for outstanding DMAs and disables the transfer. */
void AXRNA_SetTransSw(AXRNA rna, Sint32 sw) {
    Sint32 i;
    Sint32 j;
    Sint32 k;

    if (rna == NULL) {
        return;
    }
    if (sw == AXRNA_GetTransSw(rna)) {
        return;
    }
    if (sw == 1) {
        GCRNA_LockCs();
        for (i = 0; i < rna->nch; i++) {
            SJ_Reset(rna->sjrbf[i]);
            memset(&rna->trans_ck[i], 0, sizeof(SJCK));
            memset(&rna->free_ck[i], 0, sizeof(SJCK));
            memset(&rna->arq[i], 0, sizeof(ARQRequest));
            rna->trans_busy[i] = 0;
        }
        rna->trans_smpl = 0;
        rna->trans_total = 0;
        rna->flash_smpl = 0;
        rna->flash_total = 0;
        rna->play_pos = -1;
        rna->play_cur = 0;
        rna->sw |= AXRNA_SW_TRANS;
        GCRNA_UnlockCs();
    } else if (sw == 0) {
        for (i = 0; i < rna->nch; i++) {
            for (j = 0; j < 200; j++) {
                if (rna->trans_busy[i] == 0) {
                    break;
                }
                for (k = 0; k < 100000; k++) {
                }
            }
            if (j == 200) {
                RNAERR_CallErrFunc("E2071701:DMA transfer(data) to A-RAM did not finish.\n");
                return;
            }
            for (j = 0; j < 200; j++) {
                if (rna->flash_busy[i] == 0) {
                    break;
                }
                for (k = 0; k < 100000; k++) {
                }
            }
            if (j == 200) {
                RNAERR_CallErrFunc("E2071701:DMA transfer(flash) to A-RAM did not finish.\n");
                return;
            }
        }
        rna->sw &= AXRNA_SW_PLAY;
    } else {
        RNAERR_CallErrFunc("E1070308:Illigal parameter(sw).\n");
    }
}

/* Stops playback / transfer, destroys the ring stream joints and ARAM resources, releases the MIX
 * channels and AX voices, clears the slot. */
void AXRNA_Destroy(AXRNA rna) {
    Sint32 i;

    if (rna == NULL) {
        return;
    }
    AXRNA_SetPlaySw(rna, 0);
    AXRNA_SetTransSw(rna, 0);
    for (i = 0; i < rna->maxnch; i++) {
        if (rna->sjrbf[i] != NULL) {
            SJ_Destroy(rna->sjrbf[i]);
        }
        if (rna->res[i] != NULL) {
            RNARES_Destroy(rna->res[i]);
        }
        GCRNA_LockCs();
        if (rna->voice[i] != NULL) {
            MIXReleaseChannel(rna->voice[i]);
            AXFreeVoice(rna->voice[i]);
        }
        GCRNA_UnlockCs();
    }
    memset(rna, 0, sizeof(AXRNA_OBJ));
}

/* Takes an axrna_obj slot for `maxnch` channels fed from `sj[]`: an 0x2000-byte ARAM buffer
 * (RNARES) and a ring stream joint per channel, a priority-31 AX voice with a MIX channel;
 * defaults 48 kHz, 16-bit, hard left/right pan for stereo. */
AXRNA AXRNA_Create(SJ *sj, Sint32 maxnch) {
    AXRNA rna;
    Uint32 i;
    Uint32 j;

    if (maxnch <= 0) {
        RNAERR_CallErrFunc("E1070301:Illigal parameter(maxnch<=0).\n");
        return NULL;
    }
    if (sj == NULL) {
        RNAERR_CallErrFunc("E1070302:Illigal parameter(sj=null).\n");
        return NULL;
    }
    for (i = 0; i < maxnch; i++) {
        if (sj[i] == NULL) {
            RNAERR_CallErrFunc("E1070303:Illigal parameter(sj[]=null).\n");
            return NULL;
        }
    }
    for (i = 0; i < AXRNA_MAX_OBJ; i++) {
        if (axrna_obj[i].used == 0) {
            break;
        }
    }
    if (i == AXRNA_MAX_OBJ) {
        RNAERR_CallErrFunc("E1070304:Not enough RNA handle.\n");
        return NULL;
    }
    rna = &axrna_obj[i];
    rna->nch = maxnch;
    rna->maxnch = maxnch;
    for (j = 0; j < rna->maxnch; j++) {
        rna->sj[j] = sj[j];
    }
    rna->outvol = 0;
    rna->span = 0x7F;
    rna->auxa = AXRNA_VOL_MIN;
    rna->auxb = AXRNA_VOL_MIN;
    rna->fader = 0;
    for (j = 0; j < rna->maxnch; j++) {
        rna->arq_owner[j] = 0x80000000 | (i * AXRNA_MAX_NCH + j);
        if ((rna->res[j] = RNARES_Create()) == NULL) {
            RNAERR_CallErrFunc("E1070305:Can't create RNARES.\n");
            AXRNA_Destroy(rna);
            return NULL;
        }
        rna->buf[j] = RNARES_GetBuf(rna->res[j]);
        rna->bufsize = RNARES_GetBufSize(rna->res[j]);
        rna->sjrbf[j] = SJRBF_Create((void *)(rna->buf[j] * 2), rna->bufsize * 2, 0);
        if (rna->sjrbf[j] == NULL) {
            RNAERR_CallErrFunc("E1070306:Can't create SJ.\n");
            AXRNA_Destroy(rna);
            return NULL;
        }
        if ((rna->voice[j] = AXAcquireVoice(31, axrna_voice_drop, 0)) == NULL) {
            RNAERR_CallErrFunc("E1070307:Can't acquire voice(AX).\n");
            AXRNA_Destroy(rna);
            return NULL;
        }
        GCRNA_LockCs();
        if (rna->voice[j] != NULL) {
            MIXInitChannel(rna->voice[j], 3, rna->outvol, rna->auxa, rna->auxb, 0x40, rna->span,
                           rna->fader);
        }
        GCRNA_UnlockCs();
    }
    AXRNA_SetAdjsfreqFlg(rna, axrna_def_adjsfreq_fg);
    AXRNA_SetSrcType(rna, axrna_def_src_type);
    rna->adjsfreq_state = 0;
    AXRNA_SetSfreq(rna, AXRNA_DEF_SFREQ);
    AXRNA_SetBitPerSmpl(rna, 16);
    if (rna->maxnch == 2) {
        AXRNA_SetOutPan(rna, 0, -AXRNA_PAN_MAX);
        AXRNA_SetOutPan(rna, 1, AXRNA_PAN_MAX);
    } else {
        AXRNA_SetOutPan(rna, 0, 0);
    }
    rna->sw = 0;
    rna->used = 1;
    return rna;
}

/* AX voice-drop callback: releases the MIX channel and forgets the stolen voice. */
void axrna_voice_drop(void *p) {
    Sint32 i;
    Sint32 j;

    for (i = 0; i < AXRNA_MAX_OBJ; i++) {
        for (j = 0; j < AXRNA_MAX_NCH; j++) {
            if (p == axrna_obj[i].voice[j]) {
                MIXReleaseChannel(axrna_obj[i].voice[j]);
                axrna_obj[i].voice[j] = NULL;
                return;
            }
        }
    }
}

/* Destroys the live handles and the ARAM resources (reference counted). */
void AXRNA_Finish(void) {
    Sint32 i;

    if (--axrna_init_cnt == 0) {
        for (i = 0; i < AXRNA_MAX_OBJ; i++) {
            if (axrna_obj[i].used == 1) {
                AXRNA_Destroy(&axrna_obj[i]);
            }
        }
        memset(axrna_obj, 0, sizeof(axrna_obj));
        RNARES_Finish();
    }
}

/* Reserves the ARAM resources and the 32-byte-aligned zero buffer used for the silence fill. */
void AXRNA_Init(void) {
    axrna_build;
    if (axrna_init_cnt == 0) {
        RNARES_Init();
        memset(axrna_obj, 0, sizeof(axrna_obj));
        axrna_zero_dat = (void *)(((Uint32)axrna_zero_dat_real + (AXRNA_DMA_ALIGN - 1)) &
                                  ~(AXRNA_DMA_ALIGN - 1));
    }
    axrna_init_cnt++;
}

/* Registers the renderer error callback (RNAERR). */
void AXRNA_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj) {
    RNAERR_EntryErrFunc(func, obj);
}
