#ifndef CRI_UTY_H
#define CRI_UTY_H

/* CRI utility library (UTY) routines shared by the CRI middleware. */

#include "CRI/cri_xpt.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

/* uty_ppc.c: save area for the graphics quantisation registers; UTY_PushGqr and UTY_PopGqr keep
 * GQR2..GQR7 at the index of their register number (GQR0 and GQR1 are not touched) */
typedef struct {
    Uint32 gqr[8]; /* 0x00 */
} UTY_GQR;
CHECK_SIZEOF(UTY_GQR, 0x20);

void UTY_PopGqr(UTY_GQR *gqr);
void UTY_PushGqr(UTY_GQR *gqr);

/* memcpyd.c: copies ndw 32-bit words (the body works on Uint32 words with an unsigned count) */
void UTY_MemcpyDword(Uint32 *dst, Uint32 *src, Uint32 ndw);

/* memsetd.c: fills ndw 32-bit words at dst with val */
void UTY_MemsetDword(void *dst, Uint32 val, Uint32 ndw);

/* muldiv.c: a * b / c with a 64-bit intermediate */
Sint32 UTY_MulDiv(Sint32 a, Sint32 b, Sint32 c);

/* muldivr.c: 64-bit a * b / c rounded to nearest */
Sint64 UTY_MulDivRound64(Sint64 a, Sint64 b, Sint64 c);

/* cmptime.c: compares cnt1/tscl1 with cnt2/tscl2 */
Bool UTY_CmpTime(Sint32 cnt1, Sint32 tscl1, Sint32 cnt2, Sint32 tscl2);

/* uty_tmr.c: the PowerPC time base as the profiling clock */
Sint64 UTY_GetTmrUnit(void);
Bool UTY_IsTmrVoid(void);
Sint64 UTY_GetTmr(void);
void UTY_FinishTmr(void);
void UTY_InitTmr(Sint32 ch);

/* Game/fn_803CC9D0.c: looks a configuration entry up by key (the table and the name are unknown) */
int fn_803CC9D0(const char *key, unsigned int *output_word);

/* mcp_not.c: the portable memory copy over memcpy */
void MEM_Copy(void *dst, const void *src, Uint32 nbytes);

#ifdef __cplusplus
}
#endif

#endif
