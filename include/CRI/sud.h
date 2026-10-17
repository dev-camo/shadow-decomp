#ifndef CRI_SUD_H
#define CRI_SUD_H

/* CRI SUD: reader for the "<SUDPS_>" record the Sofdec encoder writes into each picture's user
 * data (sud_lib.c).
 */

#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

void SUD_SearchSudDat(Uint8 *dat, Sint32 size, void **sud, Sint32 *sudsize);
Bool SUD_AnalyTypeCcs(Uint8 *dat, Sint32 size);
Bool SUD_AnalyTypeDivField(Uint8 *dat, Sint32 size);
void SUD_Finish(void); /* name inferred */
void SUD_Init(void);

#ifdef __cplusplus
}
#endif

#endif
