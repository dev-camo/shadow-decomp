#ifndef GAME_UNKMANAGER_80170D8C_H
#define GAME_UNKMANAGER_80170D8C_H

#include <dolphin/types.h>
#include "rw/rwcore.h"
#include "layout_check.h"

/* The record that the word at 0x8 of UnkManager_80170D8C points to (placeholder name). The .CEN
 * reader copies three floats from 0x8 plus 0xC times an index, or from 0x8 itself when the index
 * is negative; the number of these positions is not known, so the array is declared with one
 * element. The RwV3d type is inferred from the member-by-member copy. */
struct UnkPoints_80170D8C {
    /* 0x00 */ u8 field_0x0[0x8];
    /* 0x08 */ RwV3d field_0x8[1];
};
/* Checks only the declared view, not the record's size. */
CHECK_SIZEOF(UnkPoints_80170D8C, 0x14);

/* The object that fn_80170D8C returns: a static object (at 0x805801D4 in the USA release,
 * GUPE8P) that the function constructs with fn_80174228 on the first call and caches in a static
 * pointer (placeholder name; the original class is unknown). Only the pointer at 0x8 is mapped. */
class UnkManager_80170D8C {
  public:
    /* 0x00 */ u8 field_0x0[0x8];
    /* 0x08 */ UnkPoints_80170D8C *field_0x8;
};
/* Checks only the declared view, not the object's size. */
CHECK_SIZEOF(UnkManager_80170D8C, 0xC);

extern "C" {
/* Returns the object, constructing it on first use (behaviour from its body). */
UnkManager_80170D8C *fn_80170D8C(void);
}

/* Objects of the unrecovered event code that defines fn_80170D8C (placeholder names; that code
 * also reads and writes them). The RwV3d type of the three vectors is inferred: the .CEN reader
 * copies and adds them member by member, and the event code passes lbl_80580218 to fn_804875B0
 * as its translation (0x8016FDE4). */
/* The position that UnkCenFile_80329514::Update adds the key position to. */
extern RwV3d lbl_80580218;
/* The position of the key that Update found, or zero when the file has no keys. */
extern RwV3d lbl_80580224;
/* The sum of the two. The next object starts 0x10 bytes after it in all three releases (at
 * 0x80580240 in the USA release, GUPE8P); Update writes its first 0xC bytes, and the use of the
 * last four is not known. */
extern RwV3d lbl_80580230;
/* Update compares it with 950, 970, 972, 974, 976 and 978 (meaning unknown). */
extern int lbl_805EF2A0;
/* The index of the position that Update takes from the manager's record; Update uses the first
 * position when it is negative. */
extern int lbl_805EF2A4;
/* The index of the key that Update found. */
extern int lbl_805EF2B8;

#endif
