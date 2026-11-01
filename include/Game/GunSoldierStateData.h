#ifndef GAME_GUNSOLDIERSTATEDATA_H
#define GAME_GUNSOLDIERSTATEDATA_H

#include "Game/SetAdapterState.h"

/* Namespace and state type names come from RTTI; member meanings are unknown. */
namespace GunSoldier {
struct STemporaryData : public SetAdapterState {
    STemporaryData();
    virtual ~STemporaryData();
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
};
CHECK_SIZEOF(STemporaryData, 0xC);
} // namespace GunSoldier

/* Original parameter type name unknown; field types are inferred from construction and use. */
struct UnkParameters_8018ED00 {
    UnkParameters_8018ED00();
    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ int field_0x20;
    /* 0x24 */ int field_0x24;
    /* 0x28 */ int field_0x28;
    /* 0x2C */ float field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ int field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ float field_0x48;
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ int field_0x50;
    /* 0x54 */ float field_0x54;
    /* 0x58 */ float field_0x58;
};
CHECK_SIZEOF(UnkParameters_8018ED00, 0x5C);

#endif
