#ifndef GAME_FN_8003A840_H
#define GAME_FN_8003A840_H

#include <dolphin/types.h>
#include "layout_check.h"

struct RpWorld;
class UnkManager_8003A840;
class LandDispManager;

/* The original world table has fifteen records with a 0x18 stride. */
struct UnkWorldEntry_8051D208 {
    /* 0x00 */ const char *field_0x0;
    /* 0x04 */ const char *field_0x4;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ u32 field_0xC;
    /* 0x10 */ RpWorld *world;
    /* 0x14 */ LandDispManager *field_0x14;
};
CHECK_SIZEOF(UnkWorldEntry_8051D208, 0x18);

extern UnkWorldEntry_8051D208 lbl_8051D208[15];

extern "C" {
UnkManager_8003A840 *fn_8003A840();
/* The original getter does not inspect the manager argument or bound the index. */
inline RpWorld *fn_800593D0(UnkManager_8003A840 *, int index) {
    return lbl_8051D208[index].world;
}
}

#endif
