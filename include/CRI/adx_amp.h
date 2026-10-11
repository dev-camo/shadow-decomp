#ifndef CRI_ADX_AMP_H
#define CRI_ADX_AMP_H

/* CRI ADXAMP amplitude stage (adx_amp.c) entries used by the ADXT layer; the handle stays opaque. */
#include "CRI/cri_xpt.h"

typedef struct ADXAMP_OBJ *ADXAMP;

#ifdef __cplusplus
extern "C" {
#endif

void ADXAMP_SetSfreq(ADXAMP amp, Sint32 sfreq);
void ADXAMP_Stop(ADXAMP amp);
void ADXAMP_Start(ADXAMP amp);
void ADXAMP_Destroy(ADXAMP amp);

#ifdef __cplusplus
}
#endif

#endif
