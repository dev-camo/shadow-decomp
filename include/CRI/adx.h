#ifndef CRI_ADX_H
#define CRI_ADX_H

/* CRI ADX library services shared by the ADX modules: the critical section (adx_crs.c), the
 * vsync wait (adx_mgc.c) and error reporting (adx_errs.c).
 */
#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* adx_crs.c */
void ADXCRS_Init(void);
void ADXCRS_Finish(void);
void ADXCRS_Lock(void);
void ADXCRS_Unlock(void);
void ADXCRS_Enter(void);
void ADXCRS_Leave(void);

/* adx_mgc.c */
void ADXM_WaitVsync(void);

/* adx_errs.c */
void ADXERR_ItoA2(Sint32 a, Sint32 b, Char8 *str, Sint32 len);
void ADXERR_CallErrFunc2(const Char8 *msg1, const Char8 *msg2);
void ADXERR_CallErrFunc1(const Char8 *msg);
void ADXERR_Finish(void);
void ADXERR_Init(void);

#ifdef __cplusplus
}
#endif

#endif
