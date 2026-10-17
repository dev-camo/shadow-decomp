#ifndef CRI_DCT_H
#define CRI_DCT_H

/* CRI DCT library (inverse DCT kernels used by the Sofdec MPV decoder). */

#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IDCT (not yet decompiled) */
void DCT_FsriInitScanTbl(Sint8 *seq, Sint8 *scan);

/* dct_ver.c */
extern const Char8 DCT_version_str[];
const Char8 *DCT_GetVerStr(void);

#ifdef __cplusplus
}
#endif

#endif
