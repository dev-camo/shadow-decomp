#ifndef GAME_UNKGLOBAL_8057D6A0_H
#define GAME_UNKGLOBAL_8057D6A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x150-byte object at 0x8057D6A0 in .bss (placeholder name; the original name and type are
 * unknown). More than 400 functions across the game read it. Only the words at 0x28, 0x90, 0xA8 and 0xAC are
 * mapped. */
struct UnkGlobal_8057D6A0 {
    /* 0x00 */ u8 field_0x0[0x28];
    /* 0x28 */ float field_0x28; /* Coaster multiplies it by 540 (meaning unknown) */
    /* 0x2C */ u8 field_0x2C[0x90 - 0x2C];
    /* 0x90 */ u32 field_0x90;
    /* 0x94 */ u8 field_0x94[0xA8 - 0x94];
    /* 0xA8 */ s32 field_0xA8;
    /* ThreatObjHolder compares it with 9 to pick its model table, FallBuildingHolder's hookA
     * with 12 and 5 to limit the building type, and GrindGadget's hookA with 13 to pick
     * its PTB file. */
    /* 0xAC */ s32 field_0xAC;
    /* 0xB0 */ u8 field_0xB0[0x150 - 0xB0];
};
CHECK_SIZEOF(UnkGlobal_8057D6A0, 0x150);

extern UnkGlobal_8057D6A0 lbl_8057D6A0;

#endif
