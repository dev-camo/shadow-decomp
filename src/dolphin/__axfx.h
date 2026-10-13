#ifndef _DOLPHIN_AXFX_INTERNAL_H_
#define _DOLPHIN_AXFX_INTERNAL_H_

#include <dolphin/axfx.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *(*__AXFXAlloc)(u32);
extern void (*__AXFXFree)(void *);

#ifdef __cplusplus
}
#endif

#endif
