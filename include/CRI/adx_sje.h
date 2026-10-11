#ifndef CRI_ADX_SJE_H
#define CRI_ADX_SJE_H

/* CRI ADX stream-joint encoder (adx_sje.c) entry points used by the SFA header insertion
 * (adx_insh.c). The encoder handle stays opaque. Signatures follow the RE4 decompilation.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

#ifdef __cplusplus
extern "C" {
#endif

void ADXSJE_Init(void);
void ADXSJE_Finish(void);
void *ADXSJE_Create(Sint32 nsj, SJ *sjs, SJ sjo);
void ADXSJE_Destroy(void *sje);
void ADXSJE_SetConfigSfa(void *sje, Sint32 a, Sint32 b, Sint32 c);
void ADXSJE_Start(void *sje);
void ADXSJE_Stop(void *sje);
void ADXSJE_ExecServer(void);

#ifdef __cplusplus
}
#endif

#endif
