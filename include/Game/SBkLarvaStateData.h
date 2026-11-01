#ifndef GAME_SBKLARVASTATEDATA_H
#define GAME_SBKLARVASTATEDATA_H

#include "Game/SetAdapterState.h"

/* State type name from RTTI; member and parameter type names are placeholders. */
struct SBkLarvaStateData : public SetAdapterState {
    SBkLarvaStateData(int arg0);
    virtual ~SBkLarvaStateData();
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
};
CHECK_SIZEOF(SBkLarvaStateData, 0xC);

struct UnkParameters_8018A3D4 {
    UnkParameters_8018A3D4();
    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ unsigned int field_0x24; // Meaning unknown; the constructor stores zero.
    /* 0x28 */ float field_0x28;
};
CHECK_SIZEOF(UnkParameters_8018A3D4, 0x2C);

#endif
