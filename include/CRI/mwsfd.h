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

/* MW player object */
struct MWPLY_OBJ {
    /* 0x00 */ Uint8 x00[0x3C];
    /* 0x3C */ SFD sfd;
    /* 0x40 */ ADXSTM stm;
    /* 0x44 */ Sint32 x44;
    /* 0x48 */ LSC lsc;
    /* 0x4C */ Uint8 x4c[0x280 - 0x4C];
    /* 0x280 */ MWSST_OBJ sst;
};

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

/* mwsfdrna.c */
Sint32 MWSFRNA_GetOutVol(MWPLY mwply);
void MWSFRNA_SetOutVol(MWPLY mwply, Sint32 vol);

#ifdef __cplusplus
}
#endif

#endif
