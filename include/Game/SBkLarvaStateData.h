#ifndef GAME_SBKLARVASTATEDATA_H
#define GAME_SBKLARVASTATEDATA_H

#include "Game/SetAdapterState.h"
#include "Game/UnkEnemySetData.h"

/* State type name from RTTI; member and parameter type names are placeholders. */
struct SBkLarvaStateData : public SetAdapterState {
    SBkLarvaStateData(int arg0);
    virtual ~SBkLarvaStateData();
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
};
CHECK_SIZEOF(SBkLarvaStateData, 0xC);

/* The constructor at 0x8018A3D4 stores UnkEnemySetData's defaults at 0x00-0x18 before the words
 * that follow. */
struct UnkParameters_8018A3D4 : public UnkEnemySetData {
    UnkParameters_8018A3D4();
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ unsigned int field_0x24; // Meaning unknown; the constructor stores zero.
    /* 0x28 */ float field_0x28;
};
CHECK_SIZEOF(UnkParameters_8018A3D4, 0x2C);

#endif
