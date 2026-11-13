#ifndef GAME_UNKOBJECT_800BADE8_H
#define GAME_UNKOBJECT_800BADE8_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The object whose address fn_800BADE8 returns from a static pointer (placeholder name; the
 * original class and function names are unknown). fn_801E3F24, the only function that sets the
 * pointer, allocates 0x58 bytes for it. Only the word at 0x30 and the byte at 0x4D are mapped. */
class UnkObject_800BADE8 {
  public:
    /* 0x00 */ u8 field_0x0[0x30];
    /* WarpHole's ManagerTask sets it to 0 while it takes a character's control; its destructor
     * sets it to 1, and its update does so when fn_802E5810 returns false. The SAVEPOINT gadget
     * sets it to 0 when it opens a SavePointSelectTask and to 1 in that task's cancel callback
     * (meaning unknown). */
    /* 0x30 */ s32 field_0x30;
    /* 0x34 */ u8 field_0x34[0x4D - 0x34];
    /* TGiPause's update sets it to 0 when the update of its folderPause or its folderStoryRoute
     * returns true, otherwise to 1 (meaning unknown). */
    /* 0x4D */ u8 field_0x4D;
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkObject_800BADE8, 0x50);

/* The static pointer that fn_800BADE8 returns (placeholder name; 0x805EF9F4 in the USA release),
 * null until fn_801E3F24 creates the object. TGiPause's update reads it directly. */
extern UnkObject_800BADE8 *lbl_805EF9F4;

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
