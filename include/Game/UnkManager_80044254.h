#ifndef GAME_UNKMANAGER_80044254_H
#define GAME_UNKMANAGER_80044254_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The manager that fn_80044254 returns (placeholder name; the original class and function names
 * are unknown). fn_80044254 constructs it on first use, registers its destructor and caches its
 * address in a static pointer. Unless they skip (see below), fn_800446D0 sets the word at 0x14 to
 * 3 and fn_800447B8 sets it to 2; each then stores its float argument at 0xC and fills 40 colour
 * entries of 24 bytes with the bytes at 0x10-0x12 and an alpha of 255 times the float at 0x0
 * (behaviour inferred). Only the word at 0x14 is mapped; the size is unknown. */
class UnkManager_80044254 {
  public:
    /* 0x00 */ u8 field_0x0[0x14];
    /* 0x14 */ s32 field_0x14; /* WarpHole's ManagerTask waits for 1 here (meaning unknown) */
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkManager_80044254, 0x18);

extern "C" {
UnkManager_80044254 *fn_80044254(void);
/* Stores arg2 at 0x444. When arg3 is nonzero it does nothing more if the word at 0x14 holds 1 or
 * 3; when arg3 is 0 it also stores 0.0f at 0x0 and never skips. WarpHole passes 1 for both (meanings
 * unknown). */
void fn_800446D0(UnkManager_80044254 *manager, float value, int arg2, int arg3);
/* Stores 0 at 0x444. When arg2 is nonzero it does nothing more if the word at 0x14 holds 0 or 2;
 * when arg2 is 0 it also stores 1.0f at 0x0 and never skips. WarpHole passes 1 (meaning
 * unknown). */
void fn_800447B8(UnkManager_80044254 *manager, float value, int arg2);
/* Returns the word at 0x14 of fn_80044254's manager. */
s32 fn_80044E10(void);
}

#endif
