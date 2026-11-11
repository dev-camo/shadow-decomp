#ifndef GAME_FN_80204874_H
#define GAME_FN_80204874_H

#include "rw/rwcore.h"

extern "C" {
/* Passes the colour to fn_800445D0 on fn_80044254's manager, which copies its four bytes to 0x10 of
 * the manager and refills the manager's 40 colour entries (behaviour inferred from the bodies;
 * constness inferred). */
void fn_80204874(const RwRGBA *color);
/* Stores value at 0x4 of fn_80044254's manager and 1.0f / value at 0x8 (behaviour inferred from
 * its body). */
void fn_802051F8(float value);
}

#endif
