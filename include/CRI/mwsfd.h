#ifndef CRI_MWSFD_H
#define CRI_MWSFD_H

/* CRI Sofdec MW player (MWSFD) internals shared by the mwsfd* units. Layouts follow the RE4
 * GameCube decompilation with this version's offsets, which differ: only fields confirmed by the
 * matched code are named. */
#include "CRI/adx_stm.h"
#include "CRI/cft.h"
#include "CRI/cri_xpt.h"
#include "CRI/lsc.h"
#include "CRI/sfd.h"
#include "CRI/sfx.h"
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
CHECK_SIZEOF(MWSST_OBJ, 0x18);

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
CHECK_SIZEOF(MWSST_IF, 0x30);

typedef struct {
    /* 0x00 */ MWSST_IF *ifc;
    /* 0x04 */ Sint32 cnt; /* number of created handles */
} MWSST_MNG;
CHECK_SIZEOF(MWSST_MNG, 0x8);

/* player states (MWPLY_OBJ.stat) */
#define MWSFD_STAT_STOP 0
#define MWSFD_STAT_PREP 1
#define MWSFD_STAT_PLAYING 2
#define MWSFD_STAT_PLAYEND 3
#define MWSFD_STAT_ERROR 4

/* Creation parameters of a player (the argument of mwPlyCreateSofdec, copied into the handle).
 * The names follow RE4's creation parameter record. The sourced code reads only mode, max_skip and
 * compo; the other fields' names are taken from that record and not yet checked against this
 * game's creation code. */
typedef struct {
    /* 0x00 */ Sint32 mode;      /* file type */
    /* 0x04 */ Sint32 max_bps;   /* maximum bit rate */
    /* 0x08 */ Sint32 max_width; /* maximum picture size */
    /* 0x0C */ Sint32 max_height;
    /* 0x10 */ Sint32 max_skip; /* frames mwPlyGetCurFrm may skip to catch up */
    /* 0x14 */ Sint32 nsec;     /* seconds of input buffering */
    /* 0x18 */ void *work;      /* work area for the component buffers */
    /* 0x1C */ Sint32 wksize;
    /* 0x20 */ Sint32 compo;  /* requested component layout */
    /* 0x24 */ Sint32 buffmt; /* output buffer format */
    /* 0x28 */ Sint32 x28;
    /* 0x2C */ Sint32 x2c;
} MWSFD_CRPRM;
CHECK_SIZEOF(MWSFD_CRPRM, 0x30);

/* What the Sofdec header callback (mwsffrm_AnalySofdecHeader) recorded about the video of one
 * header. */
typedef struct {
    /* 0x00 */ Sint32 valid;
    /* 0x04 */ Sint32 no;     /* header count when the header was seen */
    /* 0x08 */ Sint32 ccs;    /* colour type 3 */
    /* 0x0C */ Sint32 maxfrm; /* frame count, -1 when unknown */
    /* 0x10 */ Sint32 fxtype; /* SFX_COMPO_* of the video */
} MWSFFRM_SFHINF;
CHECK_SIZEOF(MWSFFRM_SFHINF, 0x14);

#define MWSFFRM_SFHINF_NUM 8

/* Picture user data buffer description (MWPLY_OBJ.picusr and what picusr_ptr points at). */
typedef struct {
    /* 0x00 */ void *buf;
    /* 0x04 */ Sint32 bsize;
    /* 0x08 */ Sint32 usize; /* bytes per picture */
} MWSFD_PICUSR;
CHECK_SIZEOF(MWSFD_PICUSR, 0xC);

/* The decoded frame mwPlyGetCurFrm hands to the application (0x90 bytes), converted from the
 * decoder's SFD_VFRM_INF by mwl_convFrmInfFromSFD; the notes name the source member. */
typedef struct {
    /* 0x00 */ void *bufadr;     /* buf */
    /* 0x04 */ Sint32 x04;       /* copy of time, made by mwPlyGetCurFrm */
    /* 0x08 */ Sint32 fmt;       /* x1c as 1, 2 or 3 */
    /* 0x0C */ Sint32 width;     /* width */
    /* 0x10 */ Sint32 height;    /* height */
    /* 0x14 */ Sint32 mb_width;  /* mb_width */
    /* 0x18 */ Sint32 mb_height; /* mb_height */
    /* 0x1C */ Sint32 ptype;     /* pic_type, 1 to 4 */
    /* 0x20 */ Sint32 fps;       /* the stream's frame rate x1000 */
    /* 0x24 */ Sint32 time;      /* x34 in frame periods */
    /* 0x28 */ Sint32 x34;       /* x34 */
    /* 0x2C */ Sint32 tunit;     /* tunit */
    /* 0x30 */ Sint32 gopno;     /* gopno: selects the Sofdec header entry (MWPLY_OBJ.sfh_cur) */
    /* 0x34 */ Sint32 time2;     /* x30 in frame periods */
    /* 0x38 */ Sint32 x30;       /* x30 */
    /* 0x3C */ Sint32 ndct;      /* ndct */
    /* 0x40 */ Sint32 nbyte;     /* nbyte */
    /* 0x44 */ Uint8 *usrdat;    /* picture user data after the 4-byte start, NULL for none */
    /* 0x48 */ Sint32 usrlen;
    /* 0x4C */ Sint32 ftype;    /* 2 when the picture is handled as fields, else 0 */
    /* 0x50 */ Uint8 ext[0x38]; /* copy of SFD_VFRM_INF from x48 on */
    /* 0x88 */ Uint8 *sudat;    /* the SUD record found in the picture user data */
    /* 0x8C */ Sint32 sulen;
} MWS_FRM;
CHECK_SIZEOF(MWS_FRM, 0x90);

/* MW player object (0x2A8 bytes; the libwork holds eight). Only the fields the matched units use
 * are named. */
struct MWPLY_OBJ {
    /* 0x000 */ Sint32 used; /* 1 for a created handle */
    /* 0x004 */ Sint32 stat; /* MWSFD_STAT_* */
    /* 0x008 */ MWSFD_CRPRM prm;
    /* 0x038 */ Sint32 x38;
    /* 0x03C */ SFD sfd;
    /* 0x040 */ ADXSTM stm;
    /* 0x044 */ Sint32 x44;
    /* 0x048 */ LSC lsc;
    /* 0x04C */ Uint8 x4c[8];
    /* 0x054 */ Sint32 noskip; /* never skip frames to catch up */
    /* 0x058 */ Uint8 x58[4];
    /* 0x05C */ Sint32 sleep_bdr;     /* sleeping at the SVM idle border */
    /* 0x060 */ Sint32 mwply_svr_flg; /* the handle's server step is running */
    /* 0x064 */ Sint32 sfd_svr_flg;   /* SFD_ExecOne is running for the handle */
    /* 0x068 */ Sint32 dec_svr_flg;   /* the player state machine is running */
    /* 0x06C */ Uint8 x6c[4];
    /* 0x070 */ Sint8 linkstm;     /* concatenated play is on */
    /* 0x071 */ Sint8 linkstm_req; /* end of the linked streams requested */
    /* 0x072 */ Sint8 pause_flg;
    /* 0x073 */ Uint8 x73;
    /* 0x074 */ Sint32 x74;
    /* 0x078 */ SFD_VFRM_INF *curfrm; /* the frame mwPlyGetCurFrm handed out last */
    /* 0x07C */ Sint32 ngetfrm;       /* frames taken */
    /* 0x080 */ Sint32 nrelfrm;       /* frames given back */
    /* 0x084 */ Sint32 nskipdisp;     /* frames skipped by mwPlyGetCurFrm */
    /* 0x088 */ Sint32 pic_struct;
    /* 0x08C */ Sint32 chroma_format;
    /* 0x090 */ Sint32 x90;
    /* 0x094 */ Sint32 x94;
    /* 0x098 */ Sint32 x98;
    /* 0x09C */ Sint32 chromapos_h;
    /* 0x0A0 */ Sint32 chromapos_v;
    /* 0x0A4 */ Sint32 xa4;
    /* 0x0A8 */ SFX_OBJ *sfx; /* the frame converter handle */
    /* 0x0AC */ Uint8 xac[8];
    /* 0x0B4 */ Sint32 sfh_cnt; /* Sofdec headers seen */
    /* 0x0B8 */ Sint32 sfh_cur; /* header number of the current frame */
    /* 0x0BC */ Sint32 sfh_wr;  /* next sfhinf slot */
    /* 0x0C0 */ MWSFFRM_SFHINF sfhinf[MWSFFRM_SFHINF_NUM];
    /* 0x160 */ MWSFD_PICUSR picusr; /* picusr_ptr points here for no user data */
    /* 0x16C */ Uint8 x16c[0x178 - 0x16C];
    /* 0x178 */ MWSFD_PICUSR *picusr_ptr;
    /* 0x17C */ Uint8 x17c[0x190 - 0x17C];
    /* 0x190 */ Sint32 tag_no; /* header number of the last tag update, -1 for none */
    /* 0x194 */ Uint8 x194[0x1A4 - 0x194];
    /* 0x1A4 */ const Char8 *fname;
    /* 0x1A8 */ Sint32 x1a8;
    /* 0x1AC */ Sint32 stm_start_req; /* start the file stream on the next server step */
    /* 0x1B0 */ void *dir;
    /* 0x1B4 */ Sint32 ofst;
    /* 0x1B8 */ Sint32 nsct;
    /* 0x1BC */ SJ sji;
    /* 0x1C0 */ Uint8 x1c0[0x1C8 - 0x1C0];
    /* 0x1C8 */ Sint32 flow_nsct; /* flow limit in bytes, although named for sectors */
    /* 0x1CC */ Uint8 x1cc[0x280 - 0x1CC];
    /* 0x280 */ MWSST_OBJ sst;
    /* 0x298 */ Uint8 x298[0x2A4 - 0x298];
    /* 0x2A4 */ Sint32 frmapi; /* get/release frame API in use: 1 mwPlyGetCurFrm/mwPlyRelCurFrm */
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
    /* 0x10 */ Sint32 main_decode; /* decode in the main server, not the idle one; inferred */
    /* 0x14 */ Uint8 x14[0x24 - 0x14];
    /* 0x24 */ Sint32 svr_bdr; /* a handle is sleeping at the idle border */
    /* 0x28 */ Uint8 x28[0x38 - 0x28];
    /* 0x38 */ Sint32 use_picusr;             /* picture user data is captured */
    /* 0x3C */ Sint32 pause_bdr;              /* pausing waits for the decode server border */
    /* 0x40 */ Sint32 (*pre_func)(void *obj); /* run before the decode server step */
    /* 0x44 */ void *pre_obj;
    /* 0x48 */ Sint32 (*post_func)(void *obj); /* run after it */
    /* 0x4C */ void *post_obj;
    /* 0x50 */ Sint32 (*idle_func)(void *obj); /* run when no handle has work */
    /* 0x54 */ void *idle_obj;
    /* 0x58 */ Sint32 svr_flg;   /* decode server running */
    /* 0x5C */ Sint32 vsync_flg; /* the vsync server step is running; inferred */
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
void mwSfdDestroy(MWPLY mwply);
/* Converts the frame frm into buf with cnvfn, which it registers with SFX_SetCnvFn (mwsfd internal,
 * not sourced). It reads the frame size from frm (offsets 0xC and 0x10) and passes buf to
 * sfxcnv_MakeDstFrm. The frame record's type is not yet known; the RE4 player's ARGB8888 entry
 * takes a frame record (MWS_FRM *) and an output buffer in these positions. */
void fn_804035A8(MWPLY mwply, void *frm, Uint8 *buf, SFX_CNVFN cnvfn);
void fn_80408680(MWPLY mwply, void *frm, Uint8 *buf); /* fn_80408680.c */
void MWSFTAG_UpdateTagInf(MWPLY mwply);
void MWSFSFX_Init(void);
void MWSFSFX_Finish(void); /* inferred name: the counterpart of MWSFSFX_Init */
void MWSFSVR_SetMwsfdSvrFlg(Sint32 flg);
void MWSFPLY_SetFlowLimit(MWPLY mwply);
void MWSFSVR_SetHnSfdSvrFlg(MWPLY mwply, Sint32 flg);
void MWSFSVR_SetHnMwplySvrFlg(MWPLY mwply, Sint32 flg);
void mwsfd_SetSleepBdr(MWPLY mwply, Sint32 sw); /* inferred name */
Sint32 MWSFSVR_VsyncThrdProc(void *obj);
Sint32 MWSFSVR_MainThrdProc(void *obj);
Sint32 MWSFSVR_IdleThrdProc(void *obj);

/* mwsfdfrm.c */
void MWSFFRM_SetShfCbFn(MWPLY mwply);
void MWSFFRM_InitSfhInfTable(MWPLY mwply);
Sint32 mwPlyIsNextFrmReady(MWPLY mwply);
void mwPlyRelCurFrm(MWPLY mwply);
void mwPlyCalcYccPlane(void *buf, Sint32 width, Sint32 height, CFT_YCC420PLN *out);
void mwPlyGetCurFrm(void *obj, void *frmobj);

/* mwsfdset.c */
SFD mwPlyGetSfdHn(MWPLY mwply);
void MWSFD_SetCond(MWPLY mwply, Sint32 id, Sint32 val);
void MWSFD_SetFlowLimit(MWPLY mwply, Sint32 min_nsct, Sint32 max_nsct);
Bool MWSFD_IsEnableHndl(MWPLY mwply);
Sint32 mwPlyGetStat(MWPLY mwply);
void mwPlySetOutVol(void *hn, Sint32 vol);
Sint32 mwPlyGetOutVol(void *hn);

/* other units of the player used by mwsfdsvr.c */
void MWSFCRE_SetSupplySj(MWPLY mwply);
void mwPlySfdStart(MWPLY mwply);
void mwPlyPause(MWPLY mwply, Sint32 sw);
void mwPlyChkSupply(MWPLY mwply);

/* mwsfdsvr.c */
void mwlSfdSleepDecSvr(MWPLY mwply);
Sint32 mwPlyExecSvrHndl(MWPLY mwply);
Sint32 MWSFSVR_DecodeServer(void *obj);
Sint32 mwSfdExecDecSvrHndl(MWPLY mwply);
Sint32 mwSfdExecSvrHndl(MWPLY mwply);

/* mwsfdsee.c */
void MWSFSEE_ChkSupply(MWPLY mwply);

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
