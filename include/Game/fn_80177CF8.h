#ifndef GAME_FN_80177CF8_H
#define GAME_FN_80177CF8_H

#include <dolphin/types.h>

extern "C" {
/* Returns the byte at 0x14 of the object that the pointer at 0x805EF320 holds, or 0 when that
 * pointer is null (behaviour from its body; what the object and the byte are is unknown). The
 * BKTankController::Controller task waits until it is 1. */
u8 fn_80177CF8(void);
}

#endif
