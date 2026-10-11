#ifndef CRI_ADX_DCD_H
#define CRI_ADX_DCD_H

/* CRI ADX 4-bit ADPCM decoder routines (adx_dcd.c, adx_dcd5.c) used by the block expander.
 * Signatures follow the RE4 decompilation.
 */
#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* adx_dcd.c: prediction coefficients for an encoder cut-off frequency */
void ADX_GetCoefficient(Sint32 cutoff, Sint32 sfreq, Sint16 *c1, Sint16 *c2);

/* adx_dcd5.c: decode nfrm frames; scl is the running scale key */
Sint32 ADX_DecodeMono4(Sint8 *src, Sint32 nfrm, Sint16 *out, Sint16 *hist, Sint16 c1, Sint16 c2,
                       Sint16 *scl, Sint16 smul, Sint16 sadd);
Sint32 ADX_DecodeSte4(Sint8 *src, Sint32 nfrm, Sint16 *outl, Sint16 *histl, Sint16 *outr,
                      Sint16 *histr, Sint16 c1, Sint16 c2, Sint16 *scl, Sint16 smul, Sint16 sadd);

#ifdef __cplusplus
}
#endif

#endif
