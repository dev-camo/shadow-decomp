#ifndef GAME_UNKMANAGER_800CBAA4_H
#define GAME_UNKMANAGER_800CBAA4_H

#include <dolphin/types.h>
#include "rw/rwcore.h"
#include "layout_check.h"

/* An element of the array at 0x60 of UnkRecord_801E1A24 (placeholder name). The elements are
 * 0x1C bytes apart and start with three floats that the .CEN reader copies as a position; the
 * RwV3d type is inferred from that member-by-member copy. The rest is not mapped. */
struct UnkPoint_800CBAA4 {
    /* 0x00 */ RwV3d position;
    /* 0x0C */ u8 field_0xC[0x1C - 0xC];
};
CHECK_SIZEOF(UnkPoint_800CBAA4, 0x1C);

/* One of the 59 0xC0-byte records at 0x10 of the object that fn_800CBAA4 returns (placeholder
 * name, after the function that its constructor fn_801E0910 passes to __construct_array for
 * them). The .CEN reader indexes the records with the word at 0xAC of lbl_8057D6A0. Only the
 * members that it copies are mapped. */
struct UnkRecord_801E1A24 {
    /* 0x00 */ u8 field_0x0[0x60];
    /* The reader takes element 1 or the element chosen by the word at 0xC4 of lbl_8057D6A0. The
     * number of elements is not known; two is what fits in front of 0x98. */
    /* 0x60 */ UnkPoint_800CBAA4 field_0x60[2];
    /* 0x98 */ u8 field_0x98[0xA8 - 0x98];
    /* 0xA8 */ RwV3d field_0xA8;
    /* 0xB4 */ RwV3d field_0xB4;
};
CHECK_SIZEOF(UnkRecord_801E1A24, 0xC0);

/* The object that fn_800CBAA4 returns: a static object (at 0x8057A894 in the USA release,
 * GUPE8P) that the function constructs with fn_801E0910 on the first call and caches in a static
 * pointer (placeholder name; the original class is unknown). This is a view of the record array
 * only: the first 0x10 bytes and the bytes after the array are not mapped. */
class UnkManager_800CBAA4 {
  public:
    /* 0x00 */ u8 field_0x0[0x10];
    /* 0x10 */ UnkRecord_801E1A24 field_0x10[59];
};
/* Checks only the declared view, not the object's size. */
CHECK_SIZEOF(UnkManager_800CBAA4, 0x2C50);

extern "C" {
/* Returns the object, constructing it on first use (behaviour from its body). */
UnkManager_800CBAA4 *fn_800CBAA4(void);
}

#endif
