#ifndef GAME_UNKENEMYSETDATA_H
#define GAME_UNKENEMYSETDATA_H

#include "layout_check.h"

/* Placement parameters common to every enemy (placeholder type name): each enemy's gadget
 * parameter table starts with these six entries, and the members follow their names. The seventh
 * entry differs per table, so it is not part of this record. */
struct UnkEnemySetDataCommon {
    /* 0x00 */ float moveRange;
    /* 0x04 */ float searchRange;
    /* 0x08 */ float searchAngle;
    /* 0x0C */ float searchWidth;
    /* 0x10 */ float searchHeight;
    /* 0x14 */ float searchHeightOffset;
};
CHECK_SIZEOF(UnkEnemySetDataCommon, 0x18);

/* The common entries followed by MoveSpeedRatio, the seventh entry of most enemy tables
 * (placeholder type name); BK_CHAOS's table has MaxLife there instead. The constructor stores the
 * defaults those tables list. */
struct UnkEnemySetData : public UnkEnemySetDataCommon {
    /* Matching workaround: the six common defaults are assigned here instead of in a constructor
     * of UnkEnemySetDataCommon. With that constructor (and this one initialising only
     * moveSpeedRatio), MWCC stops inlining this constructor in BkWingSmall: a weak
     * __ct__15UnkEnemySetDataFv appears and the code after it shifts by 0x34. Revisit when the
     * original constructor structure is known or the inline depth limit is understood. */
    UnkEnemySetData() {
        moveRange = 200.0f;
        searchRange = 0.0f;
        searchAngle = 0.0f;
        searchWidth = 100.0f;
        searchHeight = 100.0f;
        searchHeightOffset = 0.0f;
        moveSpeedRatio = 1.0f;
    }

    /* 0x18 */ float moveSpeedRatio;
};
CHECK_SIZEOF(UnkEnemySetData, 0x1C);

#endif
