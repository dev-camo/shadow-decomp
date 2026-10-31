#ifndef CRI_MWSFD_H
#define CRI_MWSFD_H

/* CRI Sofdec MW player (MWSFD) internals shared by the mwsfd* units. Layouts follow the RE4
 * GameCube decompilation with this version's offsets, which differ: only fields confirmed by the
 * matched code are named. */
#include "CRI/adx_stm.h"
#include "CRI/cri_xpt.h"
#include "CRI/lsc.h"
#include "CRI/sfd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MWPLY_OBJ MWPLY_OBJ;
typedef MWPLY_OBJ *MWPLY;

/* MW player object */
struct MWPLY_OBJ {
    /* 0x00 */ Uint8 x00[0x3C];
    /* 0x3C */ SFD sfd;
    /* 0x40 */ ADXSTM stm;
    /* 0x44 */ Sint32 x44;
    /* 0x48 */ LSC lsc;
};

/* mwsfdrna.c */
Sint32 MWSFRNA_GetOutVol(MWPLY mwply);
void MWSFRNA_SetOutVol(MWPLY mwply, Sint32 vol);

#ifdef __cplusplus
}
#endif

#endif
