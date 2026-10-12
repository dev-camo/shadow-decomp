#ifndef CRI_UTY_H
#define CRI_UTY_H

/* CRI utility library (UTY) routines shared by the CRI middleware. */

#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* memcpyd.c: copies ndw 32-bit words (the body works on Uint32 words with an unsigned count) */
void UTY_MemcpyDword(Uint32 *dst, Uint32 *src, Uint32 ndw);

/* memsetd.c: fills ndw 32-bit words at dst with val */
void UTY_MemsetDword(void *dst, Uint32 val, Uint32 ndw);

/* muldiv.c: a * b / c with a 64-bit intermediate */
Sint32 UTY_MulDiv(Sint32 a, Sint32 b, Sint32 c);

/* cmptime.c: compares cnt1/tscl1 with cnt2/tscl2 */
Bool UTY_CmpTime(Sint32 cnt1, Sint32 tscl1, Sint32 cnt2, Sint32 tscl2);

/* mcp_not.c: the portable memory copy over memcpy */
void MEM_Copy(void *dst, const void *src, Uint32 nbytes);

#ifdef __cplusplus
}
#endif

#endif
