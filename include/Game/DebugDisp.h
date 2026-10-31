#ifndef GAME_DEBUGDISP_H
#define GAME_DEBUGDISP_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* Four bytes that the SonicteamUSA::System::DebugDispBase constructor copies one by one into the
 * shape; most likely a colour (inferred). fn_802E0E84 picks a queue by comparing the first byte
 * with 0xFF. */
struct UnkDebugColor_802E0E84 {
    /* Sets the four bytes in order (fn_8040D2D8); the body is not visible to the compiler, as in
     * the original. */
    UnkDebugColor_802E0E84(u8 b0, u8 b1, u8 b2, u8 b3);

    /* 0x0 */ u8 field_0x0;
    /* 0x1 */ u8 field_0x1;
    /* 0x2 */ u8 field_0x2;
    /* 0x3 */ u8 field_0x3;
};
CHECK_SIZEOF(UnkDebugColor_802E0E84, 0x4);

extern "C" {
/* Creates a SonicteamUSA::System::DebugDispSphere (class name from RTTI) of the given radius at
 * center and queues it (behaviour inferred; the original function name is unknown). The colour is
 * passed by reference, so a caller may build it in a temporary at the call. */
void fn_802E0E84(const Fn8040Vector3 *center, const UnkDebugColor_802E0E84 &color, float radius);
/* Creates a SonicteamUSA::System::DebugDispLine (class name from RTTI) for the segment and queues
 * it (behaviour inferred). fn_802E0B58 below is its caller with 0.0f (the float at 0x805F90F8); the
 * float is handed on to the line's constructor, so it is probably a line property (not
 * understood). */
void fn_802E0AA0(const UnkSegment_80020A20 &segment, const UnkDebugColor_802E0E84 &color,
                 float arg2);
/* The same with the fixed float (behaviour inferred). */
void fn_802E0B58(const UnkSegment_80020A20 &segment, const UnkDebugColor_802E0E84 &color);
/* Creates a SonicteamUSA::System::DebugDispCylinder (class name from RTTI) at center with the
 * given rotation (radians), colour, flag (0 in the known callers), half length and radius and
 * queues it (behaviour inferred). */
void fn_802E0B7C(const Fn8040Vector3 *center, const Fn8040Vector3 *rotation,
                 const UnkDebugColor_802E0E84 &color, int arg3, float halfLength, float radius);
/* Forwards to fn_802E0C88 (the same family as fn_802E0B7C) with the colour lbl_805F06D8 and arg2 as
 * its integer flag (behaviour inferred from the 0x28-byte wrapper; the original function name is
 * unknown). */
void fn_802E0D6C(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation, int arg2, float arg3,
                 float arg4);
}

/* A zero-filled .sbss colour that several gadgets pass when they draw their range (purpose
 * unknown). */
extern UnkDebugColor_802E0E84 lbl_805F06D8;

/* A .sbss colour that the startup code at 0x8040D304 sets to the bytes FF 00 FF 00 (purpose
 * unknown); the Coaster gadget draws its debug line with it. */
extern UnkDebugColor_802E0E84 lbl_805F1370;

#endif
