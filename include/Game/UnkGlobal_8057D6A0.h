#ifndef GAME_UNKGLOBAL_8057D6A0_H
#define GAME_UNKGLOBAL_8057D6A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x150-byte object at 0x8057D6A0 in .bss (placeholder name; the original name and type are
 * unknown). More than 400 functions across the game read it. Only the words at 0x18, 0x28, 0x90,
 * 0x94, 0x98, 0xA8 and 0xAC and the bytes at 0xB2, 0xC1 and 0x125 are mapped. */
struct UnkGlobal_8057D6A0 {
    /* 0x00 */ u8 field_0x0[0x18];
    /* RollCircle divides the vector it passes to an attacked object by it (meaning unknown). */
    /* 0x18 */ float field_0x18;
    /* 0x1C */ u8 field_0x1C[0x28 - 0x1C];
    /* 0x28 */ float field_0x28; /* Coaster multiplies it by 540 (meaning unknown) */
    /* 0x2C */ u8 field_0x2C[0x90 - 0x2C];
    /* 0x90 */ u32 field_0x90;
    /* The ELEC_BAR gadget adds it to the times of its bars' cycles (meaning unknown). */
    /* 0x94 */ float field_0x94;
    /* 0x98 */ float field_0x98; /* ElecFan and Fan pass it to cos (meaning unknown) */
    /* 0x9C */ u8 field_0x9C[0xA8 - 0x9C];
    /* Fan compares it with 0x1F5 (meaning unknown). */
    /* 0xA8 */ s32 field_0xA8;
    /* ThreatObjHolder compares it with 9 to pick its model table, FallBuildingHolder's hookA
     * with 12 and 5 to limit the building type, and GrindGadget's hookA with 13 to pick
     * its PTB file. */
    /* 0xAC */ s32 field_0xAC;
    /* 0xB0 */ u8 field_0xB0[0xB2 - 0xB0];
    /* When it is set, fn_80169BE4 also tests the vector fn_80010244 returns for index 1. */
    /* 0xB2 */ u8 field_0xB2;
    /* 0xB3 */ u8 field_0xB3[0xC1 - 0xB3];
    /* CityChaosEmerald and GoalRing (unless its colour is 7) set it to 1 in their collision
     * callbacks (meaning unknown). */
    /* 0xC1 */ u8 field_0xC1;
    /* 0xC2 */ u8 field_0xC2[0x125 - 0xC2];
    /* The Researcher and ElecAccessPanel gadgets pass 1 to fn_800740E4 when it is set, otherwise 2
     * (meaning unknown). */
    /* 0x125 */ u8 field_0x125;
    /* 0x126 */ u8 field_0x126[0x150 - 0x126];
};
CHECK_SIZEOF(UnkGlobal_8057D6A0, 0x150);

extern UnkGlobal_8057D6A0 lbl_8057D6A0;

#endif
