#ifndef _DOLPHIN_EXI_INTERNAL_H_
#define _DOLPHIN_EXI_INTERNAL_H_

#include <dolphin/exi.h>

#ifdef __cplusplus
extern "C" {
#endif

// State bits and masks of each EXI channel, kept by EXIBios and returned by EXIGetState.
#define EXI_STATE_IDLE 0
#define EXI_STATE_DMA 1
#define EXI_STATE_IMM 2
#define EXI_STATE_BUSY 3
#define EXI_STATE_SELECTED 4
#define EXI_STATE_ATTACHED 8
#define EXI_STATE_LOCKED 16

// EXIBios
u32 EXIClearInterrupts(s32 chan, int exi, int tc, int ext);

// EXIUart
void __OSEnableBarnacle(s32 chan, u32 dev);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_EXI_INTERNAL_H_
