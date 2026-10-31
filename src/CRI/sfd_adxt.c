/* CRI Sofdec ADX audio driver (sfd_adxt.c, SFD_tr_ad_adxt, driver slot SFADXT_TR): feeds the
 * demuxed ADX stream into an ADXT handle through a ring-buffer stream joint, skipping the silence /
 * header the muxer inserted and aligning the audio start to the video start; the ADXT playback time
 * drives the player clock (sfadxt_GetTime).
 *
 * Names and structure follow the RE4 GameCube decompilation of the same file. The static inline
 * helpers are all expanded into their callers in this executable.
 *
 * Not yet matching: sfadxt_ExcludeHdr (38 of 93 words) and sfadxt_ExecServerSub (62 of 266) differ
 * only in register numbers; in both the original colours a parameter (data, sfd) before the other
 * locals, and no natural declaration order reproduces that. */
#include <math.h>
#include <string.h>

#include "CRI/adx_t.h"
#include "CRI/cri_xpt.h"
#include "CRI/sfd.h"
#include "CRI/sfd_tst.h"
#include "CRI/uty.h"

#define SFADXT_COND SFD_COND_AUDIO_ON
#define SFADXT_FRMSIZ 18  /* bytes per ADX frame and channel */
#define SFADXT_FRMSMPL 32 /* samples per ADX frame and channel */
#define SFADXT_HDRSIZ 0x120
#define SFADXT_COPYMAX 0x19000

#define SFADXT_WK(sfd) ((SFADXT_WORK *)(sfd)->tr[SFADXT_TR].hn)
#define SFADXT_TST(sfd) (&SFD_TIM(sfd)->tst)

Sint32 sfadxt_stat = 0;
Sint32 sfadxt_adxterr = 0;
static SFADXT_PARA sfadxt_para;

Sint32 SFADXT_Init(void);
Sint32 SFADXT_Finish(void);
Sint32 SFADXT_ExecServer(SFD sfd);
Sint32 SFADXT_Create(SFD sfd);
static Sint32 SFADXT_Destroy(SFD sfd);
Sint32 SFADXT_Standby(void);
Sint32 SFADXT_Start(SFD sfd);
Sint32 SFADXT_Stop(SFD sfd);
Sint32 SFADXT_Pause(SFD sfd, Sint32 sw);
static Sint32 SFADXT_GetWrite(SFD sfd);
Sint32 SFADXT_AddWrite(SFD sfd);
Sint32 SFADXT_GetRead(SFD sfd);
Sint32 SFADXT_AddRead(SFD sfd);
Sint32 SFADXT_Seek(SFD sfd);

const SFD_TR_IF SFD_tr_ad_adxt = {
    SFADXT_Init,     SFADXT_Finish,  SFADXT_ExecServer, SFADXT_Create, SFADXT_Destroy,
    SFADXT_Standby,  SFADXT_Start,   SFADXT_Stop,       SFADXT_Pause,  SFADXT_GetWrite,
    SFADXT_AddWrite, SFADXT_GetRead, SFADXT_AddRead,    SFADXT_Seek,
};

const Uint8 sfadxt_silence[SFADXT_FRMSIZ] = {0};

void sfadxt_ExcludeSilence(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte);
void sfadxt_ExcludeHdr(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte);
void sfadxt_AdjustSync(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte);
void sfadxt_CopyData(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte);
Sint32 sfadxt_GetTime(SFD sfd, Sint32 *ncount, Sint32 *tscale);
void SFADXT_SetSpeed(SFD sfd, Sint32 speed);
static Sint32 SFADXT_GetOutVol(SFD sfd);
Sint32 SFADXT_SetOutVol(SFD sfd, Sint32 vol);
Sint32 SFADXT_GetOutPan(SFD sfd, Sint32 ch);
Sint32 SFADXT_SetOutPan(SFD sfd, Sint32 ch, Sint32 pan);

/* audio header analysis slot of the seek work (0 without seek support) */
static inline SFSEE_AHDR *sfadxt_GetAhdr(SFD sfd) {
    SFADXT_WORK *wk;

    wk = SFADXT_WK(sfd);
    if (sfd->see.wk == 0) {
        return 0;
    }
    if (wk->x40 > 0) {
        return 0;
    }
    return &sfd->see.wk->aud;
}

/* Channel count and rate of the selected audio stream from the seek work's header analysis (-1 when
 * there is none). */
static inline Sint32 sfadxt_GetAudioInf(SFD sfd, Sint32 *nch, Sint32 *sfreq) {
    SFSEE_AHDR *ahdr;

    ahdr = sfadxt_GetAhdr(sfd);
    if (ahdr == 0) {
        return -1;
    }
    *nch = ahdr->nch;
    *sfreq = ahdr->sfreq;
    return 0;
}

/* the ADXT has decoded its header (stat is neither STOP nor DECINFO) */
static inline Bool sfadxt_IsDecoded(ADXT adxt) {
    Sint32 stat;

    stat = ADXT_GetStat(adxt);
    if (stat == ADXT_ISTAT_STOP || stat == ADXT_ISTAT_DECINFO) {
        return FALSE;
    }
    return TRUE;
}

/* The ADXT is in its PLAYING state. */
static inline Bool sfadxt_IsPlaying(ADXT adxt) {
    if (ADXT_GetStat(adxt) == ADXT_ISTAT_PLAYING) {
        return TRUE;
    }
    return FALSE;
}

/* Resumes the ADXT and the time stabiliser unless the driver's own pause is set. */
static inline void sfadxt_PauseOff(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;
    SFTST tst;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    tst = SFADXT_TST(sfd);
    if (wk->pause != 1) {
        ADXT_Pause(adxt, 0);
        SFTST_Pause(tst, 0);
    }
}

/* Pauses the ADXT and the time stabiliser (the driver keeps audio paused until Start). */
static inline void sfadxt_PauseOn(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    wk->pause = 1;
    ADXT_Pause(adxt, 1);
    SFTST_Pause(SFADXT_TST(sfd), 1);
}

/* Seek support: inserts a synthetic SFA header so the ADXT can restart mid-stream, then skips the
 * leading silence; needs the audio header analysis. Unused here (no seek work). */
Sint32 SFADXT_Seek(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;
    SFSEE_AHDR *ahdr;

    wk = SFADXT_WK(sfd);
    ahdr = sfadxt_GetAhdr(sfd);
    if (ahdr == 0) {
        return 0;
    }
    adxt = wk->adxt;
    if (wk->seekflg != 0) {
        return 0;
    }
    if (ahdr->analyzed == 0) {
        return 0;
    }
    ADXT_InsertHdrSfa(adxt, ahdr->nch, ahdr->sfreq, ahdr->nsmpl);
    ADXT_SetTimeOfst(adxt, 0);
    wk->seekflg = 1;
    wk->func = sfadxt_ExcludeSilence;
    return 0;
}

/* Not supported: error 0xFF000C03. */
Sint32 SFADXT_AddRead(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000C03);
}

/* Not supported: error 0xFF000C03. */
Sint32 SFADXT_GetRead(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000C03);
}

/* Not supported: error 0xFF000C03. */
Sint32 SFADXT_AddWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000C03);
}

/* Not supported: error 0xFF000C03. */
static Sint32 SFADXT_GetWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000C03);
}

Sint32 SFADXT_Pause(SFD sfd, Sint32 sw) {
    SFADXT_WORK *wk;
    ADXT adxt;
    Sint32 sfreq;
    Sint32 nsmpl;
    Sint32 total;
    Sint32 tscale;
    Sint32 ncount;
    SFTST_TIME frm;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    switch (sw) {
    case 0:
        wk->discard = 0;
        sfadxt_PauseOff(sfd);
        break;
    case 1:
        ADXT_Pause(adxt, 1);
        SFTST_Pause(SFADXT_TST(sfd), 1);
        break;
    case 2:
        if (sfadxt_IsDecoded(adxt)) {
            sfreq = ADXT_GetSfreq(adxt);
            SFTIM_GetTimeOneFrmVideo(sfd, &ncount, &tscale);
            nsmpl = UTY_MulDiv(sfreq, ncount, tscale);
            total = nsmpl + wk->discard;
            wk->discard = total - ADXT_DiscardSmpl(adxt, total);
            frm.cnt = nsmpl;
            frm.unit = sfreq;
            SFTST_GoNextFrame(SFADXT_TST(sfd), &frm);
        }
        break;
    }
    return 0;
}

/* Stops the ADXT. */
Sint32 SFADXT_Stop(SFD sfd) {
    ADXT_Stop(SFADXT_WK(sfd)->adxt);
    return 0;
}

/* Driver Start (from the auto-play output driver when the player goes PLAYING): clears the driver
 * pause and resumes the ADXT/stabiliser unless the user pause is on. */
Sint32 SFADXT_Start(SFD sfd) {
    ADXT adxt;
    SFTST tst;
    SFADXT_WORK *wk;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    tst = SFADXT_TST(sfd);
    wk->pause = 0;
    if (sfd->pause_sw != 1) {
        ADXT_Pause(adxt, 0);
        SFTST_Pause(tst, 0);
    }
    return 0;
}

/* Nothing to do. */
Sint32 SFADXT_Standby(void) {
    return 0;
}

/* the ADXT survives a player reset (SFPLY_GetResetFlg): it is only stopped and kept in the library
 * work */
static inline Sint32 sfadxt_ReleaseAdxt(SFD sfd, ADXT adxt) {
    if (SFPLY_GetResetFlg() != 1) {
        ADXT_Destroy(adxt);
        return 0;
    }
    /* Matching workaround: the work's adxt is read again for ADXT_Stop. Passing the parameter
     * `adxt` changes SFADXT_Destroy by 51 words (220 bytes instead of 228). Revisit when the
     * original source of the reset path is known. */
    ADXT_Stop(SFADXT_WK(sfd)->adxt);
    SFLIB_libwork.x200 = adxt;
    return 0;
}

/* Releases the ring joint and the ADXT (kept across a player reset), clears the work. */
static Sint32 SFADXT_Destroy(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;
    SJ sj;
    Sint32 ret;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    sj = wk->sj;
    if (adxt == 0) {
        return 0;
    }
    sfadxt_para = wk->para;
    ret = sfadxt_ReleaseAdxt(sfd, adxt);
    SJ_Destroy(sj);
    UTY_FinishTmr();
    return ret;
}

/* stabilised ADXT playback time (never goes backwards) */
static inline void sfadxt_UpdateTime(SFADXT_WORK *wk, ADXT adxt, SFTST tst) {
    Sint32 acount;
    Sint32 ascale;
    SFTST_TIME mt;
    SFTST_TIME hlp;
    SFTST_TIME out;
    Sint32 cnt;
    Sint32 unit;

    sfadxt_stat = ADXT_GetStat(adxt);
    ADXT_GetTime(adxt, &acount, &ascale);
    mt.cnt = acount;
    mt.unit = ascale;
    hlp.cnt = UTY_GetTmr();
    hlp.unit = UTY_GetTmrUnit();
    SFTST_Calc(tst, &mt, &hlp, &out);
    cnt = (Sint32)out.cnt;
    unit = (Sint32)out.unit;
    if (wk->tcount < cnt) {
        wk->tcount = cnt;
        wk->tunit = unit;
    }
}

/* Clock source type 2 (audio): the ADXT playback time (ADXT_GetTime, samples over the rate)
 * smoothed by the time stabiliser, monotonic (tcount only moves forward). The movie video is timed
 * against this. */
Sint32 sfadxt_GetTime(SFD sfd, Sint32 *ncount, Sint32 *tscale) {
    SFADXT_WORK *wk;
    ADXT adxt;
    SFTST tst;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    tst = SFADXT_TST(sfd);
    if (SFTIM_ChkRegularTime(sfd, ncount, tscale) == 0) {
        return 0;
    }
    if (sfd->stat == SFD_STAT_PLAYING) {
        sfadxt_UpdateTime(wk, adxt, tst);
    }
    *ncount = wk->tcount;
    *tscale = wk->tunit;
    return 0;
}

/* Driver work defaults from the library parameters (ring buffer, ADXT work): transfer state
 * sfadxt_CopyData, no time yet; creates the time stabiliser with the tolerance / excess / adjust
 * conditions (0x3D..0x42, 0x48). */
Sint32 sfadxt_InitInf(SFD sfd, SFADXT_WORK *wk) {
    SFTST tst;
    SFTST_TIME tolerance;
    SFTST_TIME excesserr;
    SFTST_TIME adjstart;
    SFTST_TIME adjpoff;

    if (sfadxt_para.buf == 0 || sfadxt_para.work == 0) {
        return SFLIB_SetErr(0, 0xFF000C06);
    }
    wk->para = sfadxt_para;
    wk->adxt = 0;
    wk->sj = 0;
    wk->tcount = -1;
    wk->tunit = 1;
    wk->pause = 0;
    wk->discard = 0;
    wk->seekflg = 0;
    wk->smplofst = 0;
    wk->func = sfadxt_CopyData;
    wk->x40 = 0;
    wk->svrfreq = -1;
    wk->totbyte = 0;
    wk->x4c = 0;
    tst = SFADXT_TST(sfd);
    tolerance.cnt = SFSET_GetCond(sfd, 0x3F);
    tolerance.unit = 1000000;
    excesserr.cnt = SFSET_GetCond(sfd, 0x40);
    excesserr.unit = 1000000;
    adjstart.cnt = SFSET_GetCond(sfd, 0x41);
    adjstart.unit = 1000000;
    adjpoff.cnt = SFSET_GetCond(sfd, 0x42);
    adjpoff.unit = 1000000;
    SFTST_Create(tst);
    SFTST_SetTstFlg(tst, SFSET_GetCond(sfd, 0x48));
    SFTST_SetTolerance(tst, &tolerance);
    SFTST_SetExcessErr(tst, &excesserr);
    SFTST_SetAdjStart(tst, &adjstart);
    SFTST_SetAdjPoff(tst, &adjpoff);
    SFTST_SetMovaveRange(tst, SFSET_GetCond(sfd, 0x3E));
    UTY_InitTmr(SFSET_GetCond(sfd, 0x3D));
    return 0;
}

/* Creates the ADXT handle (channel count and work from the driver parameters) or reuses the one
 * kept across a reset; auto recovery off, DSP rate correction on. */
static inline ADXT sfadxt_CreateAdxt(SFADXT_WORK *wk) {
    ADXT adxt;

    if (SFPLY_GetResetFlg() != 1) {
        adxt = ADXT_Create(wk->para.maxnch, wk->para.work, wk->para.worksize);
    } else {
        adxt = SFLIB_libwork.x200;
    }
    if (adxt == 0) {
        return 0;
    }
    ADXT_SetAutoRcvr(adxt, 0);
    ADXGC_SetAdjsfreqFlg(adxt, 1);
    return adxt;
}

/* Matching workaround: kept as a separate inline function. Written in SFADXT_Create itself, that
 * function differs by 20 words (same size). Revisit when the original split of the creation code
 * is known. */
static inline Sint32 sfadxt_CreateSub(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;
    SJ sj;
    Sint32 ret;
    SFAOAP *aoap;

    wk = &sfd->adxt;
    sfd->tr[SFADXT_TR].hn = wk;
    ret = sfadxt_InitInf(sfd, wk);
    if (ret != 0) {
        return ret;
    }
    adxt = sfadxt_CreateAdxt(wk);
    if (adxt == 0) {
        return SFLIB_SetErr(0, 0xFF000C04);
    }
    sj = SJRBF_Create(wk->para.buf, wk->para.bsize, wk->para.xsize);
    if (sj == 0) {
        return SFLIB_SetErr(0, 0xFF000C05);
    }
    wk->adxt = adxt;
    wk->sj = sj;
    aoap = &sfd->aoap;
    sfd->tr[SFAOAP_TR].hn = aoap;
    aoap->x00 = adxt->rna;
    aoap->SetOutPan = SFADXT_SetOutPan;
    aoap->GetOutPan = SFADXT_GetOutPan;
    aoap->SetOutVol = SFADXT_SetOutVol;
    aoap->GetOutVol = SFADXT_GetOutVol;
    aoap->SetSpeed = SFADXT_SetSpeed;
    ADXT_StartSj(adxt, sj);
    sfadxt_PauseOn(sfd);
    SFTIM_SetTimeFn(sfd, sfadxt_GetTime, 2);
    SFSET_SetCond(sfd, 0xF, 2);
    return 0;
}

/* Driver Create when audio is on (SFD_COND_AUDIO_ON): work, ADXT and ring joint, fills the
 * auto-play output driver's callback block (volume/pan/speed), starts the ADXT on the joint paused,
 * and installs the audio clock as source 2 (cond 0xF = 2). */
Sint32 SFADXT_Create(SFD sfd) {
    if (SFSET_GetCond(sfd, SFADXT_COND) == 0) {
        return 0;
    }
    return sfadxt_CreateSub(sfd);
}

/* transfer state 1: skip the silence frames the muxer put before the audio (old mux versions) */
void sfadxt_ExcludeSilence(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte) {
    SFADXT_WORK *wk;
    Sint32 ofst;
    Sint32 nch;
    Sint32 sfreq;
    Uint8 *p;

    *nbyte = 0;
    wk = SFADXT_WK(sfd);
    if (SFHDS_GetMuxVerNum(sfd) >= 108) {
        wk->func = sfadxt_ExcludeHdr;
        return;
    }
    p = data;
    for (ofst = 0; ofst < len - SFADXT_FRMSIZ; ofst += SFADXT_FRMSIZ) {
        if (memcmp(p, sfadxt_silence, SFADXT_FRMSIZ) != 0) {
            wk->func = sfadxt_ExcludeHdr;
            break;
        }
        p += SFADXT_FRMSIZ;
    }
    *nbyte = ofst;
    sfd->plyinf.a_byte += ofst;
    if (sfadxt_GetAudioInf(sfd, &nch, &sfreq) != 0) {
        return;
    }
    wk->smplofst += ofst / (nch * SFADXT_FRMSIZ) * SFADXT_FRMSMPL;
}

/* frame alignment of header-less audio (old mux versions): try every 2-byte offset up to two
 * frames; the first offset whose frames never start with a high-bit byte is the alignment,
 * otherwise the offset that found the furthest end code, or offset 0 with error 0xFF000C0A when no
 * end code was found */
static inline Sint32 sfadxt_SearchFrmTop(SFD sfd, Uint8 *data, Sint32 len) {
    Uint8 *p;
    Uint8 *q;
    Uint8 *end;
    Uint8 *foundp;
    Uint8 *found;
    Sint32 hit;
    Uint8 *lim;
    Sint32 ecsiz;

    hit = 0;
    p = data;
    found = 0;
    end = data + len;
    foundp = 0;
    lim = data + SFADXT_FRMSIZ * 2;
    for (; p < lim; p += 2) {
        hit = 0;
        for (q = p; q < end; q += SFADXT_FRMSIZ) {
            if ((Sint8)*q < 0) {
                hit = 1;
                if (ADXT_IsEndcode(q, SFADXT_FRMSIZ, &ecsiz) && found < q) {
                    found = q;
                    foundp = p;
                }
                break;
            }
        }
        if (hit == 0) {
            break;
        }
    }
    if (hit != 0) {
        if (foundp == 0) {
            SFLIB_SetErr(sfd, 0xFF000C0A);
            p = data;
        } else {
            p = foundp;
        }
    }
    return p - data;
}

/* transfer state 2: skip the ADX header; without a header (old mux versions) find the frame
 * alignment from the end codes. */
void sfadxt_ExcludeHdr(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte) {
    SFADXT_WORK *wk;
    Sint32 skip;
    Sint32 hdrsiz;

    *nbyte = 0;
    wk = SFADXT_WK(sfd);
    if (len < SFADXT_HDRSIZ) {
        return;
    }
    if (ADXT_IsHeader(data, len, &hdrsiz)) {
        skip = hdrsiz;
    } else if (SFHDS_GetMuxVerNum(sfd) >= 108) {
        skip = 0;
    } else {
        skip = sfadxt_SearchFrmTop(sfd, data, len);
    }
    wk->func = sfadxt_AdjustSync;
    *nbyte = skip;
    sfd->plyinf.a_byte += skip;
}

/* bytes up to the first end code within lim */
static inline Sint32 sfadxt_SearchEndcode(Uint8 *data, Sint32 lim, Sint32 *endflg) {
    Uint8 *p;
    Sint32 ofs;
    Sint32 ecsiz;

    p = data;
    *endflg = 0;
    for (ofs = 0; ofs < lim; ofs += SFADXT_FRMSIZ) {
        if (ADXT_IsEndcode(p, SFADXT_FRMSIZ, &ecsiz)) {
            *endflg = 1;
            break;
        }
        p += SFADXT_FRMSIZ;
    }
    return ofs;
}

/* transfer state 3: align the audio start to the video start (skip frames or insert silence). */
void sfadxt_AdjustSync(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte) {
    Sint32 lim;
    Sint32 frmbyte;
    Sint32 skip;
    Sint32 skipbyte;
    Sint32 nch;
    Sint32 endflg;
    SFTIM tim;
    Sint32 vstart;
    Sint32 astart;
    Sint32 sfreq;
    Sint32 ofs;
    Sint32 n;
    Sint32 ecsiz;
    Sint32 vflg;
    Sint32 diff;
    SFADXT_WORK *wk;
    Sint32 ins;
    Sint32 cnt;

    *nbyte = 0;
    tim = SFD_TIM(sfd);
    wk = SFADXT_WK(sfd);
    if (sfadxt_GetAudioInf(sfd, &nch, &sfreq) != 0) {
        wk->func = sfadxt_CopyData;
        return;
    }
    astart = SFTIM_GetAudioStartSample(tim, sfreq);
    if (astart < 0) {
        return;
    }
    if (SFSET_GetCond(sfd, SFD_COND_VIDEO_ON) == 0) {
        SFTIM_SetStartTime(tim, astart, sfreq);
        wk->func = sfadxt_CopyData;
        return;
    }
    vstart = SFTIM_GetVideoStartSample(tim, sfreq, &vflg);
    if (vstart < 0) {
        return;
    }
    SFTIM_SetStartTime(tim, vstart, sfreq);
    skip = 0;
    diff = (vstart - astart) - wk->smplofst;
    if (diff >= 0) {
        endflg = 0;
        skipbyte = diff / SFADXT_FRMSMPL * nch * SFADXT_FRMSIZ;
        if (skipbyte > 0) {
            frmbyte = nch * SFADXT_FRMSIZ;
            lim = skipbyte;
            /* Matching workaround: `len / frmbyte * nch * SFADXT_FRMSIZ` stands for the frame
             * count times frmbyte; the shorter `len / frmbyte * frmbyte` changes sfadxt_AdjustSync
             * by 88 words (636 bytes instead of 640). Revisit when the original expression is
             * known. */
            n = len / frmbyte * nch * SFADXT_FRMSIZ;
            if (n < skipbyte) {
                lim = n;
            }
            ofs = sfadxt_SearchEndcode(data, lim, &endflg);
            wk->smplofst += ofs / frmbyte * SFADXT_FRMSMPL;
            skip = ofs;
            skipbyte -= lim;
        }
        if (skipbyte <= 0 && vflg != 0) {
            wk->func = sfadxt_CopyData;
            endflg = ADXT_IsEndcode(data, len, &ecsiz);
        }
        if (endflg != 0) {
            SFSET_SetCond(sfd, SFADXT_COND, 0);
        }
    } else {
        if (vflg != 0) {
            cnt = (-diff) / SFADXT_FRMSMPL * SFADXT_FRMSMPL;
            if (cnt > 0) {
                /* Matching workaround: the ADXT is read through SFADXT_WK(sfd); `wk->adxt` changes
                 * sfadxt_AdjustSync by 32 words. Revisit when the original access is known. */
                ins = ADXT_InsertSilence(SFADXT_WK(sfd)->adxt, nch, cnt);
                cnt -= ins;
                wk->smplofst -= ins;
            }
            if (cnt <= 0) {
                wk->func = sfadxt_CopyData;
            }
        }
    }
    *nbyte = skip;
    sfd->plyinf.a_byte += skip;
}

/* transfer state 4: copy the audio data into the ADXT stream joint */
void sfadxt_CopyData(SFD sfd, Uint8 *data, Sint32 len, Sint32 *nbyte) {
    SFADXT_WORK *wk;
    SJ sj;
    Sint32 n;
    Uint8 *dst;
    SJCK ck;
    SJCK ck2;

    wk = SFADXT_WK(sfd);
    sj = wk->sj;
    SJ_GetChunk(sj, SJ_CK_FREE, wk->para.bsize, &ck);
    dst = ck.data;
    n = (len < ck.len) ? len : ck.len;
    n = (n < SFADXT_COPYMAX) ? n : SFADXT_COPYMAX;
    MEM_Copy(dst, data, n);
    if (n == 0) {
        SJ_UngetChunk(sj, SJ_CK_FREE, &ck);
    } else {
        SJ_SplitChunk(&ck, n, &ck, &ck2);
        SJ_PutChunk(sj, SJ_CK_DATA, &ck);
        SJ_UngetChunk(sj, SJ_CK_FREE, &ck2);
    }
    wk->totbyte += n;
    *nbyte = n;
}

/* flow counters of the audio input ring and of the decoder stream joint */
static inline void sfadxt_UpdateFlowCnt(SFD sfd) {
    SFADXT_WORK *wk;
    SJ sj;
    Sint32 wcnt;
    Sint32 rcnt;

    wk = SFADXT_WK(sfd);
    SFBUF_RingGetSj(sfd, sfd->tr[SFADXT_TR].bufin, &sj);
    if (sj == 0) {
        return;
    }
    SFBUF_GetFlowCnt(sj, &wcnt, &rcnt);
    sfd->plyinf.a_in_wcnt = SFBUF_UpdateFlowCnt(sfd->plyinf.a_in_wcnt, wcnt);
    sfd->plyinf.a_in_rcnt = SFBUF_UpdateFlowCnt(sfd->plyinf.a_in_rcnt, rcnt);
    SFBUF_GetFlowCnt(wk->sj, &wcnt, &rcnt);
    sfd->plyinf.a_sj_wcnt = SFBUF_UpdateFlowCnt(sfd->plyinf.a_sj_wcnt, wcnt);
    sfd->plyinf.a_sj_rcnt = SFBUF_UpdateFlowCnt(sfd->plyinf.a_sj_rcnt, rcnt);
}

/* read the demuxed audio ring and run the transfer state function on it */
static inline Sint32 sfadxt_Transfer(SFD sfd, Sint32 *len) {
    SFADXT_WORK *wk;
    Sint32 nbyte;
    SFBUF_RINF inf;
    Sint32 ret;
    Uint8 *data;
    Sint32 n;

    wk = SFADXT_WK(sfd);
    ret = SFBUF_RingGetRead(sfd, sfd->tr[SFADXT_TR].bufin, &inf);
    /* inf.ck1 is read before the error check; reading it after the check differs further from the
     * original */
    data = inf.ck1.data;
    n = inf.ck1.len;
    if (ret != 0) {
        return ret;
    }
    *len = n;
    wk->func(sfd, data, n, &nbyte);
    ret = SFBUF_RingAddRead(sfd, sfd->tr[SFADXT_TR].bufin, nbyte);
    if (ret != 0) {
        return ret;
    }
    sfadxt_UpdateFlowCnt(sfd);
    return ret;
}

/* the total sample count into the control queue once it is empty */
static inline void sfadxt_WriteTotSmpl(SFD sfd) {
    ADXT adxt;
    Sint32 nsmpl;
    Sint32 sfreq;

    adxt = SFADXT_WK(sfd)->adxt;
    if (SFD_TIM(sfd)->que_wr == SFD_TIM(sfd)->que_rd) {
        nsmpl = ADXT_GetNumSmpl(adxt);
        sfreq = ADXT_GetSfreq(adxt);
        if (nsmpl > 0 && sfreq > 0) {
            SFCON_WriteTotSmplQue(sfd, nsmpl, sfreq);
        }
    }
}

/* the output buffer is ready once the input buffer is prepared and the ADXT plays */
static inline void sfadxt_PrepOut(SFD sfd) {
    Sint32 bufin;
    Sint32 bufout;

    bufout = sfd->tr[SFADXT_TR].bufout;
    bufin = sfd->tr[SFADXT_TR].bufin;
    if (SFBUF_GetPrepFlg(sfd, bufout) != 1) {
        if (SFBUF_GetPrepFlg(sfd, bufin) == 1) {
            if (sfadxt_IsPlaying(SFADXT_WK(sfd)->adxt)) {
                SFBUF_SetPrepFlg(sfd, bufout, 1);
            }
        }
    }
}

/* After a transfer: maps ADXT errors to 0xFF000C07..09 (when cond 0x1A reports them), turns the
 * stabiliser adjustment off once playing ends, terminates the output buffer at ADXT play end, and
 * when the input buffer terminated with nothing left tells the ADXT no more data comes. */
static inline void sfadxt_CheckStat(SFD sfd, Sint32 len) {
    SFADXT_WORK *wk;
    ADXT adxt;
    Sint32 adxterr;
    Sint32 stat;
    SFTST tst;

    wk = SFADXT_WK(sfd);
    tst = SFADXT_TST(sfd);
    adxt = wk->adxt;
    stat = ADXT_GetStat(adxt);
    adxterr = ADXT_GetErrCode(adxt);
    if (adxterr != 0) {
        sfadxt_adxterr = adxterr;
    }
    if (SFSET_GetCond(sfd, 0x1A) == 0) {
        adxterr = 0;
    }
    switch (adxterr) {
    case -1:
        SFLIB_SetErr(sfd, 0xFF000C08);
        break;
    case -2:
        SFLIB_SetErr(sfd, 0xFF000C09);
        break;
    case 0:
        break;
    default:
        SFLIB_SetErr(sfd, 0xFF000C07);
        break;
    }
    if (stat == ADXT_ISTAT_PLAYEND_WAIT || stat == ADXT_ISTAT_PLAYEND) {
        SFTST_SetAdjFlg(tst, 0);
    }
    if (stat == ADXT_ISTAT_PLAYEND || adxterr != 0) {
        SFBUF_SetTermFlg(sfd, sfd->tr[SFADXT_TR].bufout, 1);
    }
    if (SFBUF_GetTermFlg(sfd, sfd->tr[SFADXT_TR].bufin) == 1 && len == 0) {
        ADXT_TermSupply(adxt);
        if (wk->totbyte == 0) {
            SFBUF_SetTermFlg(sfd, sfd->tr[SFADXT_TR].bufout, 1);
        }
    }
}

/* the audio header analysis once the decoder has data */
static inline void sfadxt_AnalyAhdr(SFD sfd) {
    SFSEE_AHDR *ahdr;
    ADXT adxt;

    ahdr = sfadxt_GetAhdr(sfd);
    if (ahdr != 0 && ahdr->analyzed == 0) {
        adxt = SFADXT_WK(sfd)->adxt;
        if (sfadxt_IsDecoded(adxt)) {
            ahdr->sfreq = ADXT_GetSfreq(adxt);
            ahdr->nsmpl = ADXT_GetNumSmpl(adxt);
            ahdr->nch = ADXT_GetNumChan(adxt);
            ahdr->ncount = ahdr->sfreq * ahdr->nch * 9 / 16;
            ahdr->tscale = 1;
            ahdr->analyzed = 1;
        }
    }
}

/* the server frequency follows the condition */
static inline void sfadxt_UpdateSvrFreq(SFD sfd) {
    SFADXT_WORK *wk;
    ADXT adxt;
    Sint32 freq;

    wk = SFADXT_WK(sfd);
    adxt = wk->adxt;
    freq = SFSET_GetCond(sfd, 0x1B);
    if (wk->svrfreq != freq) {
        wk->svrfreq = freq;
        ADXT_SetSvrFreq(adxt, freq);
    }
}

/* The server body: the helpers above called in order (not yet matching: see the file comment). */
static Sint32 sfadxt_ExecServerSub(SFD sfd) {
    Sint32 err;
    Sint32 len;

    if (SFSET_GetCond(sfd, SFADXT_COND) == 0) {
        return 0;
    }
    if (SFBUF_GetTermFlg(sfd, sfd->tr[SFADXT_TR].bufout) == 1) {
        return 0;
    }
    len = 0;
    err = sfadxt_Transfer(sfd, &len);
    sfadxt_PrepOut(sfd);
    sfadxt_CheckStat(sfd, len);
    sfadxt_AnalyAhdr(sfd);
    sfadxt_UpdateSvrFreq(sfd);
    sfadxt_WriteTotSmpl(sfd);
    return err;
}

/* Driver server: move demuxed ADX bytes from the audio input ring into the ADXT's joint
 * (sfadxt_Transfer through the state function: copy / skip silence / skip header), prepare the
 * output, check the ADXT state, analyse the audio header, follow the server-rate condition (0x1B)
 * and post the total sample count for concatenation. */
Sint32 SFADXT_ExecServer(SFD sfd) {
    return sfadxt_ExecServerSub(sfd);
}

/* Library finish: SFA and ADXT. */
Sint32 SFADXT_Finish(void) {
    SFA_Finish();
    ADXT_Finish();
    return 0;
}

/* Library init: ADXT and SFA, clears the driver parameters. */
Sint32 SFADXT_Init(void) {
    ADXT_Init();
    SFA_Init();
    UTY_MemsetDword(&sfadxt_para, 0, sizeof(sfadxt_para) / 4);
    return 0;
}

/* Library-wide ADXT driver parameters (ring buffer and ADXT work, 32-byte aligned) for the next
 * handle. */
void SFD_SetAdxtPara(SFADXT_PARA *para) {
    sfadxt_para.bsize = para->bsize;
    sfadxt_para.xsize = para->xsize;
    sfadxt_para.buf = (void *)(((Uint32)para->buf + 31) & ~31);
    sfadxt_para.maxnch = para->maxnch;
    sfadxt_para.x10 = para->x10;
    sfadxt_para.worksize = para->worksize;
    sfadxt_para.work = (void *)(((Uint32)para->work + 31) & ~31);
}

/* Matching workaround: pool_data off. Each of the four literals is addressed with its own lis/lfs
 * (or lis/lfd) pair instead of one pooled base; without the pragma SFADXT_SetSpeed differs by 42
 * words. Revisit when the original unit's compiler settings are known. */
#pragma pool_data off
/* Speed (1000 = normal) as a pitch shift of 1200 * ln(speed/1000) / ln 2 cents, passed to
 * ADXT_SetTranspose as cents / 100 and the remaining cents;
 * ADXT_SetTranspose is a no-op on AX, so speed change has no audio effect here. */
void SFADXT_SetSpeed(SFD sfd, Sint32 speed) {
    ADXT adxt;
    Float32 cent;
    Sint32 icent;
    Sint32 ihund;
    Float32 l;

    adxt = SFADXT_WK(sfd)->adxt;
    if (adxt == 0) {
        return;
    }
    if (speed == 1000) {
        ihund = 0;
        icent = 0;
    } else {
        l = log((Float32)speed);
        cent = 1731.234f * (l - 6.9077554f);
        ihund = (Sint32)(0.01f * cent);
        icent = (Sint32)cent - ihund * 100;
    }
    ADXT_SetTranspose(adxt, ihund, icent);
}
#pragma pool_data on

/* ADXT output volume. */
static Sint32 SFADXT_GetOutVol(SFD sfd) {
    return ADXT_GetOutVol(SFADXT_WK(sfd)->adxt);
}

/* ADXT output volume (1/10 dB).
 * Matching workaround: no return statement. ADXT_SetOutVol is void, and the original returns
 * whatever the call leaves in the result register; `return 0` adds an instruction. Revisit
 * when the original body is known. */
Sint32 SFADXT_SetOutVol(SFD sfd, Sint32 vol) {
    ADXT_SetOutVol(SFADXT_WK(sfd)->adxt, vol);
}

/* ADXT channel pan. */
Sint32 SFADXT_GetOutPan(SFD sfd, Sint32 ch) {
    return ADXT_GetOutPan(SFADXT_WK(sfd)->adxt, ch);
}

/* ADXT channel pan.
 * Matching workaround: no return statement, as in SFADXT_SetOutVol (ADXT_SetOutPan is void). */
Sint32 SFADXT_SetOutPan(SFD sfd, Sint32 ch, Sint32 pan) {
    ADXT_SetOutPan(SFADXT_WK(sfd)->adxt, ch, pan);
}
