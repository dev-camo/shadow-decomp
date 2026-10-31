#ifndef CRI_MWSFDSVM_H
#define CRI_MWSFDSVM_H

/* CRI Sofdec MW player server-manager glue (mwsfdsvm.c). */
#include "CRI/cri_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

void MWSFSVM_Error(const Char8 *fmt, ...);
Sint32 MWSFSVM_TestAndSet(Sint32 *flag);
void fn_804031D4(void);
void MWSFSVM_EntryMainFunc(Sint32 (*func)(void *obj), void *obj, const Char8 *name);
void fn_80403244(void);
void MWSFSVM_EntryIdleFunc(Sint32 (*func)(void *obj), void *obj, const Char8 *name);
void fn_804032B4(void);
void MWSFSVM_EntryIdVfunc(Sint32 id, Sint32 (*func)(void *obj), void *obj, const Char8 *name);
void fn_80403334(void);
void MWSFSVM_Init(void);

#ifdef __cplusplus
}
#endif

#endif
