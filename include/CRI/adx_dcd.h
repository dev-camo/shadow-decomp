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

/* adx_dcd.c: ADX header fields */
Sint32 ADX_DecodeInfoAinf(Uint8 *data, Sint32 len, Sint32 *ainfsiz, void *ainf, Sint16 *a,
                          Sint16 *b);
Sint32 ADX_DecodeInfoExLoop(Uint8 *data, Sint32 len, Sint32 *lptype, Sint16 *nloop, Sint16 *lpflg,
                            Sint32 *lpstart, Sint32 *lpstartofst, Sint32 *lpend, Sint32 *lpendofst);
Sint32 ADX_DecodeInfoExIdly(Uint8 *data, Sint32 len, Uint16 *idly, Uint16 *idly2);
Sint32 ADX_DecodeInfoExVer(Uint8 *data, Sint32 len, Uint8 *major, Uint8 *minor);
Sint32 ADX_DecodeInfoExADPCM2(Uint8 *data, Sint32 len, Uint16 *cutoff);
Sint32 ADX_DecodeInfo(Uint8 *data, Sint32 len, Sint16 *hdrlen, Sint8 *fmt, Sint8 *bps,
                      Sint8 *blksiz, Sint8 *nch, Sint32 *sfreq, Sint32 *nsmpl, Sint32 *blksmpl);

/* adx_dcd5.c: decode nfrm frames; scl is the running scale key */
Sint32 ADX_DecodeMono4(Sint8 *src, Sint32 nfrm, Sint16 *out, Sint16 *hist, Sint16 c1, Sint16 c2,
                       Sint16 *scl, Sint16 smul, Sint16 sadd);
Sint32 ADX_DecodeSte4(Sint8 *src, Sint32 nfrm, Sint16 *outl, Sint16 *histl, Sint16 *outr,
                      Sint16 *histr, Sint16 c1, Sint16 c2, Sint16 *scl, Sint16 smul, Sint16 sadd);

#ifdef __cplusplus
}
#endif

#endif
