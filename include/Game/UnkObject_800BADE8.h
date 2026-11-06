#ifndef GAME_UNKOBJECT_800BADE8_H
#define GAME_UNKOBJECT_800BADE8_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The object whose address fn_800BADE8 returns from a static pointer (placeholder name; the
 * original class and function names are unknown). Only the word at 0x30 is mapped; the size is
 * unknown. */
class UnkObject_800BADE8 {
  public:
    /* 0x00 */ u8 field_0x0[0x30];
    /* WarpHole's ManagerTask sets it to 0 while it takes a character's control; its destructor
     * sets it to 1, and its update does so when fn_802E5810 returns false (meaning unknown). */
    /* 0x30 */ s32 field_0x30;
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkObject_800BADE8, 0x34);

extern "C" {
UnkObject_800BADE8 *fn_800BADE8(void);
/* Clears byte 0x180A of the object fn_8003A840 returns and, when the byte at 0x4C is set, plays
 * sound 0xE001 through fn_801D560C and clears the byte (behaviour inferred from its body). */
void fn_801E2F28(UnkObject_800BADE8 *object);
/* Stores 2 at 0x48, clears byte 0x84 of lbl_8057D6A0, calls fn_801D51EC with the sound manager
 * and clears byte 0xB5 of lbl_8057D6A0 (behaviour inferred from its body). */
void fn_801E2EDC(UnkObject_800BADE8 *object);
}

#endif
