#ifndef CRI_UTY_H
#define CRI_UTY_H

/* CRI utility library (UTY) routines shared by the CRI middleware. */

#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* memcpyd.c: copies ndw 32-bit words (the body works on Uint32 words with an unsigned count) */
void UTY_MemcpyDword(Uint32 *dst, Uint32 *src, Uint32 ndw);

#ifdef __cplusplus
}
#endif

#endif
