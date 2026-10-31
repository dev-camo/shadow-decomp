/* CRI Sofdec MW player handle accessors (mwsfdset.c): the SFD handle, conditions, flow limits and
 * the audio volume and state queries of a player handle.
 *
 * Names follow the RE4 GameCube decompilation of the same file, except the ones the error messages
 * give (mwPlyGetOutVol, mwPlySetOutVol, mwPlyGetStat).
 *
 * Compiled with -inline auto,deferred: with deferred inlining the accessors inline
 * MWSFD_IsEnableHndl even where it is defined after them, and the functions are emitted in reverse
 * definition order.
 */
#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"

/* The SFD handle of a player handle (0 for an invalid handle). */
SFD mwPlyGetSfdHn(MWPLY mwply) {
    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122640: mwPlyGetSfdHn: handle is invalid.");
        return 0;
    }
    return mwply->sfd;
}

/* Sets an SFD condition on the player's decoder (library-wide defaults without a handle). */
void MWSFD_SetCond(MWPLY mwply, Sint32 id, Sint32 val) {
    SFD_SetCond((mwply != 0) ? mwply->sfd : 0, id, val);
}

/* Refill limits (sectors) of the stream controller and the load scheduler. */
void MWSFD_SetFlowLimit(MWPLY mwply, Sint32 min_nsct, Sint32 max_nsct) {
    MWSTM_SetFlowLimit(mwply->stm, min_nsct, max_nsct);
    MWSFLSC_SetFlowLimit(mwply, min_nsct);
}

/* Non-NULL and in use. */
Bool MWSFD_IsEnableHndl(MWPLY mwply) {
    if (mwply == 0) {
        return FALSE;
    }
    return mwply->used;
}

/* State of the player: stop, prep, playing, play end or error; while the player thinks it is
 * playing the SFD state decides (playing or play end: playing, negative: error, otherwise still
 * preparing). */
Sint32 mwPlyGetStat(MWPLY mwply) {
    Sint32 stat;
    Sint32 sfdstat;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFLIB_SetErrCode(-12);
        MWSFSVM_Error("W2004 mwPlyGetStat: handle is invalid");
        return MWSFD_STAT_STOP;
    }
    stat = mwply->stat;
    if (stat == MWSFD_STAT_PLAYING) {
        sfdstat = SFD_GetHnStat(mwply->sfd);
        if (sfdstat == SFD_STAT_PLAYING || sfdstat == SFD_STAT_PLAYEND) {
            return MWSFD_STAT_PLAYING;
        }
        if (sfdstat < 0) {
            return MWSFD_STAT_ERROR;
        }
        return MWSFD_STAT_PREP;
    }
    return stat;
}

/* Audio volume (1/10 dB) of the main and the side audio stream. The handle is tested as void *
 * before the conversion (required: testing the converted MWPLY changes the register use). */
void mwPlySetOutVol(void *hn, Sint32 vol) {
    MWPLY mwply = hn;

    if (MWSFD_IsEnableHndl(hn) != TRUE) {
        MWSFSVM_Error("E1122605 mwPlySetOutVol: handle is invalid.");
        return;
    }
    MWSFRNA_SetOutVol(mwply, vol);
    MWSST_SetOutVol(&mwply->sst, vol);
}

/* Audio volume of the main audio stream, or the side stream's when the main volume is 0 (0 dB)
 * and the two differ (0 without an SFD handle). The handle is tested as void * as in
 * mwPlySetOutVol. */
Sint32 mwPlyGetOutVol(void *hn) {
    MWPLY mwply = hn;
    Sint32 vol;
    Sint32 sstvol;

    if (MWSFD_IsEnableHndl(hn) != TRUE) {
        MWSFSVM_Error("E1122606 mwPlyGetOutVol: handle is invalid.");
        return 0;
    }
    if (mwply->sfd == 0) {
        return 0;
    }
    vol = MWSFRNA_GetOutVol(mwply);
    sstvol = MWSST_GetOutVol(&mwply->sst);
    if (vol == sstvol) {
        return vol;
    }
    if (vol != 0) {
        return vol;
    }
    return sstvol;
}

/* The original pools mwPlyGetSfdHn's error text first. This never-called stand-in for a function
 * the linker stripped uses the same text and, defined last, is emitted first (reverse definition
 * order). Its body is unknown. */
static void mwsfdset_Stripped(void) {
    MWSFSVM_Error("E1122640: mwPlyGetSfdHn: handle is invalid.");
}
