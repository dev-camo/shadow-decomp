#ifndef CRI_ADX_STM_H
#define CRI_ADX_STM_H

/* CRI ADX stream controller (adx_stmc.c) functions used outside it. The handle stays opaque
 * here: only the fields a source file touches are laid out where it is defined.
 */
#include "CRI/cri_xpt.h"

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

Sint32 ADXSTM_SetBufSize(ADXSTM stm, Sint32 min_nsct, Sint32 max_nsct);
void ADXSTM_SetEos(ADXSTM stm, Sint32 nsct);
void ADXSTM_StopNw(ADXSTM stm);
Sint32 ADXSTM_Start(ADXSTM stm);
Sint32 ADXSTM_Tell(ADXSTM stm);
Sint32 ADXSTM_Seek(ADXSTM stm, Sint32 pos);
Sint32 ADXSTM_GetStat(ADXSTM stm);
void ADXSTM_ReleaseFileNw(ADXSTM stm);
void ADXSTM_BindFileNw(ADXSTM stm, const Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);

#ifdef __cplusplus
}
#endif

#endif
