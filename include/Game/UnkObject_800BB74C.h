#ifndef GAME_UNKOBJECT_800BB74C_H
#define GAME_UNKOBJECT_800BB74C_H

#include <dolphin/types.h>
#include "layout_check.h"

/* One of the 0x3B records of 0x60 bytes that fn_801E5614 constructs with fn_801E5684 (placeholder
 * name after that constructor; the original class is unknown). Only the five words at 0x4C are
 * mapped: fn_801E51E4 sets them to -1 in every record, and the SECRETKEY gadget's file copies its
 * key list to and from the record that lbl_8057D6A0.field_0xA8.field_0x4 selects. */
struct UnkRecord_801E5684 {
    /* 0x00 */ u8 field_0x0[0x4C];
    /* 0x4C */ s32 field_0x4C[5];
};
CHECK_SIZEOF(UnkRecord_801E5684, 0x60);

/* The 0x3EB8-byte static object (at 0x80576958 in the USA release) that fn_800BB74C constructs and
 * caches on first use (placeholder name; the original class is unknown). The record array belongs
 * to the object that fn_801E5614 constructs at 0x30 (its array starts at that object's 0x78); this
 * view maps only that array, at its offset from the start of the outer object, and ends with it:
 * its size is not the object's. */
class UnkObject_800BB74C {
  public:
    /* 0x00 */ u8 field_0x0[0xA8];
    /* 0xA8 */ UnkRecord_801E5684 field_0xA8[0x3B];
};
CHECK_OFFSETOF(UnkObject_800BB74C, field_0xA8, 0xA8);
CHECK_SIZEOF(UnkObject_800BB74C, 0x16C8);

extern "C" {
/* Returns the object, constructing it on first use (behaviour inferred from its body). */
UnkObject_800BB74C *fn_800BB74C(void);
}

#endif
