/* CRI ADXT seamless (linked-file) playback over the load scheduler (adx_lsc.c, ADXT Ver.9.44):
 * file entry, start, loop switch, entry count and reset, plus the load scheduler server.
 *
 * Functions and error codes from Street Fighter III 3rd Strike's (PS2) adx_lsc.c. Ver.9.44 wraps
 * every public entry in ADXCRS_Enter/ADXCRS_Leave, enters files as a full range, starts the
 * scheduler under the lock and names the internal adxt_* routines in its messages. The file is
 * compiled with deferred inlining, so the functions are emitted in reverse order.
 */
#include "CRI/adx.h"
#include "CRI/adx_t.h"
#include "CRI/lsc.h"
#include "CRI/sj.h"

/* Enters a whole file into the handle's seamless list. */
void ADXT_EntryFname(ADXT adxt, Char8 *fname) {
    LSC lsc;

    ADXCRS_Enter();
    lsc = adxt->lsc;
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080848 adxt_EntryFnameRange: parameter error");
    } else if (LSC_EntryFileRange(lsc, fname, 0, 0, 0xFFFFF) < 0) {
        ADXERR_CallErrFunc2("E4063001:Can't entry file ", fname);
    }
    ADXCRS_Leave();
}

/* Starts seamless playback: the decoder reads the file stream joint in link mode while the load
 * scheduler streams the entered files (flow limit = the reload threshold in bytes).
 */
void ADXT_StartSeamless(ADXT adxt) {
    LSC lsc;

    ADXCRS_Enter();
    lsc = adxt->lsc;
    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080850 adxt_StartSeamless: parameter error");
    } else {
        ADXT_StopWithoutLsc(adxt);
        ADXCRS_Lock();
        adxt_start_sj(adxt, adxt->sjf);
        SJ_Reset(adxt->sji);
        adxt->mode = 4;
        LSC_SetFlowLimit(lsc, adxt->reload_nsct << 11);
        LSC_Start(lsc);
        ADXT_SetLnkSw(adxt, 1);
        ADXCRS_Unlock();
    }
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

/* Unreferenced, so the linker strips it. 3rd Strike has ADXT_StartFnameLp here; this body is
 * inferred: an inlined StartSeamless / EntryFname / SetSeamlessLp sequence without a message of
 * its own is what places those messages after GetNumFiles' in .rodata.
 */
void ADXT_StartFnameLp(ADXT adxt, Char8 *fname) {
    ADXT_StartSeamless(adxt);
    ADXT_EntryFname(adxt, fname);
    ADXT_SetSeamlessLp(adxt, 1);
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

/* SVM main callback body: the load scheduler server under the enter/leave hooks. */
void ADXT_ExecLscSvr(void) {
    ADXCRS_Enter();
    LSC_ExecServer();
    ADXCRS_Leave();
}
