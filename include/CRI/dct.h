#ifndef CRI_DCT_H
#define CRI_DCT_H

/* CRI DCT library (inverse DCT kernels used by the Sofdec MPV decoder). */

#include "CRI/cri_xpt.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IDCT parameter block of an MPV handle (MPV_OBJ + 0x78). DCT_FsriInitPa clears its 0x54 bytes
 * and the IDCT entry points installed in the handle receive its address. */
typedef struct {
    Sint8 cbp[6];             /* 0x00 per block: 0 when only the DC coefficient is coded */
    Uint8 pad06[0x18 - 0x06]; /* 0x06 */
    Sint32 cnt0;              /* 0x18 read with cnt1 by MPV_GetDctCnt */
    Sint32 cnt1;              /* 0x1C */
    Uint8 pad20[0x28 - 0x20]; /* 0x20 */
    Sint32 cbp_msk;           /* 0x28 coded block pattern, block 0 in the sign bit */
    Float32 *blk;             /* 0x2C six 8x8 coefficient blocks */
    Sint16 **tbl;             /* 0x30 output blocks (signed 16-bit, stored through GQR7) */
    Uint8 pad34[0x48 - 0x34]; /* 0x34 */
    Uint8 *work;              /* 0x48 */
    Uint8 pad4C[0x54 - 0x4C]; /* 0x4C */
} DCT_PA;
CHECK_SIZEOF(DCT_PA, 0x54);

/* dct_fsri.c */
void DCT_FsriTransCbp(DCT_PA *pa);
void DCT_FsriTrans6Blk(DCT_PA *pa);
void DCT_FsriInitScanTbl(const Sint8 *seq, Sint8 *scan);
void DCT_FsriSetGqr(void);
void DCT_FsriInitPa(DCT_PA *pa);
void DCT_FsriInitScaleTbl(Float32 *tbl);
void DCT_FsriInit(void);

/* dct_ac.c (not yet decompiled): double-precision reference transform */
void DCT_AcInit(void);
void DCT_AcIdctDouble(Float64 *in, Float64 *out);

/* dct_ver.c */
extern const Char8 DCT_version_str[];
const Char8 *DCT_GetVerStr(void);

#ifdef __cplusplus
}
#endif

#endif
