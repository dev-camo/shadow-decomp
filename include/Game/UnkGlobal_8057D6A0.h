#ifndef GAME_UNKGLOBAL_8057D6A0_H
#define GAME_UNKGLOBAL_8057D6A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x150-byte object at 0x8057D6A0 in .bss (placeholder name; the original name and type are
 * unknown). More than 400 functions across the game read it. Only the word at 0xAC is mapped. */
struct UnkGlobal_8057D6A0 {
    /* 0x00 */ u8 field_0x0[0xAC];
    /* 0xAC */ s32 field_0xAC; /* ThreatObjHolder compares it with 9 to pick its model table */
    /* 0xB0 */ u8 field_0xB0[0x150 - 0xB0];
};
CHECK_SIZEOF(UnkGlobal_8057D6A0, 0x150);

extern UnkGlobal_8057D6A0 lbl_8057D6A0;

#endif
