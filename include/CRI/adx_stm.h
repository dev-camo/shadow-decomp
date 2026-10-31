#ifndef CRI_ADX_STM_H
#define CRI_ADX_STM_H

/* CRI ADX stream controller (adx_stmc.c) functions used outside it. The handle stays opaque
 * here: only the fields a source file touches are laid out where it is defined.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

typedef struct ADXSTM_OBJ *ADXSTM;

/* ADXSTM_GetStat results */
#define ADXSTM_STAT_STOP 0
#define ADXSTM_STAT_PREP 1
#define ADXSTM_STAT_EXEC 2
#define ADXSTM_STAT_END 3
#define ADXSTM_STAT_ERROR 4

#ifdef __cplusplus
extern "C" {
#endif

Sint32 ADXSTM_Init(void);
void ADXSTM_Finish(void);
void ADXSTM_ExecServer(void);
/* adx_stmc.c: one CVFS server pass (name unknown) */
void fn_803BF934(void);
ADXSTM ADXSTM_Create(SJ sj, Sint32 mode);
void ADXSTM_Destroy(ADXSTM stm);
void ADXSTM_SetSj(ADXSTM stm, SJ sj);
void ADXSTM_SetPause(ADXSTM stm, Sint32 sw);
Sint32 ADXSTM_SetReqRdSize(ADXSTM stm, Sint32 nsct);
Sint32 ADXSTM_SetBufSize(ADXSTM stm, Sint32 min_nsct, Sint32 max_nsct);
void ADXSTM_SetEos(ADXSTM stm, Sint32 nsct);
void ADXSTM_EntryEosFunc(ADXSTM stm, void (*func)(void *obj), void *obj);
void ADXSTM_Stop(ADXSTM stm);
void ADXSTM_StopNw(ADXSTM stm);
Sint32 ADXSTM_Start2(ADXSTM stm, Sint32 nsct);
Sint32 ADXSTM_Start(ADXSTM stm);
Sint32 ADXSTM_Tell(ADXSTM stm);
Sint32 ADXSTM_Seek(ADXSTM stm, Sint32 pos);
Sint32 ADXSTM_GetStat(ADXSTM stm);
void ADXSTM_ReleaseFile(ADXSTM stm);
void ADXSTM_ReleaseFileNw(ADXSTM stm);
void ADXSTM_BindFile(ADXSTM stm, const Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);
void ADXSTM_BindFileNw(ADXSTM stm, const Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);

#ifdef __cplusplus
}
#endif

#endif
