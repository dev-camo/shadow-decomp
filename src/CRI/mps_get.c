/* CRI Sofdec MPS header accessors (mps_get.c): copies of the last decoded pack header (SCR, mux
 * rate), the three system headers (stream bounds) and the last packet header (stream id, PTS/DTS,
 * payload length/offset) for the Sofdec demux driver.
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/mps.h"

/* Copy of the last packet header (stream id, type, PTS/DTS, payload length and offset). */
Sint32 MPS_GetPketHd(MPS mps, MPS_PKETHD *hd) {
    if (MPSLIB_CheckHn(mps) != 0) {
        return MPSLIB_SetErr(0, 0xFF020203);
    }
    *hd = mps->pkethd;
    return 0;
}

/* Copy of the most recent system header. */
Sint32 MPS_GetLastSysHd(MPS mps, MPS_SYSHD *hd) {
    if (MPSLIB_CheckHn(mps) != 0) {
        return MPSLIB_SetErr(0, 0xFF020202);
    }
    *hd = mps->last_syshd;
    return 0;
}

/* Copy of system header `no` (0..2; -1 fields when not seen). */
Sint32 MPS_GetSysHd(MPS mps, MPS_SYSHD *hd, Sint32 no) {
    if (MPSLIB_CheckHn(mps) != 0) {
        return MPSLIB_SetErr(0, 0xFF020202);
    }
    *hd = mps->syshd[no];
    return 0;
}

/* Copy of the last pack header (SCR, mux_rate; -1 when not seen). */
Sint32 MPS_GetPackHd(MPS mps, MPS_PACKHD *hd) {
    if (MPSLIB_CheckHn(mps) != 0) {
        return MPSLIB_SetErr(0, 0xFF020201);
    }
    *hd = mps->packhd;
    return 0;
}

/* Nothing to release. */
void MPSGET_Finish(void) {}

/* Nothing to initialise. */
void MPSGET_Init(void) {}
