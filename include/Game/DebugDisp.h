#ifndef GAME_DEBUGDISP_H
#define GAME_DEBUGDISP_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* Four bytes that the SonicteamUSA::System::DebugDispBase constructor copies one by one into the
 * shape; most likely a colour (inferred). fn_802E0E84 picks a queue by comparing the first byte
 * with 0xFF. */
struct UnkDebugColor_802E0E84 {
    /* 0x0 */ u8 field_0x0;
    /* 0x1 */ u8 field_0x1;
    /* 0x2 */ u8 field_0x2;
    /* 0x3 */ u8 field_0x3;
};
CHECK_SIZEOF(UnkDebugColor_802E0E84, 0x4);

extern "C" {
/* Creates a SonicteamUSA::System::DebugDispSphere (class name from RTTI) of the given radius at
 * center and queues it (behaviour inferred; the original function name is unknown). */
void fn_802E0E84(const Fn8040Vector3 *center, const UnkDebugColor_802E0E84 *color, float radius);
}

/* A zero-filled .sbss colour that several gadgets pass when they draw their range (purpose
 * unknown). */
extern UnkDebugColor_802E0E84 lbl_805F06D8;

#endif
