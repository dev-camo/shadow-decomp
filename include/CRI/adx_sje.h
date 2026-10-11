#ifndef CRI_ADX_SJE_H
#define CRI_ADX_SJE_H

/* CRI ADX stream-joint encoder (adx_sje.c) entry points used by the SFA header insertion
 * (adx_insh.c). The encoder handle stays opaque outside adx_sje.c. Signatures follow the RE4
 * decompilation.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"

typedef struct ADXSJE_OBJ *ADXSJE;

#ifdef __cplusplus
extern "C" {
#endif

void ADXSJE_Init(void);
void ADXSJE_Finish(void);
ADXSJE ADXSJE_Create(Sint32 nch, SJ *sji, SJ sjo);
void ADXSJE_Destroy(ADXSJE sje);
void ADXSJE_SetConfigSfa(ADXSJE sje, Sint32 nch, Sint32 sfreq, Sint32 nsmpl);
void ADXSJE_Start(ADXSJE sje);
void ADXSJE_Stop(ADXSJE sje);
void ADXSJE_ExecServer(void);
void ADXSJE_ExecHndl(void *obj);

#ifdef __cplusplus
}
#endif

#endif
