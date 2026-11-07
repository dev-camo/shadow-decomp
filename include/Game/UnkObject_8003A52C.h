#ifndef GAME_UNKOBJECT_8003A52C_H
#define GAME_UNKOBJECT_8003A52C_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The objects whose addresses lbl_8051CE88 holds (placeholder name; the original class name is
 * unknown). Only the word at 0x40 and the byte at 0x978 are mapped; the size is unknown. */
class UnkObject_8003A52C {
  public:
    /* 0x000 */ u8 field_0x0[0x40];
    /* WarpHole's ManagerTask stores 2 here and waits for it to be 0 (meaning unknown). */
    /* 0x040 */ s32 field_0x40;
    /* 0x044 */ u8 field_0x44[0x978 - 0x44];
    /* WarpHole's ManagerTask also waits for it to be 0 (meaning unknown). */
    /* 0x978 */ u8 field_0x978;
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkObject_8003A52C, 0x97C);

/* Four object pointers in .data (placeholder name and inferred element type; other functions
 * index it). WarpHole's ManagerTask reads only the first. */
extern UnkObject_8003A52C *lbl_8051CE88[4];

/* Stores 1 in the byte at 0x38 and 0 in the byte at 0x979 of object (behaviour inferred from its
 * body; the original name is unknown). */
extern "C" void fn_8003A52C(UnkObject_8003A52C *object);

/* Stores value in the word at 0x2E8 + 4 * index of object (behaviour inferred from its body; the
 * original name is unknown, and the parameter types and the void result are inferred). The
 * BlockChange gadget passes its "BLOCK" values as index and 1 or 2 as value. */
extern "C" void fn_8003AAB0(UnkObject_8003A52C *object, int index, int value);

#endif
