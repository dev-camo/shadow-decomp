#ifndef CRI_LSC_H
#define CRI_LSC_H

/* CRI LSC load scheduler (lsc.c, lsc_svr.c, lsc_ini.c, lsc_err.c, lsc_crs.c): queues file
 * ranges on one ADX stream controller. Layout from the RE4 GameCube decompilation; the
 * table stride (0x238) and the fields used here are confirmed by the matched code.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"
#include "CRI/adx_stm.h"

#define LSC_MAX_OBJ 32
#define LSC_MAX_ENTRY 16
#define LSC_ID_MAX 0x7FFFFFFF

/* LSC_OBJ.stat */
#define LSC_STAT_STOP 0
#define LSC_STAT_PREP 1
#define LSC_STAT_EXEC 2
#define LSC_STAT_ERROR 3

/* LSC_ENTRY.stat */
#define LSC_ENT_WAIT 0
#define LSC_ENT_LOADING 1
#define LSC_ENT_DONE 2

typedef struct {
    /* 0x00 */ Sint32 id;
    /* 0x04 */ Char8 *fname;
    /* 0x08 */ Sint32 fname_sum; /* byte sum of fname */
    /* 0x0C */ void *dir;
    /* 0x10 */ Sint32 ofst;
    /* 0x14 */ Sint32 nsct;
    /* 0x18 */ Sint32 stat;
    /* 0x1C */ Sint32 pos;
} LSC_ENTRY;

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 stat;
    /* 0x02 */ Sint8 stm_start;
    /* 0x03 */ Sint8 loop_flag;
    /* 0x04 */ Sint8 pause_flag;
    /* 0x05 */ Uint8 pad5[3];
    /* 0x08 */ SJ sj;
    /* 0x0C */ Sint32 pad0c;
    /* 0x10 */ Sint32 pad10;
    /* 0x14 */ Sint32 min_val; /* flow limit */
    /* 0x18 */ Sint32 bufsize;
    /* 0x1C */ Sint32 wr_idx;
    /* 0x20 */ Sint32 rd_idx;
    /* 0x24 */ Sint32 num_stm;
    /* 0x28 */ ADXSTM stm;
    /* 0x2C */ Sint32 cur_nsct;
    /* 0x30 */ Sint32 pad30;
    /* 0x34 */ Sint32 pad34;
    /* 0x38 */ LSC_ENTRY tbl[LSC_MAX_ENTRY];
} LSC_OBJ;

typedef LSC_OBJ *LSC;

#ifdef __cplusplus
extern "C" {
#endif

extern LSC_OBJ lsc_obj[LSC_MAX_OBJ];

/* lsc_ini.c */
void LSC_Init(void);
void LSC_Finish(void);

/* lsc.c */
void LSC_CallStatFunc(void);
void LSC_SetFlowLimit(LSC lsc, Sint32 min_val);
Sint32 LSC_GetNumStm(LSC lsc);
Sint32 LSC_GetStat(LSC lsc);
void LSC_ExecServer(void);
void LSC_Stop(LSC lsc);
Sint32 LSC_EntryFileRange(LSC lsc, Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);
void LSC_SetStmHndl(LSC lsc, ADXSTM stm);
void LSC_Destroy(LSC lsc);

/* lsc_svr.c */
void lsc_ExecHndl(LSC lsc);

/* lsc_err.c */
void LSC_CallErrFunc(Char8 *fmt, ...);
void LSC_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj);

/* lsc_crs.c */
void LSC_LockCrs(Sint32 *msk);
void LSC_UnlockCrs(Sint32 *msk);

#ifdef __cplusplus
}
#endif

#endif
