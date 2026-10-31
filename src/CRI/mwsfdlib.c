/* CRI Sofdec MW player library (mwsfdlib.c, MWSFD/GC Ver.3.50): mwPlyInitSfdFx brings up ADXT, the
 * stream joints, the SFD decoder core with its driver table (memory input, MPS demux, MPV video,
 * ADXT audio, manual video out, auto audio out, user output), the load scheduler and the SFX
 * converter, and registers the player's servers with the SVM; mwPlyFinishSfdFx undoes it. The unit
 * holds the library work (eight player handles) and the SFD error callback that turns error codes
 * into the messages the game's error callback receives.
 *
 * Names follow the RE4 GameCube decompilation of the same file. The inlined static helpers
 * (mwPlySfdInit, mwsflib_SetLibPrm, mwsflib_EntrySvr) have no symbol in any build; their names are
 * inferred.
 */
#include <stdio.h>
#include <string.h>

#include "CRI/adx_t.h"
#include "CRI/lsc.h"
#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"
#include "CRI/sfd.h"
#include "CRI/sj.h"

#define MWSFD_MAX_ERRCODE 16

static const Char8 mwsfd_ver_str[] = "\nMWSFD/GC Ver.3.50 Build:Feb 28 2005 19:36:03\n\0"
                                     "Append: MW2407 GC20Apr2004Patch2\n";

/* driver table of the SFD core, in driver order (the unused slots are 0) */
static const SFD_TR_IF *const mwsfd_trentry[15] = {
    &SFD_tr_in_mem,
    &SFD_tr_sd_mps,
    &SFD_tr_vd_mpv,
    &SFD_tr_vo_manu,
    &SFD_tr_ad_adxt,
    &SFD_tr_ao_auto_p,
    &SFD_tr_uo,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

/* The driver table is read through SFTRN_TRIF_TBL, the type of SFD_INIT_PRM.trif_tbl; the cast
 * drops the const of the table. 0xEA24 is 59.94 Hz x 1000, replaced by mwPlySfdInit with the
 * caller's rate. */
static const SFD_INIT_PRM mwsfd_initsfdpara = {(SFTRN_TRIF_TBL *)mwsfd_trentry, 0xEA24};

const Sint32 mwsfd_siz_mwplyhn = sizeof(MWPLY_OBJ);

Sint32 mwsfd_init_cnt;
Sint32 mwsfd_init_flag;
SFD mwsfd_err_sfdhn;
MWPLY mwsfd_err_mwsfdhn;
Char8 mwg_sfd_errstr[0x100];
Sint32 mwg_sfd_errcnt;
Sint32 mwg_sfd_errcode[MWSFD_MAX_ERRCODE];
MWSFD_LIBWORK mwsfd_libwork;
static const Char8 *cri_verstr_ptr;

/* Matching workaround: stand-in for a function the linker stripped from the game. The body is not
 * the original's. It refers to the library globals up to mwg_sfd_errcnt in storage order before
 * the functions below do; storing the addresses keeps the references. Without it the .bss objects
 * land in the first-reference order of the surviving functions and the code that pools their
 * addresses changes (measured by compiling without it). Revisit when the stripped code is known. */
static void mwsfdlib_BssOrder(void **out) {
    out[0] = &mwsfd_init_cnt;
    out[1] = &mwsfd_init_flag;
    out[2] = &mwsfd_err_sfdhn;
    out[3] = &mwsfd_err_mwsfdhn;
    out[4] = &mwg_sfd_errstr;
    out[5] = &mwg_sfd_errcnt;
}

/* SFD error callback: remembers the failing player and SFD handle, records the error codes (the
 * first 15, then the latest in the last slot), formats the message for the known codes (data
 * error, picture size, read buffer, frame pool, ADXT handle count, frame released twice) and
 * reports it through MWSFSVM_Error. */
void MWSFLIB_SfdErrFunc(void *obj, Sint32 code) {
    MWPLY mwply = obj;
    SFD sfd;

    if (mwply != 0) {
        sfd = mwPlyGetSfdHn(mwply);
        mwsfd_err_mwsfdhn = mwply;
        mwsfd_err_sfdhn = sfd;
    } else {
        mwsfd_err_mwsfdhn = 0;
        mwsfd_err_sfdhn = 0;
    }
    if (code != 0) {
        mwg_sfd_errcode[mwg_sfd_errcnt] = code;
        if (mwg_sfd_errcnt < MWSFD_MAX_ERRCODE - 1) {
            mwg_sfd_errcnt++;
        }
    }
    switch (code) {
    case -3:
    case -2:
        sprintf(mwg_sfd_errstr, "DATA ERROR(%08X)", code);
        break;
    case 0xFF000F17:
    case 0xFF000F18:
        sprintf(mwg_sfd_errstr,
                "SFD ERROR(%08X): 'max_width, max_height' of creation parameter is small. "
                "Increase this value.",
                code);
        break;
    case 0xFF000408:
    case 0xFF00040C:
    case 0xFF000F1C:
        sprintf(mwg_sfd_errstr,
                "SFD ERROR(%08X): Read buffer is small. Increase 'max_bps' of creation "
                "parameter.",
                code);
        break;
    case 0xFF000F15:
        sprintf(mwg_sfd_errstr,
                "SFD ERROR(%08X): Frame pool size is incorrect. Set positive integer to "
                "'nfrm_pool_wk' of creation parameter.",
                code);
        break;
    case 0xFF000C04:
        sprintf(mwg_sfd_errstr,
                "SFD ERROR(%08X): Number of ADXT handles exceeds its maximum number. MWPLY "
                "handle uses one ADXT handle(stereo) for MWSFD_FTYPE_SFD.",
                code);
        break;
    case 0xFF000F1F:
        sprintf(mwg_sfd_errstr,
                "SFD ERROR(%08X): mwPlyRelFrm() was called twice to the same frame ID.", code);
        break;
    default:
        sprintf(mwg_sfd_errstr, "SFD ERROR(%08X)", code);
        break;
    }
    MWSFSVM_Error(mwg_sfd_errstr);
}

/* Records the library error code (returned unchanged, 0 for success). */
Sint32 MWSFLIB_SetErrCode(Sint32 code) {
    mwsfd_libwork.errcode = code;
    if (code == 0) {
        return 0;
    }
    return code;
}

/* Library finish: destroys the handles still in use, waits for the idle border when there were
 * none, calls three unnamed functions (inferred to take the servers out of the SVM) and finishes
 * the modules. */
void mwPlyFinishSfdFx(void) {
    MWSFD_LIBWORK *lw = &mwsfd_libwork;
    MWPLY mwply;
    Sint32 i;
    Sint32 ndestroyed;

    mwsfd_init_cnt--;
    if (mwsfd_init_cnt == 0) {
        ndestroyed = 0;
        for (i = 0; i < MWSFD_MAX_HN; i++) {
            mwply = &lw->hn[i];
            if (mwply->used == 1) {
                mwSfdDestroy(mwply);
                ndestroyed++;
            }
        }
        if (ndestroyed == 0) {
            mwsfd_SetSleepBdr(0, 1);
            MWSFSVM_GotoIdleBorder();
            mwsfd_SetSleepBdr(0, 0);
        }
        fn_804032B4();
        fn_804031D4();
        fn_80403244();
        MWSFSFX_Finish();
        LSC_Finish();
        SFD_Finish();
        ADXT_Finish();
        MWSTM_FinishStatic();
        SJUNI_Finish();
        SJMEM_Finish();
        SJRBF_Finish();
        fn_80403334();
    }
}

/* Whether pausing waits for the decode server border (0 in this build). */
Sint32 MWSFD_GetPauseBdr(void) {
    return mwsfd_libwork.pause_bdr;
}

/* Whether picture user data is captured (1 in this build). */
Sint32 MWSFD_GetUsePicUsr(void) {
    return mwsfd_libwork.use_picusr;
}

/* Initialises the SFD core with the driver table and the refresh rate in 1/1000 Hz after checking
 * the version; installs the library-wide error callback. */
static Sint32 mwPlySfdInit(Sint32 vfreq1000) {
    SFD_INIT_PRM sfdprm;
    Sint32 ret;

    sfdprm = mwsfd_initsfdpara;
    sfdprm.prm1 = vfreq1000;
    if (SFD_IsVersionCompatible("1.958", SFD_VERSION) != TRUE) {
        MWSFSVM_Error("E011081 mwPlySfdInit: Not compatible SFD Version.");
        ret = -1;
    } else if (SFD_Init(&sfdprm) != 0) {
        ret = MWSFLIB_SetErrCode(-0x12D);
    } else if (SFD_SetErrFn(0, MWSFLIB_SfdErrFunc, 0) != 0) {
        ret = MWSFLIB_SetErrCode(-0x12F);
    } else {
        ret = 0;
    }
    return ret;
}

/* Copies the init parameters (refresh rate, x04, frame pool size, main-thread decode flag) into the
 * library work, or the defaults (59.94 Hz, x08 = 1, pool 1, decode in the idle server). */
static void mwsflib_SetLibPrm(MWSFD_LIBWORK *lw, MWSFD_INIT_PRM *prm) {
    if (prm != 0) {
        lw->vfreq = prm->vfreq;
        lw->x08 = prm->x04;
        lw->nfrm_pool = prm->nfrm_pool;
        lw->main_decode = prm->x0c;
    } else {
        lw->vfreq = 59.94f;
        lw->x08 = 1;
        lw->nfrm_pool = 1;
        lw->main_decode = 0;
    }
}

/* Registers the vsync, main and idle servers with the SVM (helper and name inferred). Its
 * out-of-line copy is emitted here, ahead of mwPlyInitSfdFx, which puts the server names ahead of
 * mwPlyInitSfdFx's messages in .rodata as in the original; the copy is not linked. */
static void mwsflib_EntrySvr(void) {
    MWSFSVM_EntryIdVfunc(2, MWSFSVR_VsyncThrdProc, 0, "MWSFSVR_VsyncThrdProc");
    MWSFSVM_EntryMainFunc(MWSFSVR_MainThrdProc, 0, "MWSFSVR_MainThrdProc");
    MWSFSVM_EntryIdleFunc(MWSFSVR_IdleThrdProc, 0, "MWSFSVR_IdleThrdProc");
}

/* Library init: ADXT, stream joints, SFD core, load scheduler and SFX converter, then registers the
 * vsync, main and idle servers with the SVM. */
void mwPlyInitSfdFx(MWSFD_INIT_PRM *iprm) {
    MWSFD_INIT_PRM prm;
    MWSFD_INIT_PRM *pprm;
    MWSFD_LIBWORK *lw;

    if (iprm == 0) {
        MWSFSVM_Error("E1122611 mwPlyInitSfdFx: iprm is NULL.");
        return;
    }
    memset(&prm, 0, sizeof(prm));
    pprm = &prm;
    prm.vfreq = iprm->vfreq;
    prm.x04 = iprm->x04;
    prm.nfrm_pool = iprm->nfrm_pool;
    prm.x0c = iprm->x0c;
    prm.x10 = iprm->x10;
    prm.x14 = iprm->x14;
    prm.x18 = iprm->x18;
    prm.x1c = iprm->x1c;
    cri_verstr_ptr = mwsfd_ver_str;
    MWSFSVM_Init();
    prm.nfrm_pool -= 2;
    if (prm.nfrm_pool < 0) {
        prm.nfrm_pool = 0;
    }
    if (mwsfd_init_cnt == 0) {
        ADXT_Init();
        SJRBF_Init();
        SJMEM_Init();
        SJUNI_Init();
        if (MWSTM_InitStatic() != 0) {
            MWSFLIB_SetErrCode(-0x65);
            MWSFSVM_Error("E2005 mwPlyInitSfdFx: can't init GSC");
        }
        lw = &mwsfd_libwork;
        memset(lw, 0, sizeof(MWSFD_LIBWORK));
        MWSFSVR_SetMwsfdSvrFlg(0);
        lw->vsync_flg = 0;
        mwsflib_SetLibPrm(&mwsfd_libwork, pprm);
        lw->use_picusr = 1;
        lw->pause_bdr = 0;
        mwg_vcnt = 0;
        if (mwPlySfdInit((Sint32)(0.5f + prm.vfreq * 1000.0f)) != 0) {
            MWSFSVM_Error("ERR20010421A : mwPlyInitSfdFx");
        }
        mwsfd_init_flag = 1;
        MWSFD_SetCond(0, 0x1B, (Sint32)prm.vfreq);
        MWSFD_SetCond(0, 7, 1);
        LSC_Init();
        LSC_EntryErrFunc(mwsflib_LscErrFunc, 0);
        MWSFSFX_Init();
        mwsflib_EntrySvr();
    }
    mwsfd_init_cnt++;
}

/* Load scheduler errors are reported through MWSFSVM_Error. */
void mwsflib_LscErrFunc(void *obj, Char8 *msg) {
    MWSFSVM_Error(msg);
}

/* The library work (player handles, callbacks, frame pool setting). */
MWSFD_LIBWORK *MWSFLIB_GetLibWorkPtr(void) {
    return &mwsfd_libwork;
}
