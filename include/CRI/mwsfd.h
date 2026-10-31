#ifndef CRI_MWSFD_H
#define CRI_MWSFD_H

/* CRI Sofdec MW player (MWSFD) internals shared by the mwsfd* units. Layouts follow the RE4
 * GameCube decompilation with this version's offsets, which differ: only fields confirmed by the
 * matched code are named. */
#include "CRI/adx_stm.h"
#include "CRI/cri_xpt.h"
#include "CRI/lsc.h"
#include "CRI/sfd.h"
#include "CRI/sj.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MWPLY_OBJ MWPLY_OBJ;
typedef MWPLY_OBJ *MWPLY;

typedef struct MWSST_OBJ MWSST_OBJ;
typedef MWSST_OBJ *MWSST;

/* Side-stream (second audio stream) handle (mwsfdsst.c): the player embeds one, and its hn points
 * at a second one that stands for the core library's handle. */
struct MWSST_OBJ {
    /* 0x00 */ Sint32 used;
    /* 0x04 */ Sint32 x04;
    /* 0x08 */ Sint32 x08;
    /* 0x0C */ SJ sj;
    /* 0x10 */ Sint32 stmno; /* element stream number: the stream id is 0xC0 + stmno */
    /* 0x14 */ MWSST hn;
};

/* Core side-stream library interface (registered in mwsstmng.ifc). Only the slots the
 * wrappers call are named. */
typedef struct {
    /* 0x00 */ void *x00;
    /* 0x04 */ void (*Finish)(void);
    /* 0x08 */ void *x08;
    /* 0x0C */ void *x0c;
    /* 0x10 */ void (*Destroy)(MWSST hn);
    /* 0x14 */ void (*StartSj)(MWSST hn, SJ sj);
    /* 0x18 */ void (*Stop)(MWSST hn);
    /* 0x1C */ Sint32 (*GetStat)(MWSST hn);
    /* 0x20 */ void *x20;
    /* 0x24 */ void (*Pause)(MWSST hn, Sint32 sw);
    /* 0x28 */ void (*SetOutVol)(MWSST hn, Sint32 vol);
    /* 0x2C */ Sint32 (*GetOutVol)(MWSST hn);
} MWSST_IF;

typedef struct {
    /* 0x00 */ MWSST_IF *ifc;
    /* 0x04 */ Sint32 cnt; /* number of created handles */
} MWSST_MNG;

/* MW player object (0x2A8 bytes; the libwork holds eight) */
struct MWPLY_OBJ {
    /* 0x000 */ Sint32 used; /* 1 for a created handle */
    /* 0x004 */ Uint8 x04[0x38];
    /* 0x03C */ SFD sfd;
    /* 0x040 */ ADXSTM stm;
    /* 0x044 */ Sint32 x44;
    /* 0x048 */ LSC lsc;
    /* 0x04C */ Uint8 x4c[0x280 - 0x4C];
    /* 0x280 */ MWSST_OBJ sst;
    /* 0x298 */ Uint8 x298[0x2A8 - 0x298];
};
CHECK_SIZEOF(MWPLY_OBJ, 0x2A8);

#define MWSFD_MAX_HN 8

/* Parameters of mwPlyInitSfdFx. */
typedef struct {
    /* 0x00 */ Float32 vfreq;    /* video refresh rate in Hz */
    /* 0x04 */ Sint32 x04;       /* copied to MWSFD_LIBWORK.x08 */
    /* 0x08 */ Sint32 nfrm_pool; /* frame pool size; the library reserves two for itself */
    /* 0x0C */ Sint32 x0c;       /* copied to MWSFD_LIBWORK.main_decode */
    /* 0x10 */ Sint32 x10;
    /* 0x14 */ Sint32 x14;
    /* 0x18 */ Sint32 x18;
    /* 0x1C */ Sint32 x1c;
} MWSFD_INIT_PRM;
CHECK_SIZEOF(MWSFD_INIT_PRM, 0x20);

/* Library work (mwsfdlib.c, 0x15AC bytes): the player handles and the library-wide settings. */
typedef struct {
    /* 0x00 */ Sint32 x00;
    /* 0x04 */ Float32 vfreq;
    /* 0x08 */ Sint32 x08;
    /* 0x0C */ Sint32 nfrm_pool;
    /* 0x10 */ Sint32 x10;
    /* 0x14 */ Uint8 x14[0x24 - 0x14];
    /* 0x24 */ Sint32 svr_bdr; /* a handle is sleeping at the idle border */
    /* 0x28 */ Uint8 x28[0x38 - 0x28];
    /* 0x38 */ Sint32 use_picusr; /* picture user data is captured */
    /* 0x3C */ Sint32 pause_bdr;  /* pausing waits for the decode server border */
    /* 0x40 */ Uint8 x40[0x58 - 0x40];
    /* 0x58 */ Sint32 svr_flg; /* decode server running */
    /* 0x5C */ Sint32 x5c;
    /* 0x60 */ Uint8 x60[0x68 - 0x60];
    /* 0x68 */ Sint32 errcode; /* last library error */
    /* 0x6C */ MWPLY_OBJ hn[MWSFD_MAX_HN];
} MWSFD_LIBWORK;
CHECK_SIZEOF(MWSFD_LIBWORK, 0x15AC);

extern MWSFD_LIBWORK mwsfd_libwork;
extern Sint32 mwsfd_init_flag;
extern Sint32 mwg_vcnt;

/* mwsfdsst.c */
extern MWSST_MNG mwsstmng;
void MWSST_Destroy(MWSST sst);
void MWSST_Reset(MWPLY mwply);
Sint32 MWSST_GetOutVol(MWSST sst);
void MWSST_SetOutVol(MWSST sst, Sint32 vol);
void MWSST_Pause(MWSST sst, Sint32 sw);
Sint32 MWSST_GetStat(MWSST sst);
void MWSST_Stop(MWSST sst);
void MWSST_StartSj(MWSST sst);

/* mwsfdlib.c */
void mwPlyFinishSfdFx(void);
void MWSFLIB_SfdErrFunc(void *obj, Sint32 code);
Sint32 MWSFLIB_SetErrCode(Sint32 code);
Sint32 MWSFD_GetPauseBdr(void);
Sint32 MWSFD_GetUsePicUsr(void);
void mwPlyInitSfdFx(MWSFD_INIT_PRM *iprm);
void mwsflib_LscErrFunc(void *obj, Char8 *msg);
MWSFD_LIBWORK *MWSFLIB_GetLibWorkPtr(void);

/* other units of the player used by mwsfdlib.c */
void MWSFD_SetCond(MWPLY mwply, Sint32 id, Sint32 val);
SFD mwPlyGetSfdHn(MWPLY mwply);
void mwSfdDestroy(MWPLY mwply);
void MWSFSFX_Init(void);
void MWSFSFX_Finish(void); /* inferred name: the counterpart of MWSFSFX_Init */
void MWSFSVR_SetMwsfdSvrFlg(Sint32 flg);
void mwsfd_SetSleepBdr(MWPLY mwply, Sint32 sw); /* inferred name */
Sint32 MWSFSVR_VsyncThrdProc(void *obj);
Sint32 MWSFSVR_MainThrdProc(void *obj);
Sint32 MWSFSVR_IdleThrdProc(void *obj);

/* mwsfdsl.c */
void MWSFLSC_SetFlowLimit(MWPLY mwply, Sint32 nsct);
Bool MWSFLSC_IsFsStatErr(LSC lsc);

/* mwsfdrna.c */
Sint32 MWSFRNA_GetOutVol(MWPLY mwply);
void MWSFRNA_SetOutVol(MWPLY mwply, Sint32 vol);

#ifdef __cplusplus
}
#endif

#endif
