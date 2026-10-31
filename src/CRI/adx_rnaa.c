/* CRI ADX renderer front end (adx_rnaa.c): the ADXRNA_* wrappers the ADX talk layer uses to drive
 * the platform renderer, here the AX renderer AXRNA. The balance and total-sample entries are
 * no-ops on AX. Bodies from the RE4 GameCube decompilation of the same file; ADXRNA_GetStat is
 * not in this game.
 */
#include "CRI/adx_rna.h"

/* Forwards the sampling-rate correction switch. */
void ADXRNA_SetAdjsfreqFlg(AXRNA rna, Sint32 flg) {
    AXRNA_SetAdjsfreqFlg(rna, flg);
}

/* Passes stream header info to the renderer (unused by AXRNA). */
Sint32 ADXRNA_SetStmHdInfo(AXRNA rna, void *hdinfo) {
    AXRNA_SetStmHdInfo(rna, hdinfo);
    return 0;
}

/* No-op: the AX renderer does not need the total sample count. */
void ADXRNA_SetTotalNumSmpl(AXRNA rna, Sint32 nsmpl) {}

/* Forwards a discard request (AXRNA does not implement it and returns 0). */
Sint32 ADXRNA_DiscardData(AXRNA rna, Sint32 nsmpl) {
    return AXRNA_DiscardData(rna, nsmpl);
}

/* Forwards the PCM bit depth. */
void ADXRNA_SetBitPerSmpl(AXRNA rna, Sint32 bps) {
    AXRNA_SetBitPerSmpl(rna, bps);
}

/* No-op: balance is not implemented on AX. */
void ADXRNA_SetOutBalance(AXRNA rna, Sint32 bal) {}

/* Forwards the per-channel pan (-15..15). */
void ADXRNA_SetOutPan(AXRNA rna, Sint32 ch, Sint32 pan) {
    AXRNA_SetOutPan(rna, ch, pan);
}

/* Forwards the output volume (1/10 dB, <= 0). */
void ADXRNA_SetOutVol(AXRNA rna, Sint32 vol) {
    AXRNA_SetOutVol(rna, vol);
}

/* Forwards the sampling rate. */
void ADXRNA_SetSfreq(AXRNA rna, Sint32 sfreq) {
    AXRNA_SetSfreq(rna, sfreq);
}

/* Forwards the active channel count. */
void ADXRNA_SetNumChan(AXRNA rna, Sint32 nch) {
    AXRNA_SetNumChan(rna, nch);
}

/* Runs the renderer server (ARAM refills), last phase of ADXT_ExecServer. */
void ADXRNA_ExecServer(void) {
    AXRNA_ExecServer();
}

/* Free samples in the renderer's ARAM ring. */
Sint32 ADXRNA_GetNumRoom(AXRNA rna) {
    return AXRNA_GetNumRoom(rna);
}

/* Samples buffered in the renderer's ARAM ring but not yet played. */
Sint32 ADXRNA_GetNumData(AXRNA rna) {
    return AXRNA_GetNumData(rna);
}

/* Starts (1) / stops (0) the AX voices. */
void ADXRNA_SetPlaySw(AXRNA rna, Sint32 sw) {
    AXRNA_SetPlaySw(rna, sw);
}

/* Enables (1) / disables (0) the PCM transfer from the decoder output into ARAM. */
void ADXRNA_SetTransSw(AXRNA rna, Sint32 sw) {
    AXRNA_SetTransSw(rna, sw);
}

/* Stops playback and transfer, then destroys the AXRNA. */
void ADXRNA_Destroy(AXRNA rna) {
    AXRNA_SetPlaySw(rna, 0);
    AXRNA_SetTransSw(rna, 0);
    AXRNA_Destroy(rna);
}

/* Creates an AXRNA over the decoder output stream joints (the work area is not used on AX). */
AXRNA ADXRNA_Create(SJ *sjo, Sint32 nch, void *work) {
    return AXRNA_Create(sjo, nch);
}

/* Registers the renderer error callback. */
void ADXRNA_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj) {
    AXRNA_EntryErrFunc(func, obj);
}

/* Renderer shutdown. */
void ADXRNA_Finish(void) {
    AXRNA_Finish();
}

/* Renderer init. */
void ADXRNA_Init(void) {
    AXRNA_Init();
}
