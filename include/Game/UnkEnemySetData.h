#ifndef GAME_UNKENEMYSETDATA_H
#define GAME_UNKENEMYSETDATA_H

#include "layout_check.h"

/* Placement parameters common to every enemy (placeholder type name): each enemy's gadget
 * parameter table starts with these seven entries, and the members follow their names. The
 * constructor stores the defaults those tables list (the same in all of them, except one table
 * whose seventh entry is MaxLife instead). */
struct UnkEnemySetData {
    UnkEnemySetData()
        : moveRange(200.0f), searchRange(0.0f), searchAngle(0.0f), searchWidth(100.0f),
          searchHeight(100.0f), searchHeightOffset(0.0f), moveSpeedRatio(1.0f) {}

    /* 0x00 */ float moveRange;
    /* 0x04 */ float searchRange;
    /* 0x08 */ float searchAngle;
    /* 0x0C */ float searchWidth;
    /* 0x10 */ float searchHeight;
    /* 0x14 */ float searchHeightOffset;
    /* 0x18 */ float moveSpeedRatio;
};
CHECK_SIZEOF(UnkEnemySetData, 0x1C);

#endif
