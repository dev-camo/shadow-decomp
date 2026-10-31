/* CRI ADXT server (adx_tsvr.c, ADXT Ver.9.44): the per-handle state machine run from
 * ADXT_ExecServer, the decoder-information stage and the decoder trap and stream end-of-sector
 * callbacks (looping and linked files).
 *
 * Source from the RE4 GameCube decompilation, with this version's Ver.9.44 addition: the handle's
 * extra field xc0 (see adx_t.h) is compared with the decoded data length in the server step and in
 * the end-of-sector callback.
 *
 * Not yet matching: adxt_ExecHndl differs in 22 words and adxt_nlp_trap_entry in 2, all register
 * numbers. */
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_b.h"
#include "CRI/adx_dcd.h"
#include "CRI/adx_t.h"

void (*adxt_enddecinfo_cbfn)(ADXT adxt, Sint32 sfreq, Sint32 nch, Sint32 nsmpl) = NULL;
static Sint32 adxt_dbg_rna_ndata = 0;
Sint32 adxt_dbg_ndt = 0;
Sint32 adxt_dbg_nch = 0;

static void adxt_stat_decinfo(ADXT adxt);
void adxt_nlp_trap_entry(void *obj);
void adxt_eos_entry(void *obj);
void adxt_trap_entry(void *obj);
void adxt_trap_entry_lps(void *obj);

/* Per-handle server step (every ADXT_ExecServer): PLAYING -> PLAYEND_WAIT when the decoder has
 * ended and less than 64 bytes are left in every output ring; DECINFO parses the header
 * (adxt_stat_decinfo); PREP starts the renderer once enough PCM is buffered (or the stream is
 * already over, padding the rings with silence); PLAYEND_WAIT -> PLAYEND when the renderer has
 * drained. Then terminates the decoder's supply at the stream end (mode 0/1 file, 2 memory) and
 * turns stream and scheduler errors into ADXT_ISTAT_ERROR with errcode -1. */
void adxt_ExecHndl(ADXT adxt) {
    Sint32 nch;
    Sint32 i;
    Sint32 j;
    SJ sj;
    Sint32 nbyte;
    ADXSJD sjd;
    AXRNA rna;
    Sint32 ndata;
    Sint32 nroom;
    SJCK ck;
    Sint32 lim;
    Sint32 n2;

    if (adxt == NULL) {
        ADXERR_CallErrFunc1("E02080842 adxt_ExecHndl: parameter error");
        return;
    }
    if (adxt->stat == ADXT_ISTAT_PLAYING) {
        if (adxt->lpsw == 0 && ADXSJD_GetDecDtLen(adxt->sjd) >= adxt->xc0) {
            ADXSJD_SetTrapNumSmpl(adxt->sjd, -1);
        }
        if (ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_END) {
            nch = ADXSJD_GetNumChan(adxt->sjd);
            adxt_dbg_nch = nch;
            for (i = 0; i < nch; i++) {
                adxt_dbg_ndt = SJ_GetNumData(adxt->sjo[i], SJ_CK_DATA);
                if (adxt_dbg_ndt >= 0x40) {
                    break;
                }
            }
            if (i == nch) {
                ADXRNA_SetTransSw(adxt->rna, 0);
                adxt->stat = ADXT_ISTAT_PLAYEND_WAIT;
            }
        }
    } else if (adxt->stat == ADXT_ISTAT_DECINFO) {
        adxt_stat_decinfo(adxt);
    } else if (adxt->stat == ADXT_ISTAT_PREP) {
        rna = adxt->rna;
        sjd = adxt->sjd;
        ndata = ADXRNA_GetNumData(rna);
        nroom = ADXRNA_GetNumRoom(rna);
        n2 = adxt->maxdecsmpl * 2;
        if (n2 < 0x4000) {
            lim = n2;
        } else {
            lim = 0x4000;
        }
        if (ndata >= lim || nroom <= ADXSJD_GetBlkSmpl(sjd) ||
            ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_END) {
            if (adxt->pausesw == 0) {
                if (adxt->x72 == 0) {
                    ADXRNA_SetPlaySw(rna, 1);
                    adxt->x9c = 0;
                    adxt->startvsync = adxt_vsync_cnt;
                }
                adxt->stat = ADXT_ISTAT_PLAYING;
            }
            adxt->x71 = 1;
        }
        if (ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_END) {
            nch = ADXT_GetNumChan(adxt);
            nbyte = adxt->maxdecsmpl * nch * 2;
            for (j = 0; j < nch; j++) {
                sj = adxt->sjo[j];
                SJ_GetChunk(sj, SJ_CK_FREE, nbyte, &ck);
                memset(ck.data, 0, ck.len);
                SJ_PutChunk(sj, SJ_CK_DATA, &ck);
            }
        }
    } else if (adxt->stat == ADXT_ISTAT_PLAYEND_WAIT) {
        adxt_dbg_rna_ndata = ADXRNA_GetNumData(adxt->rna);
        if (ADXRNA_GetNumData(adxt->rna) <= 0) {
            ADXRNA_SetPlaySw(adxt->rna, 0);
            adxt->stat = ADXT_ISTAT_PLAYEND;
        }
    }

    if (adxt->stm != NULL && ADXT_GetStat(adxt) != ADXT_ISTAT_STOP) {
        switch (adxt->mode) {
        case 0:
        case 1:
            if (ADXSTM_GetStat(adxt->stm) == ADXSTM_STAT_END) {
                ADXSJD_TermSupply(adxt->sjd);
            }
            break;
        case 2:
            ADXSJD_TermSupply(adxt->sjd);
            break;
        case 3:
            break;
        }
    }
    if (adxt->stm != NULL && ADXSTM_GetStat(adxt->stm) == ADXSTM_STAT_ERROR) {
        adxt->errcode = -1;
        adxt->stat = ADXT_ISTAT_ERROR;
    }
    if (adxt->lsc != NULL && LSC_GetStat(adxt->lsc) == LSC_STAT_ERROR) {
        adxt->errcode = -1;
        adxt->stat = ADXT_ISTAT_ERROR;
    }
}

void ADXT_ExecHndl(ADXT adxt) {
    ADXCRS_Enter();
    adxt_ExecHndl(adxt);
    ADXCRS_Leave();
}

/* The decoder has read the header: size the decode step, set up looping and end handling and
 * program the renderer. */
static void adxt_stat_decinfo(ADXT adxt) {
    ADXSJD sjd;
    Sint32 sfreq;
    Sint32 nloop;
    Sint32 blk;
    Sint32 nch;
    Sint32 nsmpl;
    Sint32 bps;
    void *info;
    Sint32 lpendofst;
    Sint32 lpendsct;
    Char8 buf[32];
    Sint32 oct;
    Sint32 cent;
    Sint32 st;

    sjd = adxt->sjd;
    oct = 0;
    cent = 0;
    if ((adxt->mode == 0 || adxt->mode == 1) && adxt->stmstart == 1) {
        if (ADXSTM_GetStat(adxt->stm) == ADXSTM_STAT_EXEC) {
            return;
        }
        if (adxt->sjf != NULL) {
            SJ_Reset(adxt->sjf);
        }
        adxt_start_stm(adxt, adxt->stm_fname, adxt->stm_dir, adxt->stm_ofst, adxt->stm_nsct);
        adxt->stmstart = 0;
    }
    st = ADXSJD_GetStat(sjd);
    if (st == ADXSJD_STAT_DECODE) {
        nch = ADXSJD_GetNumChan(sjd);
        if (nch > adxt->maxnch) {
            ADXERR_ItoA2(nch, adxt->maxnch, buf, 16);
            ADXERR_CallErrFunc2("E9081001 adxt_stat_decinfo: can't play this number of channels",
                                buf);
            ADXT_Stop(adxt);
            return;
        }
        sfreq = ADXSJD_GetSfreq(sjd);
        nloop = ADXSJD_GetNumLoop(sjd);
        if (nloop > 0) {
            adxt->maxdecsmpl = sfreq / adxt->svrfreq * 3;
        } else {
            adxt->maxdecsmpl = sfreq / adxt->svrfreq * 3 / 2;
        }
        blk = ADXSJD_GetBlkSmpl(sjd) * 2;
        adxt->maxdecsmpl = blk * ((adxt->maxdecsmpl + blk) / blk);
        ADXSJD_SetMaxDecSmpl(sjd, adxt->maxdecsmpl);
        if (nloop > 0) {
            if (adxt->mode == 2) {
                adxt->lpendmod = 0;
            } else {
                lpendofst = ADXSJD_GetLpEndOfst(sjd);
                adxt->lpendmod = 0x800 - lpendofst % 0x800;
                lpendsct = (lpendofst + 0x7FF) / 0x800;
                adxt->lpendmod = adxt->lpendmod % 0x800;
                adxt->lpendsct = lpendsct;
                ADXSTM_SetEos(adxt->stm, lpendsct);
                ADXSTM_EntryEosFunc(adxt->stm, adxt_eos_entry, adxt);
            }
            ADXSJD_GetLpEndPos(sjd);
            adxt->trapnsmpl = ADXSJD_GetLpStartPos(sjd);
            ADXSJD_SetTrapNumSmpl(sjd, adxt->trapnsmpl);
            ADXSJD_SetTrapDtLen(sjd, 0);
            ADXSJD_SetTrapCnt(sjd, 0);
            ADXSJD_EntryTrapFunc(sjd, adxt_trap_entry_lps, adxt);
        } else {
            if (adxt->stm != NULL) {
                ADXSTM_SetEos(adxt->stm, 0x7FFFFFFF);
            }
            ADXSJD_SetTrapNumSmpl(sjd, ADXSJD_GetTotalNumSmpl(sjd));
            ADXSJD_SetTrapDtLen(sjd, 0);
            ADXSJD_SetTrapCnt(sjd, 0);
            ADXSJD_EntryTrapFunc(sjd, adxt_nlp_trap_entry, adxt);
        }
        sfreq = ADXSJD_GetSfreq(sjd);
        nch = ADXSJD_GetNumChan(sjd);
        nsmpl = ADXSJD_GetTotalNumSmpl(sjd);
        bps = ADXSJD_GetOutBps(sjd);
        ADXRNA_SetBitPerSmpl(adxt->rna, bps);
        ADXRNA_SetSfreq(adxt->rna, sfreq);
        ADXRNA_SetNumChan(adxt->rna, nch);
        ADXRNA_SetTotalNumSmpl(adxt->rna, nsmpl);
        ADXT_SetOutVol(adxt, adxt->outvol);
        ADXT_GetTranspose(adxt, &oct, &cent);
        if (oct != 0 || cent != 0) {
            ADXT_SetTranspose(adxt, oct, cent);
        }
        if (ADXSJD_GetNumChan(adxt->sjd) == 1) {
            ADXT_SetOutPan(adxt, 0, adxt->outpan[0]);
        } else {
            ADXT_SetOutPan(adxt, 0, adxt->outpan[0]);
            ADXT_SetOutPan(adxt, 1, adxt->outpan[1]);
        }
        if (adxt->amp != NULL) {
            ADXAMP_SetSfreq(adxt->amp, sfreq);
        }
        if (ADXSJD_GetFormat(sjd) == ADXB_TYPE_SPSD) {
            info = ADXSJD_GetSpsdInfo(sjd);
            ADXRNA_SetStmHdInfo(adxt->rna, info);
        }
        ADXRNA_SetTransSw(adxt->rna, 1);
        if (adxt_enddecinfo_cbfn != NULL) {
            adxt_enddecinfo_cbfn(adxt, sfreq, nch, nsmpl);
        }
        adxt->stat = ADXT_ISTAT_PREP;
    } else if (st == ADXSJD_STAT_ERROR) {
        adxt->stat = ADXT_ISTAT_ERROR;
    }
}

/* Non-loop end trap: with the link switch on, look for a following ADX file behind the footer
 * (its info code) and restart the decoder on it. */
void adxt_nlp_trap_entry(void *obj) {
    ADXT adxt;
    ADXSJD sjd;
    SJ sji;
    SJCK ck;
    SJCK ck3;
    SJCK ck2;
    SJCK ck4;
    Sint16 ofst;
    Sint16 ofst2;
    Sint32 ofst1;
    Sint32 n1;
    Sint32 n2;
    Sint32 ofst3;

    adxt = obj;
    sjd = adxt->sjd;
    sji = adxt->sji;
    if (adxt->lnksw == 0) {
        return;
    }
    ofst2 = 0;
    SJ_GetChunk(sji, SJ_CK_DATA, 0x7FFFFFFF, &ck);
    SJ_GetChunk(sji, SJ_CK_DATA, 0x7FFFFFFF, &ck2);
    if (ADX_DecodeFooter(ck.data, ck.len, &ofst) != 0) {
        ADXT_SetLnkSw(adxt, 0);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
        return;
    }
    ofst1 = ofst;
    n1 = ADX_ScanInfoCode(ck.data + ofst1, ck.len - ofst1, &ofst);
    if (n1 == 0) {
        n2 = -1;
    } else {
        n2 = ADX_ScanInfoCode(ck2.data, ck2.len, &ofst2);
    }
    ofst1 += ofst;
    /* ofst3: a separate copy of ofst2; using ofst2 directly compiles differently */
    ofst3 = ofst2;
    if (n1 != 0 && n2 != 0) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck);
        ADXT_SetLnkSw(adxt, 0);
        return;
    }
    if (n1 == 0) {
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck2);
        SJ_SplitChunk(&ck, ofst1, &ck, &ck3);
        SJ_PutChunk(sji, SJ_CK_FREE, &ck);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck3);
    } else {
        SJ_PutChunk(sji, SJ_CK_FREE, &ck);
        SJ_SplitChunk(&ck2, ofst3, &ck2, &ck4);
        SJ_PutChunk(sji, SJ_CK_FREE, &ck2);
        SJ_UngetChunk(sji, SJ_CK_DATA, &ck4);
    }
    adxt->decsmpl += ADXSJD_GetDecNumSmpl(sjd);
    ADXSJD_Stop(sjd);
    ADXSJD_Start(sjd);
    ADXSJD_ExecHndl(sjd);
    if (ADXSJD_GetStat(sjd) != ADXSJD_STAT_DECODE) {
        ADXT_SetLnkSw(adxt, 0);
        return;
    }
    ADXSJD_SetMaxDecSmpl(sjd, adxt->maxdecsmpl);
    ADXSJD_SetTrapNumSmpl(sjd, ADXSJD_GetTotalNumSmpl(sjd));
    ADXSJD_SetTrapDtLen(sjd, 0);
    ADXSJD_SetTrapCnt(sjd, 0);
}

/* Stream end-of-sector callback (looping): seek back to the loop start sector. */
void adxt_eos_entry(void *obj) {
    ADXT adxt;
    ADXSTM stm;
    ADXSJD sjd;
    Sint32 ofst;

    adxt = obj;
    stm = adxt->stm;
    sjd = adxt->sjd;
    if (stm == NULL || sjd == NULL) {
        return;
    }
    ofst = ADXSJD_GetLpStartOfst(sjd);
    if (adxt->lpsw == 0) {
        if (ADXSJD_GetDecDtLen(adxt->sjd) >= adxt->xc0) {
            ADXSJD_SetTrapNumSmpl(adxt->sjd, -1);
        }
        ADXSTM_SetEos(adxt->stm, 0x7FFFFFFF);
    } else {
        ADXSTM_Seek(stm, ofst / 0x800);
    }
}

/* Decoder trap at the loop end: rewind the decoder to the loop start. */
void adxt_trap_entry(void *obj) {
    ADXT adxt;
    ADXSJD sjd;
    SJ sji;
    Sint32 lpstart;
    Sint32 lpstartofst;
    Sint32 lpend;
    SJCK ck;

    adxt = obj;
    sjd = adxt->sjd;
    sji = adxt->sji;
    lpstart = ADXSJD_GetLpStartPos(sjd);
    lpstartofst = ADXSJD_GetLpStartOfst(sjd);
    lpend = ADXSJD_GetLpEndPos(sjd);
    if ((adxt->mode == 2 || adxt->mode == 3) && adxt->lpsw == 0) {
        ADXSJD_SetTrapNumSmpl(adxt->sjd, -1);
        return;
    }
    SJ_GetChunk(sji, SJ_CK_DATA, adxt->lpendmod, &ck);
    if (ck.len < adxt->lpendmod) {
        ADXERR_CallErrFunc1("E8101201 adxt_trap_entry: not enough data");
    }
    SJ_PutChunk(sji, SJ_CK_FREE, &ck);
    ADXSJD_SetTrapCnt(sjd, 0);
    ADXSJD_SetTrapNumSmpl(sjd, adxt->trapnsmpl = lpend - lpstart);
    ADXSJD_SetTrapDtLen(sjd, lpstartofst);
    ADXSJD_SetDecPos(sjd, lpstart);
    if (adxt->mode == 2) {
        SJ_Reset(sji);
        SJ_GetChunk(sji, SJ_CK_DATA, lpstartofst, &ck);
        SJ_PutChunk(sji, SJ_CK_FREE, &ck);
    }
    ADXSJD_RestoreSnapshot(sjd);
    adxt->lpcnt++;
}

/* First trap at the loop start: remember the decoder state and arm the loop end trap. */
void adxt_trap_entry_lps(void *obj) {
    ADXT adxt;
    ADXSJD sjd;
    Sint32 lpstart;
    Sint32 lpstartofst;
    Sint32 lpend;

    adxt = obj;
    sjd = adxt->sjd;
    lpstart = ADXSJD_GetLpStartPos(sjd);
    lpstartofst = ADXSJD_GetLpStartOfst(sjd);
    lpend = ADXSJD_GetLpEndPos(sjd);
    ADXSJD_TakeSnapshot(sjd);
    ADXSJD_SetTrapCnt(sjd, 0);
    ADXSJD_SetTrapNumSmpl(sjd, adxt->trapnsmpl = lpend - lpstart);
    ADXSJD_SetTrapDtLen(sjd, lpstartofst);
    ADXSJD_SetDecPos(sjd, lpstart);
    ADXSJD_EntryTrapFunc(sjd, adxt_trap_entry, adxt);
}
