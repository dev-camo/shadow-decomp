/* CRI ADXT seamless (linked-file) playback over the load scheduler (adx_lsc.c, ADXT Ver.9.44):
 * file entry, start, loop switch, entry count and reset, plus the load scheduler server.
 *
 * Function names follow the Street Fighter III 3rd Strike decompilation of the same file. Every
 * public entry runs under ADXCRS_Enter/ADXCRS_Leave. The file is compiled with deferred inlining,
 * so the functions are emitted in reverse order.
 */
#include "CRI/adx.h"
#include "CRI/adx_t.h"
#include "CRI/lsc.h"
#include "CRI/sj.h"

/* Workers behind ADXT_EntryFname and ADXT_StartSeamless. The .rodata order shows separate functions
 * defined after ADXT_SetSeamlessLp: their messages sit between GetNumFiles' and SetSeamlessLp's. In
 * this ADXT version the messages carry the lowercase worker names. With deferred inlining their
 * out-of-line copies are emitted at that point, and the linker strips them.
 */
static void adxt_EntryFnameRange(ADXT adxt, Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);
static void adxt_StartSeamless(ADXT adxt);

/* Enters a whole file into the handle's seamless list. */
void ADXT_EntryFname(ADXT adxt, Char8 *fname) {
    ADXCRS_Enter();
    adxt_EntryFnameRange(adxt, fname, 0, 0, ADXSTM_NSCT_INF);
    ADXCRS_Leave();
}

/* Starts seamless playback. */
void ADXT_StartSeamless(ADXT adxt) {
    ADXCRS_Enter();
    adxt_StartSeamless(adxt);
    ADXCRS_Leave();
}

/* Loops the seamless list. */
void ADXT_SetSeamlessLp(ADXT adxt, Sint32 flg) {
    ADXCRS_Enter();
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080851 adxt_SetSeamlessLp: parameter error");
    } else {
        LSC_SetLpFlg(adxt->lsc, flg);
    }
    ADXCRS_Leave();
}

/* Enters part of a file into the seamless list. The dir, ofst and nsct parameters are inferred from
 * LSC_EntryFileRange, which receives them unchanged; the only caller passes the whole file.
 */
static void adxt_EntryFnameRange(ADXT adxt, Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct) {
    LSC lsc;

    /* original quirk: adxt is dereferenced before the null test below */
    lsc = adxt->lsc;
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080848 adxt_EntryFnameRange: parameter error");
    } else if (LSC_EntryFileRange(lsc, fname, dir, ofst, nsct) < 0) {
        ADXERR_CallErrFunc2("E4063001:Can't entry file ", fname);
    }
}

/* Starts seamless playback: the decoder reads the file stream joint in link mode while the load
 * scheduler streams the entered files (flow limit = the reload threshold in bytes).
 */
static void adxt_StartSeamless(ADXT adxt) {
    LSC lsc;

    /* original quirk: adxt is dereferenced before the null test below */
    lsc = adxt->lsc;
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080850 adxt_StartSeamless: parameter error");
    } else {
        ADXT_StopWithoutLsc(adxt);
        ADXCRS_Lock();
        adxt_start_sj(adxt, adxt->sjf);
        SJ_Reset(adxt->sji);
        adxt->mode = 4;
        LSC_SetFlowLimit(lsc, adxt->reload_nsct * ADXT_SCT_SIZE);
        LSC_Start(lsc);
        ADXT_SetLnkSw(adxt, 1);
        ADXCRS_Unlock();
    }
}

/* Number of files in the seamless list (-1: no handle). */
Sint32 ADXT_GetNumFiles(ADXT adxt) {
    Sint32 n;

    ADXCRS_Enter();
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080854 adxt_GetNumFiles: parameter error");
        n = -1;
    } else {
        n = LSC_GetNumStm(adxt->lsc);
    }
    ADXCRS_Leave();
    return n;
}

/* Clears the seamless list of a stopped handle. */
void ADXT_ResetEntry(ADXT adxt) {
    ADXCRS_Enter();
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080849 adxt_ResetEntry: parameter error");
    } else if (adxt->stat == ADXT_ISTAT_STOP) {
        LSC_ResetEntry(adxt->lsc);
    }
    ADXCRS_Leave();
}

/* SVM main callback body: the load scheduler server under the enter/leave hooks (name inferred). */
void ADXT_ExecLscSvr(void) {
    ADXCRS_Enter();
    LSC_ExecServer();
    ADXCRS_Leave();
}
