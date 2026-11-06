#ifndef GAME_UNKGLOBAL_8057D6A0_H
#define GAME_UNKGLOBAL_8057D6A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The record that starts at 0xA8 of lbl_8057D6A0 (placeholder name): every caller of fn_802039C8
 * and fn_802039E8 passes the address of lbl_8057D6A0 plus 0xA8. Its size is not known: 0x24 only
 * covers the bytes up to 0x23 that fn_802039C8 and fn_802039E8 read. */
struct UnkRecord_802039C8 {
    /* Fan compares it with 0x1F5 (meaning unknown). */
    /* 0x00 */ s32 field_0x0;
    /* ThreatObjHolder compares it with 9 to pick its model table, FallBuildingHolder's hookA
     * with 12 and 5 to limit the building type, and GrindGadget's hookA with 13 to pick
     * its PTB file. */
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ u8 field_0x8[0xA - 0x8];
    /* When it is set, fn_80169BE4 also tests the vector fn_80010244 returns for index 1. */
    /* 0x0A */ u8 field_0xA;
    /* 0x0B */ u8 field_0xB[0x19 - 0xB];
    /* CityChaosEmerald and GoalRing (unless its colour is 7) set it to 1 in their collision
     * callbacks (meaning unknown). */
    /* 0x19 */ u8 field_0x19;
    /* 0x1A */ u8 field_0x1A[0x20 - 0x1A];
    /* While it is set, ElecSecurityCode's update returns true right after its model update (the
     * model's fn_80162FB0, called only when the model pointer is not null), skipping its state,
     * model matrix and part updates (meaning unknown). WarpHole's ManagerTask sets it through
     * fn_80203970 in its constructor and may clear it in its destructor. */
    /* 0x20 */ u8 field_0x20;
    /* 0x21 */ u8 field_0x21; /* fn_802039E8 tests it */
    /* 0x22 */ u8 field_0x22;
    /* 0x23 */ u8 field_0x23; /* fn_802039C8 returns it and fn_802039E8 tests it */
};
CHECK_SIZEOF(UnkRecord_802039C8, 0x24);

/* The 0x150-byte object at 0x8057D6A0 in .bss (placeholder name; the original name and type are
 * unknown). More than 400 functions across the game read it. Only the words at 0x18, 0x28, 0x90,
 * 0x94 and 0x98, the record at 0xA8 and the byte at 0x125 are mapped. */
struct UnkGlobal_8057D6A0 {
    /* 0x00 */ u8 field_0x0[0x18];
    /* RollCircle divides the vector it passes to an attacked object by it (meaning unknown). */
    /* 0x18 */ float field_0x18;
    /* 0x1C */ u8 field_0x1C[0x28 - 0x1C];
    /* 0x28 */ float field_0x28; /* Coaster multiplies it by 540 (meaning unknown) */
    /* 0x2C */ u8 field_0x2C[0x84 - 0x2C];
    /* While it is set, the SAVEPOINT gadget's CanSave returns false and its PauseWatch task clears
     * the gadget's flags 0 and 1; fn_801E2EDC clears it (meaning unknown). */
    /* 0x84 */ u8 field_0x84;
    /* 0x85 */ u8 field_0x85[0x90 - 0x85];
    /* 0x90 */ u32 field_0x90;
    /* The ELEC_BAR gadget adds it to the times of its bars' cycles, and SpringPanel's colour
     * cycle follows its fractional part (meaning unknown). */
    /* 0x94 */ float field_0x94;
    /* 0x98 */ float field_0x98; /* ElecFan and Fan pass it to cos (meaning unknown) */
    /* 0x9C */ u8 field_0x9C[0xA8 - 0x9C];
    /* 0xA8 */ UnkRecord_802039C8 field_0xA8;
    /* 0xCC */ u8 field_0xCC[0x125 - 0xCC];
    /* The Researcher and ElecAccessPanel gadgets pass 1 to fn_800740E4 when it is set, otherwise 2
     * (meaning unknown). */
    /* 0x125 */ u8 field_0x125;
    /* 0x126 */ u8 field_0x126[0x150 - 0x126];
};
CHECK_SIZEOF(UnkGlobal_8057D6A0, 0x150);

extern UnkGlobal_8057D6A0 lbl_8057D6A0;

/* fn_802039C8 returns the record's byte at 0x23 and fn_802039E8 whether its byte at 0x21 or 0x23
 * is nonzero (behaviour inferred from their bodies; constness inferred). */
extern "C" {
bool fn_802039C8(const UnkRecord_802039C8 *record);
bool fn_802039E8(const UnkRecord_802039C8 *record);
/* Sets (set true) or clears the record's field_0x20 and returns true when that changed it
 * (behaviour inferred from its body). */
bool fn_80203970(UnkRecord_802039C8 *record, bool set);
}

#endif
