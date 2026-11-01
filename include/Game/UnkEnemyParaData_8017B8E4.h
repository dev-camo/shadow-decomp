#ifndef GAME_UNKENEMYPARADATA_8017B8E4_H
#define GAME_UNKENEMYPARADATA_8017B8E4_H

#include "layout_check.h"

/* Original type names are unknown; layouts follow the parameter readers. */
struct UnkEnemyParaEntry_800C9178 {
    /* 0x00 */ short field_0x0;
    /* 0x02 */ short field_0x2;
    /* 0x04 */ short field_0x4;
    /* 0x06 */ signed char field_0x6;
    /* 0x07 */ unsigned char field_0x7;
};
CHECK_SIZEOF(UnkEnemyParaEntry_800C9178, 0x8);

struct UnkEnemyParaGroup_800C961C {
    /* 0x00 */ const UnkEnemyParaEntry_800C9178 *entries;
    /* 0x04 */ signed char count;
    /* 0x05 */ signed char field_0x5;
};
CHECK_SIZEOF(UnkEnemyParaGroup_800C961C, 0x8);

struct UnkEnemyParaData_8017B8E4 {
    /* 0x00 */ const UnkEnemyParaGroup_800C961C *groups;
    /* 0x04 */ signed char count;
};
CHECK_SIZEOF(UnkEnemyParaData_8017B8E4, 0x8);

/* Separate records returned by the parameter interface's slot 0x74. */
struct UnkEnemyParaEntry_801A6230 {
    /* 0x00 */ int field_0x0;
    /* 0x04 */ unsigned int field_0x4;
};
CHECK_SIZEOF(UnkEnemyParaEntry_801A6230, 0x8);

#endif
