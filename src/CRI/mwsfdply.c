/* CRI Sofdec MW player start, stop and pause (mwsfdply.c): the end-of-file notice to the decoder,
 * the stream flow limits, pausing and stopping the decoder, and starting a play from a file.
 *
 * Names follow the RE4 GameCube decompilation of the same file, where the functions come in
 * this order. This version names the pause function after its error messages (mwPlyPause) and
 * keeps the decoder stop (mwSfdStopDec) and the pause state query (mwPlySfdGetPaStat) in the
 * linked executable; the start from a stream joint or from memory and the stop with a handle
 * check were removed by the linker.
 *
 * The original object also held functions the linker removed. Their error texts remain in
 * .rodata between the surviving functions' texts, which fixes the order of the definitions
 * below; the stand-ins that reproduce them are marked.
 */
#include <stdio.h>
#include <string.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"

/* Matching workaround: stand-in for mwPlyStartFnameRange, which the linker removed. Only the
 * order of its error texts is known, from the literals that stay in .rodata (the file name
 * length message is the one the file start shares with it); the body is not the original's.
 * Never called. The same holds for each stand-in below. Revisit when the stripped code is
 * known. */
static void mwsfply_StrippedStartFnameRange(Uint32 ofst, Uint32 nsct) {
    Char8 buf[32];

    MWSFSVM_Error("E407021: mwPlyStartFnameRange: handle is invalid.");
    MWSFSVM_Error("E407022: mwPlyStartFnameRange: fname is NULL.");
    MWSFSVM_Error("E211121: filename is longer.");
    sprintf(buf, "%08x.%08x", ofst, nsct);
    MWSFSVM_Error("E211151: ADXF_GetFnameRangeEx() faild.");
}

/* Ends the SFD supply once the file stream controller has read its whole range. */
void mwPlyChkSupply(MWPLY mwply) {
    ADXSTM stm = mwply->stm;
    SFD sfd = mwply->sfd;

    if (stm != 0 && MWSTM_GetStat(stm) == ADXSTM_STAT_END) {
        SFD_TermSupply(sfd);
    }
}

/* Passes MWSFD_SetFlowLimit 80% of the player's flow limit (flow_nsct) and the limit itself. */
void MWSFPLY_SetFlowLimit(MWPLY mwply) {
    Sint32 max_nsct = mwply->flow_nsct;

    MWSFD_SetFlowLimit(mwply, (Sint32)(max_nsct * 0.8), max_nsct);
}

/* Matching workaround: stand-in, as above, for mwPlyStartAfs. */
static void mwsfply_StrippedStartAfs(void) {
    MWSFSVM_Error("E1122638: mwPlyStartAfs: handle is invalid.");
}

/* Pauses or resumes the decoder and the side audio stream. With the pause border set and mode 1
 * it first sleeps the decode server when the audio is on, or when that condition cannot be read.
 *
 * Matching workaround: the handle is taken as void * and converted to a typed local, as in
 * mwPlySetOutVol. With a typed parameter the handle and the pause switch exchange registers (16
 * differing words). Revisit when the original public prototype is known. */
void mwPlyPause(void *hn, Sint32 sw) {
    MWPLY mwply = hn;
    SFD sfd;
    Sint32 val;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122604 mwPlyPause; handle is invalid.");
        return;
    }
    sfd = mwply->sfd;
    if (mwply->pause_flg == 0 && sw == 0) {
        return;
    }
    if (MWSFD_GetPauseBdr() == 1 && mwply->prm.mode == MWSFD_FTYPE_SFD) {
        if (SFD_GetCond(sfd, SFD_COND_AUDIO_ON, &val) == 0) {
            if (val == 1) {
                mwlSfdSleepDecSvr(mwply);
            }
        } else {
            mwlSfdSleepDecSvr(mwply);
        }
    }
    if (SFD_Pause(sfd, sw) != 0) {
        MWSFLIB_SetErrCode(-0x136);
        MWSFSVM_Error("E2007 mwPlyPause; can't pause (%s)", (sw == 1) ? "ON" : "OFF");
    }
    MWSST_Pause(&mwply->sst, sw);
    mwply->pause_flg = sw;
}

/* Matching workaround: stand-in, as above, for mwSfdStop. */
static void mwsfply_StrippedSfdStop(void) {
    MWSFSVM_Error("E1122602 mwSfdStop: handle is invalid.");
}

/* Stops the decoder side of the handle: the decoder, the side audio stream, the file stream
 * controller and the load scheduler. */
void mwSfdStopDec(MWPLY mwply) {
    SFD sfd = mwply->sfd;

    if (sfd != NULL) {
        mwlSfdSleepDecSvr(mwply);
        mwply->stat = MWSFD_STAT_STOP;
        mwply->sfd = NULL;
        if (SFD_Stop(sfd) != 0) {
            MWSFLIB_SetErrCode(-0x134);
            MWSFSVM_Error("E2003 mwSfdStop:can't stop SFD");
        }
        mwply->sfd = sfd;
        MWSST_Stop(&mwply->sst);
        mwply->x2a0 = 0;
        if (mwply->stm != NULL) {
            MWSTM_ReqStop(mwply->stm);
        }
        if (mwply->lsc != NULL) {
            LSC_Stop(mwply->lsc);
        }
    }
}

/* Puts the decoder in standby. */
static inline void mwPlySfdStandby(MWPLY mwply) {
    if (SFD_Standby(mwply->sfd) != 0) {
        MWSFLIB_SetErrCode(-0x137);
        MWSFSVM_Error("E20010703F mwPlySfdStandby: ");
    }
}

/* Prepares the handle for a new play: resets the decoder handle and the additional information
 * stream joint, puts the decoder in standby and the side audio stream in pause, and marks the
 * play as preparing. Stops early, with the play state untouched, when a reset fails. */
static inline void mw_sfd_start_ex(MWPLY mwply) {
    /* The original makes this call here and does not use the result. */
    MWSFLIB_GetLibWorkPtr();
    if (mwply->sfd != NULL) {
        if (MWSFCRE_ResetSfdHn(mwply) != 0) {
            MWSFSVM_Error("E0203263: mw_sfd_start_ex: RESET failed.");
            return;
        }
        MWSST_Reset(mwply);
        MWSFTAG_ResetAinfSj(mwply);
        if (MWSFTAG_SetAinfSj(mwply) != 0) {
            MWSFSVM_Error("E201213 mw_sfd_start_ex: can't set AddInfSJ");
            return;
        }
        MWSFTAG_InitTagInf(mwply);
        MWSFFRM_InitSfhInfTable(mwply);
    }
    mwply->ngetfrm = 0;
    mwply->nrelfrm = 0;
    mwPlySfdStandby(mwply);
    mwPlyPause(mwply, mwply->pause_flg);
    MWSST_Pause(&mwply->sst, 1);
    MWSST_StartSj(&mwply->sst);
    mwply->nskipdisp = 0;
    mwply->linkstm_req = 0;
    mwply->stat = MWSFD_STAT_PREP;
    mwply->frmapi = 0;
}

/* Matching workaround: stand-ins, as above, for the start from a stream joint (mwPlyStartSj,
 * which shares the start step with the file start) and the start from memory (mwPlyStartMem). */
static void mwsfply_StrippedStartSj(MWPLY mwply) {
    MWSFSVM_Error("E1122609 mwPlyStartSj: handle is invalid.");
    mw_sfd_start_ex(mwply);
}

static void mwsfply_StrippedStartMem(void) {
    MWSFSVM_Error("E1122610 mwPlyStartMem: handle is invalid.");
    MWSFSVM_Error("E4111701 mwPlyStartMem: can't play file type MPV. Use memory file system(MFS).");
}

/* Starts a play from a file: stops the previous play, prepares the handle, takes over the file
 * name and asks the decode server to start the file stream. */
void mwSfdStartFnameSub(MWPLY mwply, const Char8 *fname, Sint32 ofst, Sint32 nsct) {
    mwply->sji = mwply->file_sj;
    mwSfdStopDec(mwply);
    mw_sfd_start_ex(mwply);
    if ((Sint32)strlen(fname) > mwply->fname_siz) {
        MWSFSVM_Error("E211121: filename is longer.");
        strncpy(mwply->fname, fname, mwply->fname_siz);
    } else {
        strcpy(mwply->fname, fname);
    }
    mwply->dir = NULL;
    mwply->ofst = 0;
    mwply->nsct = ADXSTM_NSCT_INF;
    mwply->stm_start_req = 1;
    MWSFSEE_StartFnameSub1(mwply, ofst, nsct);
    MWSFSEE_StartFnameSub2(mwply, ofst, nsct);
}

/* Matching workaround: #pragma dont_inline around mwSfdStartFname. Without it mwSfdStartFnameSub
 * is expanded into mwSfdStartFname and that into mwPlyStartFname (0x30C bytes each instead of
 * 0x84 and 0x20), although mwPlyPause and mwSfdStopDec are expanded into mwSfdStartFnameSub.
 * The pragma gives the same object as compiling the unit with -inline auto,level=2 (inlining two
 * calls deep) and no pragma; that flag does not fit the library as a whole (mwsfdsfx.c, sfd_mps.c
 * and mwsfdsvr.c change with it). Revisit when the original unit's inline settings are known. */
#pragma dont_inline on

/* Starts a play of the whole file fname.
 *
 * Matching workaround: the handle is taken as void *, as in mwPlyPause. With a typed parameter
 * the handle and the file name exchange registers (5 differing words). */
void mwSfdStartFname(void *hn, const Char8 *fname) {
    MWPLY mwply = hn;

    if (MWSFD_IsEnableHndl(mwply) != TRUE) {
        MWSFSVM_Error("E1122601: mwPlyStartFname: handle is invalid.");
        return;
    }
    if (fname == NULL) {
        MWSFSVM_Error("E10915C: mwPlyStartFname: fname is NULL.");
        return;
    }
    mwSfdStartFnameSub(mwply, fname, 0, -1);
}
#pragma dont_inline off

/* The name is inferred from the error text above: the entry the game calls. */
void mwPlyStartFname(MWPLY mwply, const Char8 *fname) {
    mwSfdStartFname(mwply, fname);
}

/* Whether the decoder is paused: 0 or 1; other states are reported and 0. */
Sint32 mwPlySfdGetPaStat(MWPLY mwply) {
    Sint32 stat;

    if (mwply->sfd == NULL) {
        return 0;
    }
    stat = SFD_GetPaStat(mwply->sfd);
    switch (stat) {
    case 1:
        return 1;
    case 0:
        return 0;
    default:
        MWSFSVM_Error("E10821A : Invalid value of SFD_GetPaStat : %d", stat);
        break;
    }
    return 0;
}

/* Starts the decoder once it is in standby; -0x133 when it cannot. */
Sint32 mwPlySfdStart(MWPLY mwply) {
    if (SFD_Start(mwply->sfd) != 0) {
        MWSFLIB_SetErrCode(-0x133);
        MWSFSVM_Error("E20010703G mwPlySfdStart: ");
        return -0x133;
    }
    return 0;
}
