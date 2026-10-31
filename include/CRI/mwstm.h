#ifndef CRI_MWSTM_H
#define CRI_MWSTM_H

/* CRI Sofdec MW player stream-controller glue (mwstm.c). Signatures follow the RE4
 * decompilation. */
#include "CRI/adx_stm.h"

#ifdef __cplusplus
extern "C" {
#endif

Sint32 MWSTM_GetStat(ADXSTM stm);
void MWSTM_ReqStop(ADXSTM stm);
Sint32 MWSTM_ReqStart(ADXSTM stm);
void MWSTM_SetFileRange(ADXSTM stm, const Char8 *fname, void *dir, Sint32 ofst, Sint32 nsct);
void MWSTM_Destroy(ADXSTM stm);
ADXSTM MWSTM_Create(SJ sj);
void MWSTM_SetFlowLimit(ADXSTM stm, Sint32 min_nsct, Sint32 max_nsct);
Bool MWSTM_IsFsStatErr(ADXSTM stm);
Sint32 MWSTM_FinishStatic(void);
Sint32 MWSTM_InitStatic(void);

#ifdef __cplusplus
}
#endif

#endif
