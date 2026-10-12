/* CRI ADXT file starts (adx_tlk2.c, ADXT Ver.9.44): playback of a named file or of a file inside an
 * AFS archive through the handle's stream controller.
 *
 * Functions from Street Fighter III 3rd Strike's (PS2) adx_tlk2.c. Ver.9.44 wraps the public
 * entries in ADXCRS_Enter/ADXCRS_Leave around adxt_* workers and starts a named file as a full
 * range. The file is compiled with deferred inlining, so the functions are emitted in reverse
 * order. */
#include <string.h>

#include "CRI/adx.h"
#include "CRI/adx_f.h"
#include "CRI/adx_t.h"

/* Starts an AFS file: looks up patid/fid, then the next server pass opens the range. */
static inline void adxt_StartAfs(ADXT adxt, Sint32 patid, Sint32 fid) {
    Char8 error[16];
    Sint32 ofst;
    Sint32 fnsct;
    void *dir;

    if (adxt == 0) {
        ADXERR_CallErrFunc1("E02080811 adxt_StartAfs: parameter error");
        return;
    }
    ADXT_Stop(adxt);
    if (ADXF_GetFnameRangeEx(patid, fid, (Char8 *)adxt->wkend, &dir, &ofst, &fnsct) != 0) {
        return;
    }
    if (adxt->stm == 0) {
        ADXERR_ItoA2(patid, fid, error, 16);
        ADXERR_CallErrFunc2("E8101202 adxt_StartAfs: can't open ", error);
        adxt->errcode = -1;
        adxt->stat = ADXT_ISTAT_ERROR;
        return;
    }
    adxt->stm_fname = adxt->wkend;
    adxt->stm_dir = dir;
    adxt->stm_ofst = ofst;
    adxt->stm_nsct = fnsct;
    adxt->stat = ADXT_ISTAT_DECINFO;
    adxt->stmstart = 1;
    adxt->mode = 1;
    ADXT_SetLnkSw(adxt, 0);
}

void ADXT_StartAfs(ADXT adxt, Sint32 patid, Sint32 fid) {
    ADXCRS_Enter();
    adxt_StartAfs(adxt, patid, fid);
    ADXCRS_Leave();
}

/* Starts a sector range of a named file; the name is copied into the handle's work area. */
static inline void adxt_StartFnameRange(ADXT adxt, Char8 *fname, void *dir, Sint32 ofst,
                                        Sint32 nsct) {
    if (adxt == 0 || fname == 0) {
        ADXERR_CallErrFunc1("E02080807 adxt_StartFnameRange: parameter error");
        return;
    }
    ADXT_Stop(adxt);
    strcpy((Char8 *)adxt->wkend, fname);
    adxt->stm_fname = adxt->wkend;
    adxt->stm_dir = dir;
    adxt->stm_ofst = ofst;
    adxt->stm_nsct = nsct;
    adxt->stat = ADXT_ISTAT_DECINFO;
    adxt->stmstart = 1;
    adxt->mode = 0;
    ADXT_SetLnkSw(adxt, 0);
}

/* Starts a whole named file. */
void ADXT_StartFname(ADXT adxt, Char8 *fname) {
    ADXCRS_Enter();
    adxt_StartFnameRange(adxt, fname, 0, 0, 0xFFFFF);
    ADXCRS_Leave();
}
