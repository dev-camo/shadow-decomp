/* CRI ADXT (ADX talk) handle API (adx_tlk.c, ADXT Ver.9.44): creation/destruction of the stream
 * playback handles over the ADX work layout, start/stop, pause, volume/pan/balance, status and time
 * queries, the sample-time bookkeeping used for the vsync-based time mode, silence insertion and the
 * header/end code tests.
 *
 * Bodies from the RE4 GameCube decompilation (Ver.9.31). Ver.9.44 wraps every public ADXT_* entry
 * in ADXCRS_Enter/ADXCRS_Leave around an adxt_* worker (the names in its error messages), adds
 * ADXT_StopWithoutLsc and ADXT_GetTimeReal (names from 3rd Strike's symbols; GetTimeReal inferred
 * from its 1/100 s result), the handle byte at 0xA9 that gates the header default volume and pan,
 * and the xc0 reset. The file is compiled with deferred inlining: .text is the reverse of this file,
 * the string literals follow codegen order and .bss is in reverse declaration order. Functions whose
 * public entry is unreferenced in this game are kept for their messages.
 */
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_amp.h"
#include "CRI/adx_dcd.h"
#include "CRI/adx_rna.h"
#include "CRI/adx_stm.h"
#include "CRI/adx_t.h"
#include "CRI/lsc.h"
#include "CRI/sj.h"

extern Uint32 __cvt_fp2unsigned(Float32 value);
/* Called only by unreferenced (linker-stripped) functions kept from the reference. */
extern void ADXSJD_SetKeyString(ADXSJD sjd, Char8 *str);
extern Sint32 ADXSJD_GetFmtBps(ADXSJD sjd);
extern Sint32 ADXSJD_GetHdrLen(ADXSJD sjd);
extern Sint32 ADXSTM_GetNumSctIbuf(ADXSTM stm);

/* work layout of one handle: the decoder output rings (0x3060 16-bit samples per channel), the
 * renderer work (0x40C0 bytes per channel) and the file stream ring (the rest, in sectors) */
#define ADXT_OBUF_NSMPL 0x3060
#define ADXT_OBUF_BSIZE 0x2000
#define ADXT_OBUF_STRIDE 0x2060
#define ADXT_RNA_WORK 0x40C0
#define ADXT_IBUF_XSIZE 0x24
#define ADXT_SCT_SIZE 0x800

/* ADXT_OBJ.stat (public) */
#define ADXT_STAT_PLAYEND 5

/* the vsync-based time position in adxt_time_unit units from the current sample time */
#define ADXT_SET_TIME_POS(adxt, nc, ts)                                                            \
    (adxt)->x9c = __cvt_fp2unsigned((Float32)adxt_time_unit * ((Float32)(nc) / (Float32)(ts)))

/* (.bss is the reverse of this order) */
static Float32 adxt_mviop_f;
Sint32 adxt_mviop_d;
static Sint32 adxt_mvtmp_d;
Sint32 adxt_time_unit;
static Float32 adxt_diff_av;
Sint32 adxt_fileid_buf[4];
Sint32 adxt_svrcnt_hndl;
Sint32 adxt_svrcnt_adxstm;
Sint32 adxt_svrcnt_adxf;
Sint32 adxt_svrcnt_rna;
Sint32 adxt_svrcnt_sjd;
Sint32 adxt_svrcnt;
static Sint32 adxt_time_adjust_cnt;
void (*pl2detachfunc)(ADXT adxt);
void (*ahxdetachfunc)(ADXT adxt);
static Sint32 adxt_last_svrfreq;
Sint32 adxt_def_svrfreq;
Sint32 adxt_tsvr_enter_cnt;
Sint32 adxt_time_mode;

static Sint32 adxstm_seteos_sct = 25;
Uint32 adxt_time_adjust_sw = 1;

Sint32 ADXT_GetStat(ADXT adxt);
Sint32 ADXT_GetNumChan(ADXT adxt);
Sint32 ADXT_GetSfreq(ADXT adxt);
Sint32 ADXT_GetOutPan(ADXT adxt, Sint32 ch);
void ADXT_SetOutPan(ADXT adxt, Sint32 ch, Sint32 pan);
Sint32 ADXT_GetOutBalance(ADXT adxt);
void ADXT_SetOutBalance(ADXT adxt, Sint32 bal);
Sint32 ADXT_GetNumSmplObuf(ADXT adxt);
void ADXT_GetTime(ADXT adxt, Sint32 *ncount, Sint32 *tscale);
void ADXT_SetLnkSw(ADXT adxt, Sint32 sw);
void ADXT_Stop(ADXT adxt);
void ADXT_Destroy(ADXT adxt);
void ADXT_ExecServer(void);

static inline void adxt_SetKeyString(ADXT adxt, Char8 *str);
static inline Sint32 adxt_GetOutBalance(ADXT adxt);
static inline void adxt_SetOutBalance(ADXT adxt, Sint32 bal);
static inline Sint32 adxt_InsertSilence(ADXT adxt, Sint32 nch, Sint32 nsmpl);
static inline Sint32 adxt_GetDecDtLen(ADXT adxt);
static inline Sint32 adxt_GetDecNumSmpl(ADXT adxt);
static inline void adxt_SetLnkSw(ADXT adxt, Sint32 sw);
static inline void adxt_SetTimeOfst(ADXT adxt, Sint32 ofst);
static inline Sint32 adxt_DiscardSmpl(ADXT adxt, Sint32 nsmpl);
static inline void adxt_TermSupply(ADXT adxt);
static inline Sint32 adxt_GetStatPause(ADXT adxt);
static inline void adxt_Pause(ADXT adxt, Sint32 sw);
static inline Bool adxt_IsReadyPlayStart(ADXT adxt);
static inline void adxt_SetWaitPlayStart(ADXT adxt, Sint32 sw);
static inline SJ adxt_GetInputSj(ADXT adxt);
static inline void adxt_SetLpFlg(ADXT adxt, Sint32 flg);
static inline Sint32 adxt_GetLpCnt(ADXT adxt);
static inline void adxt_ClearErrCode(ADXT adxt);
static inline Sint32 adxt_GetErrCode(ADXT adxt);
static inline void adxt_ExecServer(void);
static inline void adxt_SetAutoRcvr(ADXT adxt, Sint32 sw);
static inline Bool adxt_IsCompleted(ADXT adxt);
static inline Float32 adxt_GetIbufRemainTime(ADXT adxt);
static inline Sint32 adxt_GetNumSmplObuf(ADXT adxt);
static inline Sint32 adxt_GetNumSctIbuf(ADXT adxt);
static inline void adxt_SetReloadSct(ADXT adxt, Sint32 nsct);
static inline void adxt_ResetReloadTime(ADXT adxt);
static inline void adxt_SetReloadTime(ADXT adxt, Float32 time);
static inline void adxt_SetSvrFreq(ADXT adxt, Sint32 freq);
static inline void adxt_SetDefSvrFreq(Sint32 freq);
static inline Sint32 adxt_GetOutVol(ADXT adxt);
static inline void adxt_SetOutVol(ADXT adxt, Sint32 vol);
static inline Sint32 adxt_GetOutPan(ADXT adxt, Sint32 ch);
static inline void adxt_SetOutPan(ADXT adxt, Sint32 ch, Sint32 pan);
static inline Sint32 adxt_GetNumChan(ADXT adxt);
static inline Sint32 adxt_GetSfreq(ADXT adxt);
static inline Sint32 adxt_GetFmtBps(ADXT adxt);
static inline Sint32 adxt_GetHdrLen(ADXT adxt);
static inline Sint32 adxt_GetNumSmpl(ADXT adxt);
static inline Float32 adxt_GetMvTime(ADXT adxt);
static inline Sint32 adxt_GetTimeReal(ADXT adxt);
void adxt_GetTime(ADXT adxt, Sint32 *ncount, Sint32 *tscale);
static inline Sint32 adxt_GetStat(ADXT adxt);
static inline void adxt_Stop(ADXT adxt);
static inline void adxt_StopWithoutLsc(ADXT adxt);
static inline void adxt_StartSj(ADXT adxt, SJ sj);
static inline void adxt_DestroyAll(void);
void adxt_Destroy(ADXT adxt);
ADXT adxt_Create(Sint32 maxnch, void *work, Sint32 worksize);
static inline void adxt_get_smpl_time(ADXT adxt, Sint32 *ncount, Sint32 *tscale);
static inline void adxt_start_sjd(ADXT adxt, SJ sj);
void adxt_start_sj(ADXT adxt, SJ sj);

/* start the decoder on the input stream joint sj (shared by the file stream and SJ starts) */
static inline void adxt_start_sjd(ADXT adxt, SJ sj) {
    Sint32 i;

    for (i = 0; i < adxt->maxnch; i++) {
        SJ_Reset(adxt->sjo[i]);
    }
    ADXSJD_SetInSj(adxt->sjd, sj);
    adxt->sji = sj;
    ADXSJD_Start(adxt->sjd);
    adxt->stat = ADXT_ISTAT_DECINFO;
    adxt->lpcnt = 0;
    adxt->x71 = 0;
    adxt->lpendsct = 0x7FFFFFFF;
    adxt->trapnsmpl = -1;
    adxt->x9c = 0;
    adxt->decsmpl = 0;
    adxt->startvsync = adxt_vsync_cnt;
    adxt->xc0 = 0;
    if (adxt->amp != 0) {
        ADXAMP_Start(adxt->amp);
    }
}

/* Destroys every live handle (ADXT_Finish). */
static inline void adxt_DestroyAll(void) {
    Sint32 i;

    for (i = 0; i < ADXT_MAX_OBJ; i++) {
        if (adxt_obj[i].used == 1) {
            adxt_Destroy(&adxt_obj[i]);
        }
    }
}

/* Starts playback from a caller-supplied stream joint (mode 3, no file stream): the decoder reads the
 * ADX data the caller puts into `sj`, linked playback is enabled. This is how the Sofdec player feeds
 * the movie's audio track (sfd_adxt.c). */
static inline void adxt_StartSj(ADXT adxt, SJ sj) {
    if (adxt == 0 || sj == 0) {
        ADXERR_CallErrFunc1("E02080812 adxt_StartSj: parameter error");
        return;
    }
    adxt_Stop(adxt);
    ADXCRS_Lock();
    adxt_start_sjd(adxt, sj);
    adxt->mode = 3;
    adxt_SetLnkSw(adxt, 1);
    ADXCRS_Unlock();
}

/* Stops the renderer and the decoder without touching the load scheduler (destroys a memory input
 * joint) and returns the handle to STOP. */
static inline void adxt_StopWithoutLsc(ADXT adxt) {
    SJ sj;

    ADXCRS_Lock();
    ADXRNA_SetTransSw(adxt->rna, 0);
    ADXRNA_SetPlaySw(adxt->rna, 0);
    ADXSJD_Stop(adxt->sjd);
    if (adxt->mode == 2 && adxt->sji != 0) {
        sj = adxt->sji;
        adxt->sji = 0;
        SJ_Destroy(sj);
    }
    if (adxt->amp != 0) {
        ADXAMP_Stop(adxt->amp);
    }
    adxt->sji = 0;
    adxt->stat = ADXT_ISTAT_STOP;
    adxt->stmstart = 0;
    ADXCRS_Unlock();
}

/* Stops playback: releases the file binding, stops the load scheduler (mode 4), the renderer's
 * transfer and voices and the decoder, destroys a memory-source stream joint (mode 2) and returns the
 * handle to ADXT_ISTAT_STOP. */
static inline void adxt_Stop(ADXT adxt) {
    SJ sj;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080813 adxt_Stop: parameter error");
        return;
    }
    if (adxt->stm != 0) {
        ADXSTM_ReleaseFileNw(adxt->stm);
    }
    ADXCRS_Lock();
    if (adxt->mode == 4) {
        LSC_Stop(adxt->lsc);
        if (adxt->sji != 0) {
            SJ_Reset(adxt->sji);
        }
    }
    adxt_StopWithoutLsc(adxt);
    ADXCRS_Unlock();
}

/* Public handle state: 0 stop, 1 decinfo, 2 prep, 3 playing, 4 playend-wait, 5 playend, 6 error
 * (ADXT_ISTAT_*); -1 for a 0 handle. */
static inline Sint32 adxt_GetStat(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080814 adxt_GetStat: parameter error");
        return -1;
    }
    return adxt->stat;
}

/* the decoded sample count and its rate: samples handed to the decoder minus what is still
 * waiting in the renderer and the output stream joint, plus the earlier linked files */
static inline void adxt_get_smpl_time(ADXT adxt, Sint32 *ncount, Sint32 *tscale) {
    Sint32 dec;

    if (adxt->stat == ADXT_ISTAT_PLAYING || adxt->stat == ADXT_ISTAT_PLAYEND_WAIT) {
        *tscale = ADXSJD_GetSfreq(adxt->sjd);
        dec = ADXSJD_GetDecNumSmpl(adxt->sjd);
        *ncount =
            adxt->decsmpl + (dec - (ADXRNA_GetNumData(adxt->rna) + adxt_GetNumSmplObuf(adxt)));
    } else if (adxt->stat == ADXT_ISTAT_PLAYEND) {
        *ncount = ADXSJD_GetTotalNumSmpl(adxt->sjd);
        *tscale = ADXSJD_GetSfreq(adxt->sjd);
        *ncount = *ncount * (16 / ADXSJD_GetOutBps(adxt->sjd));
        *ncount = *ncount + adxt->decsmpl;
    } else {
        *ncount = 0;
        *tscale = 1;
    }
    *ncount = *ncount + adxt->timeofst;
    if (*ncount < 0) {
        *ncount = 0;
    }
}

/* Playback time in 1/100 s (name from 3rd Strike's symbols, inferred for this build). */
static inline Sint32 adxt_GetTimeReal(ADXT adxt) {
    Sint32 nc;
    Sint32 ts;

    adxt_GetTime(adxt, &nc, &ts);
    return (Sint32)(100.0f * ((Float32)nc / (Float32)ts));
}

/* (public entry unreferenced in this game) movie synchronisation: the decoded position in 1/100 vsync units */
static inline Float32 adxt_GetMvTime(ADXT adxt) {
    adxt_mvtmp_d = adxt_vsync_cnt - adxt->startvsync;
    adxt_mviop_d = adxt_mvtmp_d * 100;
    adxt_mviop_f = (Float32)adxt_mviop_d / 100.0f;
    return adxt_mviop_f;
}

/* Total sample count of the stream once the header is known (stat >= PREP), else 0. */
static inline Sint32 adxt_GetNumSmpl(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080817 adxt_GetNumSmpl: parameter error");
        return -1;
    }
    if (adxt->stat >= ADXT_ISTAT_PREP) {
        return ADXSJD_GetTotalNumSmpl(adxt->sjd);
    }
    return 0;
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetHdrLen(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080822 adxt_GetHdrLen: parameter error");
        return -1;
    }
    if (adxt->stat >= ADXT_ISTAT_PREP) {
        return ADXSJD_GetHdrLen(adxt->sjd);
    }
    return 0;
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetFmtBps(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080821 adxt_GetFmtBps: parameter error");
        return -1;
    }
    if (adxt->stat >= ADXT_ISTAT_PREP) {
        return ADXSJD_GetFmtBps(adxt->sjd);
    }
    return 0;
}

/* Sampling rate of the stream in Hz once the header is known, else 0. */
static inline Sint32 adxt_GetSfreq(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080819 adxt_GetSfreq: parameter error");
        return -1;
    }
    if (adxt->stat >= ADXT_ISTAT_PREP) {
        return ADXSJD_GetSfreq(adxt->sjd);
    }
    return 0;
}

/* Channel count of the stream once the header is known, else 0. */
static inline Sint32 adxt_GetNumChan(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080820 adxt_GetNumChan: parameter error");
        return -1;
    }
    if (adxt->stat >= ADXT_ISTAT_PREP) {
        return ADXSJD_GetNumChan(adxt->sjd);
    }
    return 0;
}

/* pan: -128 = default (the stream's own, +-15 for stereo), else the value plus the stream's;
 * everything centred when the output is mono */
static inline void adxt_SetOutPan(ADXT adxt, Sint32 ch, Sint32 pan) {
    Sint32 defpan;
    Sint32 p;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080825 adxt_SetOutPan: parameter error");
        return;
    }
    if (adxt->xa9 == 1) {
        defpan = ADXSJD_GetDefPan(adxt->sjd, ch);
        if (defpan == -128) {
            defpan = 0;
        }
    } else {
        defpan = 0;
    }
    if (adxt_output_mono_flag == 0) {
        if (pan == -128) {
            if (ADXSJD_GetNumChan(adxt->sjd) == 2) {
                p = (ch == 0) ? -15 : 15;
            } else {
                p = 0;
            }
            p += defpan;
        } else {
            p = pan + defpan;
        }
    } else {
        p = 0;
    }
    adxt->outpan[ch] = pan;
    if (ch < adxt->maxnch) {
        ADXRNA_SetOutPan(adxt->rna, ch, p);
    } else {
        ADXERR_CallErrFunc1("E8101208 adxt_SetOutPan: parameter error");
    }
}

/* The pan last set for channel `ch` (-15..15, -128 = the stream default). */
static inline Sint32 adxt_GetOutPan(ADXT adxt, Sint32 ch) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080826 adxt_GetOutPan: parameter error");
        return 0;
    }
    return adxt->outpan[ch];
}

/* Sets the output volume in 1/10 dB (0 = full, down to -999) and pushes it, plus the stream's own AINF
 * default volume, to the renderer. */
static inline void adxt_SetOutVol(ADXT adxt, Sint32 vol) {
    Sint32 defvol;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080823 adxt_SetOutVol: parameter error");
        return;
    }
    adxt->outvol = vol;
    if (adxt->xa9 == 1) {
        defvol = ADXSJD_GetDefOutVol(adxt->sjd);
    } else {
        defvol = 0;
    }
    ADXRNA_SetOutVol(adxt->rna, adxt->outvol + defvol);
}

/* The volume last set with ADXT_SetOutVol. */
static inline Sint32 adxt_GetOutVol(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080824 adxt_GetOutVol: parameter error");
        return 0;
    }
    return adxt->outvol;
}

/* Sets the assumed server call rate (Hz) used when sizing the per-call decode step of new handles
 * (ADXT_Init sets 60). */
static inline void adxt_SetDefSvrFreq(Sint32 freq) {
    adxt_def_svrfreq = freq;
    adxt_last_svrfreq = freq;
}

/* Per-handle server call rate (Hz); adxt_stat_decinfo derives maxdecsmpl = sfreq / svrfreq from it. */
static inline void adxt_SetSvrFreq(ADXT adxt, Sint32 freq) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080840 adxt_SetSvrFreq: parameter error");
        return;
    }
    adxt->svrfreq = freq;
    adxt_last_svrfreq = freq;
}

/* (public entry unreferenced in this game) reload threshold from a time in seconds */
static inline void adxt_SetReloadTime(ADXT adxt, Float32 time) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080838 adxt_SetReloadTime: parameter error");
        return;
    }
    adxt->reload_nsct = (Sint16)(time * (Float32)(ADXSJD_GetSfreq(adxt->sjd) * adxt->maxnch * 18 /
                                                  32 / ADXT_SCT_SIZE));
}

/* (public entry unreferenced in this game) */
static inline void adxt_ResetReloadTime(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E03111501 adxt_ResetReloadTime: parameter error");
        return;
    }
    adxt->reload_nsct = (Sint16)(0.85f * (Float32)adxt->ibuf_nsct);
}

/* (public entry unreferenced in this game) */
static inline void adxt_SetReloadSct(ADXT adxt, Sint32 nsct) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080839 adxt_SetReloadSct: parameter error");
        return;
    }
    adxt->reload_nsct = nsct;
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetNumSctIbuf(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080834 adxt_GetNumSctIbuf: parameter error");
        return -1;
    }
    return ADXSTM_GetNumSctIbuf(adxt->stm);
}

/* samples waiting in the first output stream joint (16-bit) */
static inline Sint32 adxt_GetNumSmplObuf(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080837 adxt_GetNumSmplObuf: parameter error");
        return -1;
    }
    if (adxt->sjo[0] != 0) {
        return SJ_GetNumData(adxt->sjo[0], SJ_CK_DATA) / 2;
    }
    return 0;
}

/* (public entry unreferenced in this game) playback time left in the input buffer, in seconds */
static inline Float32 adxt_GetIbufRemainTime(ADXT adxt) {
    Float32 rtime = 0.0f;
    Sint32 stat;
    Sint32 nch;
    Sint32 sfreq;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080835 adxt_GetIbufRemainTime: parameter error");
        return -1.0f;
    }
    stat = adxt_GetStat(adxt);
    nch = adxt_GetNumChan(adxt);
    sfreq = adxt_GetSfreq(adxt);
    if (stat >= ADXT_ISTAT_PREP && nch > 0 && sfreq > 0) {
        rtime = (Float32)(ADXSTM_GetNumSctIbuf(adxt->stm) * ADXT_SCT_SIZE / (nch * 18) * 32) /
                (Float32)sfreq;
    }
    return rtime;
}

/* (public entry unreferenced in this game) */
static inline Bool adxt_IsCompleted(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080802 adxt_IsCompleted: parameter error");
        return 0;
    }
    if (adxt->stat == ADXT_STAT_PLAYEND) {
        return 1;
    }
    return 0;
}

/* Automatic recovery switch: when set, a stream underrun restarts the renderer instead of stopping. */
static inline void adxt_SetAutoRcvr(ADXT adxt, Sint32 sw) {
    adxt->autorcvr = sw;
}

/* decoders, handles, renderer; the phase counter keeps a nested call (a callback calling the
 * server) out */
static inline void adxt_ExecServer(void) {
    Sint32 i;

    ADXCRS_Lock();
    if (adxt_tsvr_enter_cnt != 0) {
        ADXCRS_Unlock();
        return;
    }
    adxt_tsvr_enter_cnt = 1;
    ADXCRS_Unlock();
    ADXSJD_ExecServer();
    adxt_tsvr_enter_cnt = 2;
    for (i = 0; i < ADXT_MAX_OBJ; i++) {
        if (adxt_obj[i].used == 1) {
            ADXT_ExecHndl(&adxt_obj[i]);
        }
    }
    adxt_tsvr_enter_cnt = 3;
    ADXRNA_ExecServer();
    adxt_tsvr_enter_cnt = 0;
}

/* Error code of the handle (-1 after a stream controller / load scheduler error), 0 if none. */
static inline Sint32 adxt_GetErrCode(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080843 adxt_GetErrCode: parameter error");
        return -1;
    }
    return adxt->errcode;
}

static inline void adxt_ClearErrCode(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080844 adxt_ClearErrCode: parameter error");
        return;
    }
    adxt->errcode = 0;
    adxt->x64 = 0;
    adxt->x68 = 0;
    adxt->x6a = 0;
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetLpCnt(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080829 adxt_GetLpCnt: parameter error");
        return -1;
    }
    return adxt->lpcnt;
}

/* (public entry unreferenced in this game) */
static inline void adxt_SetLpFlg(ADXT adxt, Sint32 flg) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080828 adxt_SetLpFlg: parameter error");
        return;
    }
    adxt->lpsw = flg;
}

/* (public entry unreferenced in this game) */
static inline SJ adxt_GetInputSj(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080833 adxt_GetInputSj: parameter error");
        return 0;
    }
    return adxt->sji;
}

/* (public entry unreferenced in this game) */
static inline void adxt_SetWaitPlayStart(ADXT adxt, Sint32 sw) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080830 adxt_SetWaitPlayStart: parameter error");
        return;
    }
    adxt->pausesw = sw;
}

/* (public entry unreferenced in this game) */
static inline Bool adxt_IsReadyPlayStart(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080831 adxt_IsReadyPlayStart: parameter error");
        return 0;
    }
    return adxt->x71;
}

/* Pause (sw 1) / resume (sw 0): stops or restarts the AX renderer voices and rebases the vsync time
 * position so ADXT_GetTime stays continuous. */
static inline void adxt_Pause(ADXT adxt, Sint32 sw) {
    Sint32 stat;
    Sint32 mode;
    Sint32 nc;
    Sint32 ts;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080846 adxt_Pause: parameter error");
        return;
    }
    stat = adxt->stat;
    if (sw == adxt->x72) {
        return;
    }
    ADXCRS_Lock();
    adxt->x72 = sw;
    if (stat == ADXT_ISTAT_PLAYING || stat == ADXT_ISTAT_PLAYEND_WAIT) {
        if (sw == 1) {
            ADXRNA_SetPlaySw(adxt->rna, 0);
        } else {
            ADXRNA_SetPlaySw(adxt->rna, 1);
            adxt->startvsync = adxt_vsync_cnt;
        }
        mode = adxt_time_mode;
        adxt_time_mode = 0;
        adxt_GetTime(adxt, &nc, &ts);
        adxt_time_mode = mode;
        ADXT_SET_TIME_POS(adxt, nc, ts);
    }
    ADXCRS_Unlock();
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetStatPause(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080847 adxt_GetStatPause: parameter error");
        return 0;
    }
    return adxt->x72;
}

/* Tells the decoder that no more input data will arrive (end of a memory / stream-joint source). */
static inline void adxt_TermSupply(ADXT adxt) {
    ADXSJD_TermSupply(adxt->sjd);
}

/* drop nsmpl samples of renderer data (while paused); the server is run once so the decoder
 * refills, then the time position is resynchronised */
static inline Sint32 adxt_DiscardSmpl(ADXT adxt, Sint32 nsmpl) {
    Sint32 n;
    Sint32 mode;
    Sint32 nc;
    Sint32 ts;

    if (adxt->x72 == 0) {
        return 0;
    }
    n = ADXRNA_DiscardData(adxt->rna, nsmpl);
    ADXT_ExecServer();
    mode = adxt_time_mode;
    adxt_time_mode = 0;
    adxt_GetTime(adxt, &nc, &ts);
    adxt_time_mode = mode;
    ADXT_SET_TIME_POS(adxt, nc, ts);
    adxt->startvsync = adxt_vsync_cnt;
    return n;
}

/* Offset in samples added to every ADXT_GetTime result (the Sofdec player uses it to align the audio
 * clock with the video's first PTS). */
static inline void adxt_SetTimeOfst(ADXT adxt, Sint32 ofst) {
    adxt->timeofst = ofst;
}

/* Linked-file switch: after an ADX end code the decoder continues with the next concatenated stream
 * instead of finishing (forwarded to the ADXSJD). */
static inline void adxt_SetLnkSw(ADXT adxt, Sint32 sw) {
    adxt->lnksw = sw;
    if (adxt->sjd != 0) {
        ADXSJD_SetLnkSw(adxt->sjd, sw);
    }
}

static inline Sint32 adxt_GetDecNumSmpl(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080818 adxt_GetDecNumSmpl: parameter error");
        return -1;
    }
    return ADXSJD_GetDecNumSmpl(adxt->sjd);
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetDecDtLen(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E04041901 adxt_GetDecDtLen: parameter error");
        return -1;
    }
    return ADXSJD_GetDecDtLen(adxt->sjd);
}

/* put nsmpl samples of zero ADX blocks (18 bytes per channel per 32 samples) in front of the
 * decoder input, in two pieces around the ring buffer wrap; returns the samples inserted */
static inline Sint32 adxt_InsertSilence(ADXT adxt, Sint32 nch, Sint32 nsmpl) {
    Sint32 blksz;
    Sint32 nbyte;
    SJ sji;
    Sint32 n2;
    Sint32 n1;
    SJCK ck;
    SJCK ck2;

    sji = adxt->sji;
    if (adxt->sji == 0) {
        return 0;
    }
    blksz = nch * 18;
    nbyte = nsmpl / 32 * blksz;
    SJ_GetChunk(sji, SJ_CK_FREE, nbyte, &ck);
    n1 = ck.len / blksz * blksz;
    memset(ck.data, 0, n1);
    SJ_SplitChunk(&ck, n1, &ck, &ck2);
    SJ_PutChunk(sji, SJ_CK_DATA, &ck);
    SJ_UngetChunk(sji, SJ_CK_FREE, &ck2);
    nbyte -= n1;
    SJ_GetChunk(sji, SJ_CK_FREE, nbyte, &ck);
    n2 = ck.len / blksz * blksz;
    memset(ck.data, 0, n2);
    SJ_SplitChunk(&ck, n2, &ck, &ck2);
    SJ_PutChunk(sji, SJ_CK_DATA, &ck);
    SJ_UngetChunk(sji, SJ_CK_FREE, &ck2);
    return (n1 + n2) / blksz * 32;
}

/* (public entry unreferenced in this game) balance: -15..15, centred when the output is mono */
static inline void adxt_SetOutBalance(ADXT adxt, Sint32 bal) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080870 adxt_SetOutBalance: parameter error");
        return;
    }
    if (bal < -15) {
        adxt->outbal = -15;
    } else if (bal > 15) {
        adxt->outbal = 15;
    } else {
        adxt->outbal = bal;
    }
    if (adxt_output_mono_flag == 0) {
        ADXRNA_SetOutBalance(adxt->rna, adxt->outbal);
    } else {
        ADXRNA_SetOutBalance(adxt->rna, -128);
    }
}

/* (public entry unreferenced in this game) */
static inline Sint32 adxt_GetOutBalance(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080871 adxt_GetOutBalance: parameter error");
        return 0;
    }
    return adxt->outbal;
}

/* (public entry unreferenced in this game) */
static inline void adxt_SetKeyString(ADXT adxt, Char8 *str) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080860 adxt_SetKeyString: parameter error");
        return;
    }
    ADXSJD_SetKeyString(adxt->sjd, str);
}

ADXT ADXT_Create(Sint32 maxnch, void *work, Sint32 worksize) {
    ADXT ret;

    ADXCRS_Enter();
    ret = adxt_Create(maxnch, work, worksize);
    ADXCRS_Leave();
    return ret;
}

/* Allocates a stream playback handle for up to `maxnch` channels inside the caller's `work` buffer
 * (64-byte aligned): the decoder output rings (0x3060 samples per channel), the AX renderer work and
 * the file stream ring (the rest, in 2 KiB sectors; reload threshold 85% of it). Builds the stream
 * joints, the ADXSTM stream controller, the ADXSJD decoder, the ADXRNA renderer and an LSC scheduler. Returns 0 (with an error message) if no slot is
 * free or the parameters are bad. */
ADXT adxt_Create(Sint32 maxnch, void *work, Sint32 worksize) {
    ADXT adxt;
    Sint32 i;
    Uint8 *wk = (Uint8 *)(((Uint32)work + 63) & ~63);
    Sint32 wksize = worksize - ((Sint32)wk - (Sint32)work);
    Sint32 obufbytes;

    if (maxnch < 0 || work == 0 || worksize < 0) {
        ADXERR_CallErrFunc1("E02080804 adxt_Create: parameter error");
        return 0;
    }
    for (i = 0; i < ADXT_MAX_OBJ; i++) {
        if (adxt_obj[i].used == 0) {
            break;
        }
    }
    if (i == ADXT_MAX_OBJ) {
        ADXERR_CallErrFunc1("E03100801 adxt_Create: Too many handles.\n");
        return 0;
    }
    adxt = &adxt_obj[i];
    memset(adxt, 0, sizeof(ADXT_OBJ));
    adxt->maxnch = maxnch;
    obufbytes = (maxnch * ADXT_OBUF_NSMPL) << 1;
    adxt->ibuf = wk + obufbytes;
    adxt->ibufsize = (wksize - obufbytes - 0x124) / ADXT_SCT_SIZE * ADXT_SCT_SIZE;
    adxt->ibufxsize = ADXT_IBUF_XSIZE;
    adxt->wkend = adxt->ibufsize + adxt->ibufxsize + adxt->ibuf;
    adxt->obuf = wk;
    adxt->obufsize = ADXT_OBUF_BSIZE;
    adxt->obufstride = ADXT_OBUF_STRIDE;
    adxt->sji = 0;
    adxt->sjf = SJRBF_Create(adxt->ibuf, adxt->ibufsize, adxt->ibufxsize);
    if (adxt->sjf == 0) {
        adxt_Destroy(adxt);
        return 0;
    }
    if ((adxt->stm = ADXSTM_Create(adxt->sjf, 0)) == 0) {
        adxt_Destroy(adxt);
        return 0;
    }
    for (i = 0; i < maxnch; i++) {
        adxt->sjo[i] = SJRBF_Create(adxt->obuf + adxt->obufstride * i * 2, adxt->obufsize * 2,
                                    (adxt->obufstride - adxt->obufsize) * 2);
        if (adxt->sjo[i] == 0) {
            adxt_Destroy(adxt);
            return 0;
        }
    }
    if ((adxt->sjd = ADXSJD_Create(adxt->sjf, maxnch, adxt->sjo)) == 0) {
        adxt_Destroy(adxt);
        return 0;
    }
    if ((adxt->rna = ADXRNA_Create(adxt->sjo, maxnch, wk + maxnch * ADXT_RNA_WORK)) == 0) {
        adxt_Destroy(adxt);
        return 0;
    }
    if ((adxt->lsc = LSC_Create(adxt->sjf)) == 0) {
        adxt_Destroy(adxt);
        return 0;
    }
    LSC_SetStmHndl(adxt->lsc, adxt->stm);
    ADXCRS_Lock();
    adxt->svrfreq = adxt_def_svrfreq;
    adxt->ibuf_nsct = adxt->ibufsize / ADXT_SCT_SIZE;
    adxt->reload_nsct = (Sint16)(0.85f * (Float32)adxt->ibuf_nsct);
    adxt->outvol = 0;
    for (i = 0; i < maxnch; i++) {
        adxt->outpan[i] = -128;
    }
    adxt->outbal = 0;
    adxt->lpsw = 1;
    adxt->x54 = 0;
    adxt->x58 = 0;
    adxt->x5c = 0;
    adxt->errcode = 0;
    adxt->x64 = 0;
    adxt->x68 = 0;
    adxt->x6a = 0;
    adxt->autorcvr = 1;
    adxt->x72 = 0;
    adxt->timeofst = 0;
    adxt_SetLnkSw(adxt, 0);
    adxt->xa9 = 1;
    adxt->used = 1;
    ADXCRS_Unlock();
    return adxt;
}

void ADXT_Destroy(ADXT adxt) {
    ADXCRS_Enter();
    adxt_Destroy(adxt);
    ADXCRS_Leave();
}

/* Stops the stream, detaches the AHX / Pro Logic II extensions if any, releases the stream controller,
 * the decoder, the renderer and every stream joint, and frees the adxt_obj slot. */
void adxt_Destroy(ADXT adxt) {
    ADXSTM stm;
    Sint32 i;
    SJ sj;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080805 adxt_Destroy: parameter error");
        return;
    }
    if (ahxdetachfunc != 0) {
        ahxdetachfunc(adxt);
    }
    if (pl2detachfunc != 0) {
        pl2detachfunc(adxt);
    }
    if (adxt->used == 1) {
        adxt_Stop(adxt);
    }
    if (adxt->rna != 0) {
        void *rna = adxt->rna;
        adxt->rna = 0;
        ADXRNA_Destroy(rna);
    }
    if (adxt->sjd != 0) {
        void *sjd = adxt->sjd;
        adxt->sjd = 0;
        ADXSJD_Destroy(sjd);
    }
    stm = adxt->stm;
    if (stm != 0) {
        adxt->stm = 0;
        ADXSTM_EntryEosFunc(stm, 0, 0);
        ADXSTM_Destroy(stm);
    }
    if (adxt->lsc != 0) {
        void *lsc = adxt->lsc;
        adxt->lsc = 0;
        LSC_Destroy(lsc);
    }
    ADXCRS_Lock();
    if (adxt->sjf != 0) {
        sj = adxt->sjf;
        adxt->sjf = 0;
        SJ_Destroy(sj);
    }
    for (i = 0; i < adxt->maxnch; i++) {
        if (adxt->sjo[i] != 0) {
            sj = adxt->sjo[i];
            adxt->sjo[i] = 0;
            SJ_Destroy(sj);
        }
        if (adxt->sjx[i] != 0) {
            sj = adxt->sjx[i];
            adxt->sjx[i] = 0;
            SJ_Destroy(sj);
        }
        if (adxt->sjy[i] != 0) {
            sj = adxt->sjy[i];
            adxt->sjy[i] = 0;
            SJ_Destroy(sj);
        }
    }
    if (adxt->amp != 0) {
        void *amp = adxt->amp;
        adxt->amp = 0;
        ADXAMP_Destroy(amp);
    }
    memset(adxt, 0, sizeof(ADXT_OBJ));
    adxt->used = 0;
    ADXCRS_Unlock();
}

void ADXT_DestroyAll(void) {
    ADXCRS_Enter();
    adxt_DestroyAll();
    ADXCRS_Leave();
}

/* Starts the decoder on the input stream joint sj (same steps as adxt_start_sjd). */
void adxt_start_sj(ADXT adxt, SJ sj) {
    Sint32 i;

    for (i = 0; i < adxt->maxnch; i++) {
        SJ_Reset(adxt->sjo[i]);
    }
    ADXSJD_SetInSj(adxt->sjd, sj);
    adxt->sji = sj;
    ADXSJD_Start(adxt->sjd);
    adxt->stat = ADXT_ISTAT_DECINFO;
    adxt->lpcnt = 0;
    adxt->x71 = 0;
    adxt->lpendsct = 0x7FFFFFFF;
    adxt->trapnsmpl = -1;
    adxt->x9c = 0;
    adxt->decsmpl = 0;
    adxt->startvsync = adxt_vsync_cnt;
    adxt->xc0 = 0;
    if (adxt->amp != 0) {
        ADXAMP_Start(adxt->amp);
    }
}

/* Starts file playback: sizes the stream ring (reload threshold / total, sectors -> bytes), sets the
 * end-of-stream lead (adxstm_seteos_sct sectors), rewinds and binds the file range (`ofst`, `nsct`
 * sectors, no-wait) on the ADXSTM, starts the transfer and the decoder on the file stream joint. */
void adxt_start_stm(ADXT adxt, void *fname, void *dir, Sint32 ofst, Sint32 nsct) {
    ADXSTM_SetBufSize(adxt->stm, adxt->reload_nsct << 11, adxt->ibuf_nsct << 11);
    ADXSTM_SetEos(adxt->stm, adxstm_seteos_sct);
    ADXSTM_EntryEosFunc(adxt->stm, 0, 0);
    ADXSTM_Seek(adxt->stm, 0);
    ADXSTM_StopNw(adxt->stm);
    ADXSTM_ReleaseFileNw(adxt->stm);
    ADXSTM_BindFileNw(adxt->stm, fname, dir, ofst, nsct);
    ADXSTM_Start(adxt->stm);
    adxt_start_sjd(adxt, adxt->sjf);
}

void ADXT_StartSj(ADXT adxt, SJ sj) {
    ADXCRS_Enter();
    adxt_StartSj(adxt, sj);
    ADXCRS_Leave();
}

void ADXT_StopWithoutLsc(ADXT adxt) {
    ADXCRS_Enter();
    adxt_StopWithoutLsc(adxt);
    ADXCRS_Leave();
}

void ADXT_Stop(ADXT adxt) {
    ADXCRS_Enter();
    adxt_Stop(adxt);
    ADXCRS_Leave();
}

Sint32 ADXT_GetStat(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetStat(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_GetTime(ADXT adxt, Sint32 *ncount, Sint32 *tscale) {
    ADXCRS_Enter();
    adxt_GetTime(adxt, ncount, tscale);
    ADXCRS_Leave();
}

/* Playback time: sample mode (adxt_time_mode 0) returns decoded-and-rendered samples over the sampling
 * rate; vsync mode counts vsyncs since start in adxt_time_unit units and resynchronises to the sample
 * time when they drift more than 60 ms apart (adxt_time_adjust_cnt). The Sofdec player uses this to
 * slave the video clock to the audio. */
void adxt_GetTime(ADXT adxt, Sint32 *ncount, Sint32 *tscale) {
    Sint32 nc;
    Sint32 ts;
    Sint32 mode;
    Float32 diff;

    if (adxt == 0 || ncount == 0 || tscale == 0) {
        ADXERR_CallErrFunc1("E02080815 adxt_GetTime: parameter error");
        return;
    }
    if (adxt_time_mode == 0) {
        adxt_get_smpl_time(adxt, ncount, tscale);
        return;
    }
    adxt_diff_av = 0.0f;
    if (adxt->stat == ADXT_ISTAT_PLAYING || adxt->stat == ADXT_ISTAT_PLAYEND_WAIT) {
        if (adxt->x72 == 0) {
            *ncount = adxt->x9c + (adxt_vsync_cnt - adxt->startvsync) * 100;
        } else {
            *ncount = adxt->x9c;
        }
        adxt_get_smpl_time(adxt, &nc, &ts);
        diff = 1000.0f * ((Float32)nc / (Float32)ts - (Float32)*ncount / (Float32)adxt_time_unit);
        adxt_diff_av = diff;
        if (diff > 60.0f || diff < -60.0f) {
            if (adxt_time_adjust_sw == 1) {
                mode = adxt_time_mode;
                adxt_time_mode = 0;
                adxt_GetTime(adxt, &nc, &ts);
                adxt_time_mode = mode;
                adxt_time_adjust_cnt++;
            }
            ADXT_SET_TIME_POS(adxt, nc, ts);
            adxt->startvsync = adxt_vsync_cnt;
        }
    } else if (adxt->stat == ADXT_ISTAT_PLAYEND) {
        nc = ADXSJD_GetTotalNumSmpl(adxt->sjd);
        ts = ADXSJD_GetSfreq(adxt->sjd);
        nc = nc * (16 / ADXSJD_GetOutBps(adxt->sjd));
        *ncount = (Sint32)((Float32)adxt_time_unit * ((Float32)nc / (Float32)ts));
        *ncount += adxt->x9c + 1;
    } else {
        *ncount = 0;
    }
    *ncount = *ncount + adxt->timeofst;
    *tscale = adxt_time_unit;
}

Sint32 ADXT_GetTimeReal(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetTimeReal(adxt);
    ADXCRS_Leave();
    return ret;
}

Float32 ADXT_GetMvTime(ADXT adxt) {
    Float32 ret;

    ADXCRS_Enter();
    ret = adxt_GetMvTime(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetNumSmpl(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetNumSmpl(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetHdrLen(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetHdrLen(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetFmtBps(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetFmtBps(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetSfreq(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetSfreq(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetNumChan(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetNumChan(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetOutPan(ADXT adxt, Sint32 ch, Sint32 pan) {
    ADXCRS_Enter();
    adxt_SetOutPan(adxt, ch, pan);
    ADXCRS_Leave();
}

Sint32 ADXT_GetOutPan(ADXT adxt, Sint32 ch) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetOutPan(adxt, ch);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetOutVol(ADXT adxt, Sint32 vol) {
    ADXCRS_Enter();
    adxt_SetOutVol(adxt, vol);
    ADXCRS_Leave();
}

Sint32 ADXT_GetOutVol(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetOutVol(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetDefSvrFreq(Sint32 freq) {
    ADXCRS_Enter();
    adxt_SetDefSvrFreq(freq);
    ADXCRS_Leave();
}

void ADXT_SetSvrFreq(ADXT adxt, Sint32 freq) {
    ADXCRS_Enter();
    adxt_SetSvrFreq(adxt, freq);
    ADXCRS_Leave();
}

void ADXT_SetReloadTime(ADXT adxt, Float32 time) {
    ADXCRS_Enter();
    adxt_SetReloadTime(adxt, time);
    ADXCRS_Leave();
}

void ADXT_ResetReloadTime(ADXT adxt) {
    ADXCRS_Enter();
    adxt_ResetReloadTime(adxt);
    ADXCRS_Leave();
}

void ADXT_SetReloadSct(ADXT adxt, Sint32 nsct) {
    ADXCRS_Enter();
    adxt_SetReloadSct(adxt, nsct);
    ADXCRS_Leave();
}

Sint32 ADXT_GetNumSctIbuf(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetNumSctIbuf(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetNumSmplObuf(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetNumSmplObuf(adxt);
    ADXCRS_Leave();
    return ret;
}

Float32 ADXT_GetIbufRemainTime(ADXT adxt) {
    Float32 ret;

    ADXCRS_Enter();
    ret = adxt_GetIbufRemainTime(adxt);
    ADXCRS_Leave();
    return ret;
}

/* (public entry unreferenced in this game) */
Bool ADXT_IsIbufSafety(ADXT adxt) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080836 ADXT_IsIbufSafety: parameter error");
        return 0;
    }
    if (ADXSTM_GetNumSctIbuf(adxt->stm) >= adxt->reload_nsct) {
        return 1;
    }
    return 0;
}

Bool ADXT_IsCompleted(ADXT adxt) {
    Bool ret;

    ADXCRS_Enter();
    ret = adxt_IsCompleted(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetAutoRcvr(ADXT adxt, Sint32 sw) {
    ADXCRS_Enter();
    adxt_SetAutoRcvr(adxt, sw);
    ADXCRS_Leave();
}

void ADXT_ExecServer(void) {
    ADXCRS_Enter();
    adxt_ExecServer();
    ADXCRS_Leave();
}

Sint32 ADXT_GetErrCode(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetErrCode(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_ClearErrCode(ADXT adxt) {
    ADXCRS_Enter();
    adxt_ClearErrCode(adxt);
    ADXCRS_Leave();
}

Sint32 ADXT_GetLpCnt(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetLpCnt(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetLpFlg(ADXT adxt, Sint32 flg) {
    ADXCRS_Enter();
    adxt_SetLpFlg(adxt, flg);
    ADXCRS_Leave();
}

SJ ADXT_GetInputSj(ADXT adxt) {
    SJ ret;

    ADXCRS_Enter();
    ret = adxt_GetInputSj(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetWaitPlayStart(ADXT adxt, Sint32 sw) {
    ADXCRS_Enter();
    adxt_SetWaitPlayStart(adxt, sw);
    ADXCRS_Leave();
}

Bool ADXT_IsReadyPlayStart(ADXT adxt) {
    Bool ret;

    ADXCRS_Enter();
    ret = adxt_IsReadyPlayStart(adxt);
    ADXCRS_Leave();
    return ret;
}

void ADXT_Pause(ADXT adxt, Sint32 sw) {
    ADXCRS_Enter();
    adxt_Pause(adxt, sw);
    ADXCRS_Leave();
}

Sint32 ADXT_GetStatPause(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetStatPause(adxt);
    ADXCRS_Leave();
    return ret;
}

/* Pitch transpose is not supported by the AX renderer: no-op kept for the API. */
void ADXT_SetTranspose(ADXT adxt, Sint32 oct, Sint32 cent) {}

/* No-op (see ADXT_SetTranspose). */
void ADXT_GetTranspose(ADXT adxt, Sint32 *oct, Sint32 *cent) {}

void ADXT_TermSupply(ADXT adxt) {
    ADXCRS_Enter();
    adxt_TermSupply(adxt);
    ADXCRS_Leave();
}

Sint32 ADXT_DiscardSmpl(ADXT adxt, Sint32 nsmpl) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_DiscardSmpl(adxt, nsmpl);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetTimeOfst(ADXT adxt, Sint32 ofst) {
    ADXCRS_Enter();
    adxt_SetTimeOfst(adxt, ofst);
    ADXCRS_Leave();
}

void ADXT_SetLnkSw(ADXT adxt, Sint32 sw) {
    ADXCRS_Enter();
    adxt_SetLnkSw(adxt, sw);
    ADXCRS_Leave();
}

Sint32 ADXT_GetDecNumSmpl(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetDecNumSmpl(adxt);
    ADXCRS_Leave();
    return ret;
}

Sint32 ADXT_GetDecDtLen(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetDecDtLen(adxt);
    ADXCRS_Leave();
    return ret;
}

/* Unreferenced, so the linker strips it. Only its parameter check is known (its message keeps the
 * .rodata order); what it registers is not reproduced. */
void ADXT_SetCbDec(ADXT adxt, void (*func)(void *obj), void *obj) {
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E04041902 ADXT_SetCbDec: parameter error");
        return;
    }
}

/* True if `data` starts with an ADX header (magic 0x8000) whose info decodes; returns its length in
 * *hdrsiz. Used by the Sofdec audio demuxer to find the start of an ADX track. */
Bool ADXT_IsHeader(Uint8 *data, Sint32 size, Sint32 *hdrsiz) {
    Sint16 hdrlen;
    Sint8 fmt;
    Sint8 bps;
    Sint8 blksiz;
    Sint8 nch;
    Sint32 sfreq;
    Sint32 nsmpl;
    Sint32 blksmpl;

    if (size < 2) {
        return 0;
    }
    if (*(Uint16 *)data != 0x8000) {
        return 0;
    }
    if (ADX_DecodeInfo(data, size, &hdrlen, &fmt, &bps, &blksiz, &nch, &sfreq, &nsmpl, &blksmpl) <
        0) {
        return 0;
    }
    *hdrsiz = hdrlen;
    return 1;
}

/* True if `data` starts with an ADX end code (magic 0x8001); *ecsiz gets the remaining size. */
Bool ADXT_IsEndcode(Uint8 *data, Sint32 size, Sint32 *ecsiz) {
    if (size < 2) {
        return 0;
    }
    if (*(Uint16 *)data != 0x8001) {
        return 0;
    }
    *ecsiz = size;
    return 1;
}

Sint32 ADXT_InsertSilence(ADXT adxt, Sint32 nch, Sint32 nsmpl) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_InsertSilence(adxt, nch, nsmpl);
    ADXCRS_Leave();
    return ret;
}

void ADXT_SetOutBalance(ADXT adxt, Sint32 bal) {
    ADXCRS_Enter();
    adxt_SetOutBalance(adxt, bal);
    ADXCRS_Leave();
}

Sint32 ADXT_GetOutBalance(ADXT adxt) {
    Sint32 ret;

    ADXCRS_Enter();
    ret = adxt_GetOutBalance(adxt);
    ADXCRS_Leave();
    return ret;
}

/* mono output switch: every live handle's pan and balance are re-applied */
void ADXT_SetOutputMono(Sint32 flag) {
    Sint32 i;
    Sint32 ch;
    ADXT adxt;

    adxt_output_mono_flag = flag;
    for (i = 0; i < ADXT_MAX_OBJ; i++) {
        adxt = &adxt_obj[i];
        if (adxt->used == 1) {
            for (ch = 0; ch < adxt->maxnch; ch++) {
                adxt_SetOutPan(adxt, ch, adxt_GetOutPan(adxt, ch));
            }
            adxt_SetOutBalance(adxt, adxt_GetOutBalance(adxt));
        }
    }
}

void ADXT_SetKeyString(ADXT adxt, Char8 *str) {
    ADXCRS_Enter();
    adxt_SetKeyString(adxt, str);
    ADXCRS_Leave();
}
