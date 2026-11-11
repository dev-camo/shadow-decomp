#ifndef GAME_FN_8040DB6C_H
#define GAME_FN_8040DB6C_H

#include "layout_check.h"

/* One 44-byte element of the array that fn_8040DB6C indexes (placeholder name). Recovered view:
 * only the members that the camera mode's update at USA (GUPE8P) 0x80008E2C reads are mapped. It
 * tests bits of the first word and reads the four floats, which it uses as two pairs of signed
 * amounts; what the array holds is not established. */
typedef struct Fn8040DB6CEntry {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ char field_0x4[0xC];
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ char field_0x18[0x4];
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ char field_0x24[0x8];
} Fn8040DB6CEntry;
CHECK_SIZEOF(Fn8040DB6CEntry, 0x2C);

#ifdef __cplusplus
extern "C" {
#endif
/* Returns &base[index]. */
Fn8040DB6CEntry *fn_8040DB6C(Fn8040DB6CEntry *base, unsigned int index);
/* Returns the array in the object at lbl_8056ED4C, which it constructs with fn_8040DC5C at its
 * first call (behaviour from its body; the type of that object is not recovered, and the result
 * is typed as fn_8040DB6C takes it). */
Fn8040DB6CEntry *fn_80009208(void);
#ifdef __cplusplus
}
#endif

#endif
