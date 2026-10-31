/* CRI Sofdec MW player servers (mwsfdsvr.c): the decode server (MWSFSVR_DecodeServer runs
 * SFD_ExecOne per handle and then the player state machine mwSfdExecDecSvrHndl: PREP starts the
 * stream and the decoder, PLAYING watches for the end and for stream errors), the vsync tick and
 * the three SVM server callbacks of the library.
 *
 * Names follow the RE4 GameCube decompilation of the same file. The inlined static helpers have no
 * symbol in any build; their names are inferred, except mwlSfdExecDecSvrPlaying, which its error
 * message names.
 *
 * The main callback decodes when the library is set to decode in the main server and the idle
 * callback otherwise. Compiled with -inline auto,deferred: with deferred inlining the helpers are
 * inlined even into callers defined before them, and the functions are emitted in reverse
 * definition order, so the definitions below run from the last function of the unit to the first.
 */
#include "CRI/adx.h"
#include "CRI/mwsfd.h"
#include "CRI/mwsfdrsc.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"
#include "CRI/sfd.h"

static Sint32 mwg_field_no = 0;
Sint32 mwg_vcnt = 0;
MWPLY mwsfd_hn_last = 0;
/* not referenced by any function that remains in the game */
static Sint32 mwsfd_svr_bdr_cnt = 0;

static Sint32 mwsfd_ExecSvrHndl(void *obj);

/* Starts the file stream controller on the handle's file range and points the decoder input at
 * the file joint; -1 if the controller is still running or refuses, 1 once started. */
static Sint32 mwsfd_StartStm(MWPLY mwply) {
    if (MWSTM_GetStat(mwply->stm) == ADXSTM_STAT_EXEC) {
        return -1;
    }
    if (mwply->sji != 0) {
        SJ_Reset(mwply->sji);
    }
    MWSTM_SetFileRange(mwply->stm, mwply->fname, mwply->dir, mwply->ofst, mwply->nsct);
    if (MWSTM_ReqStart(mwply->stm) == -1) {
        mwply->stat = MWSFD_STAT_ERROR;
        MWSFLIB_SetErrCode(-0x66);
        MWSFSVM_Error("E211141 MWSTM_ReqStart: can't start '%s'", mwply->fname);
        mwply->stm_start_req = 0;
        return -1;
    }
    MWSFCRE_SetSupplySj(mwply);
    return 1;
}

/* Starts the SFD decoder, releases the pause if none is requested and enables concatenated play
 * for a linked stream. */
static void mwsfd_StartPlay(MWPLY mwply) {
    mwPlySfdStart(mwply);
    if (mwply->pause_flg == 0) {
        mwPlyPause(mwply, 0);
    }
    if (mwply->linkstm == 1) {
        if (SFD_SetConcatPlay(mwply->sfd) != 0) {
            MWSFSVM_Error("E99072103 mwPlyStartXX: can't link stream");
        }
    }
}

/* The PLAYING step: once the load scheduler has no streams left, ends the supply of a linked stream
 * (on request) or checks the supply of a single one; the player is at the play end when the
 * decoder is. */
static void mwlSfdExecDecSvrPlaying(MWPLY mwply) {
    SFD sfd = mwply->sfd;

    if (mwply->linkstm_req == 1) {
        if (LSC_GetNumStm(mwply->lsc) == 0) {
            if (SFD_TermSupply(sfd) != 0) {
                MWSFSVM_Error("E99072102 mwlSfdExecDecSvrPlaying: can't term");
            }
            mwply->linkstm_req = 0;
        }
    }
    if (mwply->linkstm == 0 && mwply->linkstm_req == 0 && LSC_GetNumStm(mwply->lsc) == 0) {
        mwPlyChkSupply(mwply);
    }
    if (SFD_GetHnStat(sfd) == SFD_STAT_PLAYEND) {
        mwply->stat = MWSFD_STAT_PLAYEND;
    }
}

/* The player state machine, one step per decode pass. In PREP the file stream is started on
 * request and the decoder once its input is ready, then the player is PLAYING; in PLAYING linked
 * streams are ended once the load scheduler runs dry and the supply is checked. Stream and load
 * scheduler errors set the error state. */
Sint32 mwSfdExecDecSvrHndl(MWPLY mwply) {
    SFD sfd;
    Sint32 sfdstat;
    Sint32 sststat;
    MWSST sst;

    switch (mwply->stat) {
    case MWSFD_STAT_STOP:
        break;
    case MWSFD_STAT_PREP:
        sfd = mwply->sfd;
        if (mwply->stm_start_req == 1) {
            if (mwsfd_StartStm(mwply) == 1) {
                mwply->stm_start_req = 0;
            }
        }
        if (mwply->sst.used == 1) {
            sst = &mwply->sst;
            sfdstat = SFD_GetHnStat(mwply->sfd);
            sststat = MWSST_GetStat(sst);
            if (sfdstat == SFD_STAT_STBY &&
                (sststat == 2 || SJ_GetNumData(sst->sj, SJ_CK_DATA) == 0)) {
                mwsfd_StartPlay(mwply);
                if (mwply->pause_flg == 0) {
                    MWSST_Pause(sst, 0);
                }
            }
        } else {
            if (SFD_GetHnStat(mwply->sfd) == SFD_STAT_STBY) {
                mwsfd_StartPlay(mwply);
            }
        }
        sfdstat = SFD_GetHnStat(sfd);
        if (sfdstat == SFD_STAT_PLAYING || sfdstat == SFD_STAT_PLAYEND) {
            mwply->stat = MWSFD_STAT_PLAYING;
        }
        break;
    case MWSFD_STAT_PLAYING:
        mwlSfdExecDecSvrPlaying(mwply);
        break;
    case MWSFD_STAT_PLAYEND:
        break;
    }
    if (mwply->stm != 0 && MWSTM_IsFsStatErr(mwply->stm) != 0) {
        mwply->stat = MWSFD_STAT_ERROR;
    }
    if (mwply->lsc != 0 && MWSFLSC_IsFsStatErr(mwply->lsc) == 1) {
        mwply->stat = MWSFD_STAT_ERROR;
    }
    MWSFSEE_ChkSupply(mwply);
    return 0;
}

/* SVM vsync-type callback: counts the field and the vsync and advances the SFD clock. */
Sint32 MWSFSVR_VsyncThrdProc(void *obj) {
    MWSFD_LIBWORK *lw;

    mwg_field_no++;
    mwg_vcnt++;
    if (mwsfd_init_flag == 1) {
        lw = MWSFLIB_GetLibWorkPtr();
        if (MWSFSVM_TestAndSet(&lw->vsync_flg) == 1) {
            if (mwsfd_init_flag == 1) {
                SFD_VbIn();
            }
            lw->vsync_flg = 0;
        }
    }
    return 0;
}

/* SVM main-type callback: decodes when the library decodes in the main server. */
Sint32 MWSFSVR_MainThrdProc(void *obj) {
    Sint32 ret = 0;

    if (MWSFLIB_GetLibWorkPtr()->main_decode == 1) {
        ret = MWSFSVR_DecodeServer(obj);
    }
    return ret;
}

/* SVM idle-type callback: decodes unless the library decodes in the main server. */
Sint32 MWSFSVR_IdleThrdProc(void *obj) {
    Sint32 ret = 0;

    if (MWSFLIB_GetLibWorkPtr()->main_decode != 1) {
        ret = MWSFSVR_DecodeServer(obj);
    }
    return ret;
}

/* The Sofdec decode server pass: pre hook, the handle step on all eight handles, post hook, then
 * the idle hook when no SFD handle has work; returns 1 while a handle has work. */
Sint32 MWSFSVR_DecodeServer(void *obj) {
    MWSFD_LIBWORK *lw;
    MWSFD_LIBWORK *hooks;
    Sint32 i;
    Sint32 busy;
    MWPLY mwply;
    Sint32 (*func)(void *obj);
    void *fobj;

    if (mwsfd_init_flag != 1) {
        return 0;
    }
    lw = MWSFLIB_GetLibWorkPtr();
    if (MWSFSVM_TestAndSet(&lw->svr_flg) != 1) {
        return 0;
    }
    hooks = MWSFLIB_GetLibWorkPtr();
    func = hooks->pre_func;
    fobj = hooks->pre_obj;
    if (func != 0) {
        func(fobj);
    }
    for (i = 0; i < MWSFD_MAX_HN; i++) {
        mwply = &lw->hn[i];
        if (mwply != 0) {
            mwPlyExecSvrHndl(mwply);
        }
    }
    hooks = MWSFLIB_GetLibWorkPtr();
    hooks->svr_flg = 0;
    busy = SFD_IsSvrWait() != 1;
    hooks = MWSFLIB_GetLibWorkPtr();
    func = hooks->post_func;
    fobj = hooks->post_obj;
    if (func != 0) {
        func(fobj);
    }
    if (busy != 1) {
        hooks = MWSFLIB_GetLibWorkPtr();
        if (hooks->svr_bdr != 1) {
            hooks = MWSFLIB_GetLibWorkPtr();
            func = hooks->idle_func;
            fobj = hooks->idle_obj;
            if (func != 0) {
                func(fobj);
            }
        }
    }
    return busy;
}

/* Matching workaround: #pragma dont_inline around the handle step functions below. With
 * inlining the handle step is expanded into the decode server (0x1EC bytes instead of 0x114) and
 * mwsfd_ExecSvrHndl into its caller. Revisit when the original unit's inline settings are known. */
#pragma dont_inline on
/* Public entry of the handle step. */
Sint32 mwPlyExecSvrHndl(MWPLY mwply) {
    return mwSfdExecSvrHndl(mwply);
}

/* One decode step of a handle unless the library is not initialised, the handle is unused, already
 * running, or a handle sleeps at the border. */
Sint32 mwSfdExecSvrHndl(MWPLY mwply) {
    MWSFD_LIBWORK *lw;

    if (mwsfd_init_flag != 1) {
        return 0;
    }
    if (mwply == 0) {
        MWSFSVM_Error("E1071901 mwPlyExecSvrHndl: NULL handle.");
        return 0;
    }
    if (mwply->used != 1) {
        return 0;
    }
    if (mwply->mwply_svr_flg == 1) {
        return 0;
    }
    lw = MWSFLIB_GetLibWorkPtr();
    if (lw->svr_bdr == 1) {
        return 0;
    }
    return mwsfd_ExecSvrHndl(mwply);
}

/* One decode step of an in-use handle: SFD_ExecOne, then the player state machine unless the
 * player is stopped. Returns 1 while the SFD handle still has work. The handle is taken as void *
 * and converted. Matching workaround: with an MWPLY parameter mwsfd_ExecSvrHndl differs by 14
 * words (same size). Revisit when the original prototype is known. */
static Sint32 mwsfd_ExecSvrHndl(void *obj) {
    MWPLY mwply;
    SFD sfd;

    mwply = obj;
    sfd = mwply->sfd;
    mwply->mwply_svr_flg = 1;
    if (mwply->used != 1) {
        mwply->mwply_svr_flg = 0;
        return 0;
    }
    mwsfd_hn_last = mwply;
    mwply->sfd_svr_flg = 1;
    SFD_ExecOne(sfd);
    mwply->sfd_svr_flg = 0;
    if (mwply->stat == MWSFD_STAT_STOP) {
        mwply->dec_svr_flg = 0;
    } else {
        mwply->dec_svr_flg = 1;
        mwSfdExecDecSvrHndl(mwply);
    }
    mwply->mwply_svr_flg = 0;
    return SFD_IsHnSvrWait(sfd) != 1;
}

#pragma dont_inline off

/* "Decode server running" flag of the library. */
void MWSFSVR_SetMwsfdSvrFlg(Sint32 flg) {
    MWSFLIB_GetLibWorkPtr()->svr_flg = flg;
}

/* "Handle server step running" flag of a handle. */
void MWSFSVR_SetHnMwplySvrFlg(MWPLY mwply, Sint32 flg) {
    mwply->mwply_svr_flg = flg;
}

/* "SFD_ExecOne running" flag of a handle. */
void MWSFSVR_SetHnSfdSvrFlg(MWPLY mwply, Sint32 flg) {
    mwply->sfd_svr_flg = flg;
}

/* Marks the handle and the library as sleeping at the server border. The name is inferred (from the
 * RE4 GameCube decompilation, where the code is inlined). */
void mwsfd_SetSleepBdr(MWPLY mwply, Sint32 sw) {
    MWSFD_LIBWORK *lw;

    lw = MWSFLIB_GetLibWorkPtr();
    if (mwply != 0) {
        mwply->sleep_bdr = sw;
    }
    lw->svr_bdr = sw;
}

/* Waits up to 10 vsyncs for the handle's server step to finish. The helper is inferred (it is
 * inlined and has no symbol). Matching workaround: with the loop written in mwlSfdSleepDecSvr
 * that function is 224 bytes instead of 204 (53 differing words). Revisit when the original
 * helper is known. */
static void mwsfd_SleepLoop(MWPLY mwply) {
    Sint32 i;

    for (i = 0; i < 10; i++) {
        mwsfd_SetSleepBdr(mwply, 1);
        ADXM_WaitVsync();
        mwsfd_SetSleepBdr(mwply, 0);
        if (mwply->mwply_svr_flg != 1) {
            break;
        }
    }
}

/* Waits until the decode server is between passes before a stop or pause touches the decoder: goes
 * to the SVM idle border and, if the handle's server step is still running, waits up to 10
 * vsyncs. */
void mwlSfdSleepDecSvr(MWPLY mwply) {
    mwPlySaveRsc();
    mwsfd_SetSleepBdr(mwply, 1);
    MWSFSVM_GotoIdleBorder();
    mwsfd_SetSleepBdr(mwply, 0);
    mwPlyRestoreRsc();
    if (mwply->mwply_svr_flg == 1) {
        mwsfd_SleepLoop(mwply);
    }
}

/* Matching workaround: order-only stand-in for a function the linker stripped from the game (its
 * body is unknown); its error text is the first literal of the unit. Revisit when the stripped
 * function is recovered. */
static void mwsfdsvr_Stripped(void) {
    MWSFSVM_Error("E2011101: MWSFSVR_IsSvrBdrHndl: handle is invalid.");
}
