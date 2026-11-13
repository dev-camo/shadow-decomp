#ifndef GAME_UNKOBJECT_800B74B4_H
#define GAME_UNKOBJECT_800B74B4_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The object that fn_800B74B4 returns (placeholder name; the original class is unknown). */
class UnkObject_800B74B4;

/* One of the 0x3B records of 0x50 bytes in the constant table that fn_80176C3C searches (at
 * 0x804C4A18 in the USA release; placeholder name, the original type is unknown). Only the words
 * at 0x0 and 0x8 are mapped. */
struct UnkRecord_80176C3C {
    /* 0x00 */ s32 field_0x0; /* the number fn_80176C3C looks for */
    /* 0x04 */ u8 field_0x4[0x8 - 0x4];
    /* TGiPause loads the pause project "CsdFiles/pause/pauseBoss" when the record of
     * lbl_8057D6A0.field_0xA8.field_0x0 has bit 0x10 or bit 0x4 set (meanings unknown). */
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ u8 field_0xC[0x50 - 0xC];
};
CHECK_SIZEOF(UnkRecord_80176C3C, 0x50);

/* The names, the parameter types and the const qualifiers are not known (inferred); the behaviour
 * comes from the bodies. */
extern "C" {
/* Returns the object, setting a static pointer to the 4-byte static object (at 0x805EE5F8 in the
 * USA release) on the first call. */
UnkObject_800B74B4 *fn_800B74B4(void);
/* Returns the table's record whose field_0x0 is number, or null when there is none; -1 stands for
 * lbl_8057D6A0.field_0xA8.field_0x0. The body does not read object. */
const UnkRecord_80176C3C *fn_80176C3C(UnkObject_800B74B4 *object, int number);
}

#endif
