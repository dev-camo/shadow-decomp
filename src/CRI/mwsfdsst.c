/* CRI Sofdec MW player side stream (mwsfdsst.c, MWSST): wrapper for a second audio stream decoded
 * by a separately registered core library (mwsstmng.ifc). No such library is registered in this
 * game, so every call is a guarded no-op; the player still embeds one MWSST_OBJ per handle.
 *
 * Names follow the RE4 GameCube decompilation of the same file. The inlined static helpers have no
 * symbol in any build; their names are inferred.
 */
#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/sj.h"

/* element stream ids of the side streams start here */
#define SFSST_STMID_BASE 0xC0

MWSST_MNG mwsstmng = {0};

/* A side stream exists only when a core library is registered and the object has a core handle. */
static Bool mwsst_IsValid(MWSST sst) {
    if (mwsstmng.ifc == 0) {
        return FALSE;
    }
    if (sst->used != 1) {
        return FALSE;
    }
    if (sst->hn == 0) {
        return FALSE;
    }
    return TRUE;
}

/* Stops the core handle. */
static void mwsst_Stop(MWSST sst) {
    MWSST hn;

    if (mwsst_IsValid(sst) == TRUE) {
        hn = sst->hn;
        if (mwsstmng.ifc != 0 && mwsstmng.ifc->Stop != 0) {
            mwsstmng.ifc->Stop(hn);
        }
    }
}

/* Destroys the core handle. */
static void mwsst_DestroyHn(MWSST hn) {
    MWSST_IF *ifc;

    ifc = mwsstmng.ifc;
    if (hn != 0 && ifc != 0 && ifc->Destroy != 0) {
        ifc->Destroy(hn);
    }
}

/* Drops a reference on the core library, finishing it with the last one. */
static void mwsst_ReleaseLib(void) {
    MWSST_IF *ifc;

    ifc = mwsstmng.ifc;
    if (ifc != 0 && mwsstmng.cnt != 0) {
        mwsstmng.cnt--;
        if (mwsstmng.cnt == 0 && ifc->Finish != 0) {
            ifc->Finish();
        }
    }
}

/* Stops and destroys the side stream at the idle border. */
void MWSST_Destroy(MWSST sst) {
    MWSST hn;
    SJ sj;

    if (mwsst_IsValid(sst) == TRUE) {
        hn = sst->hn;
        sj = sst->sj;
        if (hn != 0) {
            MWSFSVM_GotoIdleBorder();
            /* as in the original, the core handle is passed, so the stop goes to hn->hn */
            mwsst_Stop(hn);
            sst->used = 0;
            mwsst_DestroyHn(hn);
            SJ_Destroy(sj);
            sst->hn = 0;
            mwsst_ReleaseLib();
        }
    }
}

/* Resets the ring and re-registers it as the SFD output for the side stream's element stream. */
void MWSST_Reset(MWPLY mwply) {
    SFD sfd;
    MWSST sst;
    MWSST hn;
    SJ sj;
    Sint32 stmno;

    sfd = mwply->sfd;
    sst = &mwply->sst;
    hn = sst->hn;
    sj = sst->sj;
    stmno = sst->stmno;
    if (mwsst_IsValid(sst) == TRUE) {
        if (hn != 0) {
            /* as in the original, the core handle is passed, so the stop goes to hn->hn */
            mwsst_Stop(hn);
        }
        SJ_Reset(sj);
        SFD_SetElementOutSj(sfd, SFSST_STMID_BASE + stmno, sj, 0, 0);
    }
}

/* Volume of the side stream (0 without a core library). */
Sint32 MWSST_GetOutVol(MWSST sst) {
    Sint32 vol = 0;
    MWSST hn;

    if (mwsst_IsValid(sst) != TRUE) {
        return 0;
    }
    hn = sst->hn;
    if (mwsstmng.ifc != 0 && mwsstmng.ifc->GetOutVol != 0) {
        vol = mwsstmng.ifc->GetOutVol(hn);
    }
    return vol;
}

/* Sets the volume of the side stream. */
void MWSST_SetOutVol(MWSST sst, Sint32 vol) {
    MWSST hn;

    if (mwsst_IsValid(sst) == TRUE) {
        hn = sst->hn;
        if (mwsstmng.ifc != 0 && mwsstmng.ifc->SetOutVol != 0) {
            mwsstmng.ifc->SetOutVol(hn, vol);
        }
    }
}

/* Pauses or resumes the side stream. */
void MWSST_Pause(MWSST sst, Sint32 sw) {
    MWSST hn;

    if (mwsst_IsValid(sst) == TRUE) {
        hn = sst->hn;
        if (mwsstmng.ifc != 0 && mwsstmng.ifc->Pause != 0) {
            mwsstmng.ifc->Pause(hn, sw);
        }
    }
}

/* State of the side stream (0 without a core library). */
Sint32 MWSST_GetStat(MWSST sst) {
    Sint32 stat = 0;
    MWSST hn;

    if (mwsst_IsValid(sst) != TRUE) {
        return 0;
    }
    hn = sst->hn;
    if (mwsstmng.ifc != 0 && mwsstmng.ifc->GetStat != 0) {
        stat = mwsstmng.ifc->GetStat(hn);
    }
    return stat;
}

/* Stops the side stream. */
void MWSST_Stop(MWSST sst) {
    mwsst_Stop(sst);
}

/* Starts the side stream on its ring buffer joint. */
void MWSST_StartSj(MWSST sst) {
    MWSST hn;
    SJ sj;

    if (mwsst_IsValid(sst) == TRUE) {
        hn = sst->hn;
        sj = sst->sj;
        if (mwsstmng.ifc != 0 && mwsstmng.ifc->StartSj != 0) {
            mwsstmng.ifc->StartSj(hn, sj);
        }
    }
}
