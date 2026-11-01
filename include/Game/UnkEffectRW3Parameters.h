#ifndef GAME_UNKEFFECTRW3PARAMETERS_H
#define GAME_UNKEFFECTRW3PARAMETERS_H

#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Original type names are unknown; these records are returned by the manager's getters. */
struct UnkEffectRW3Param0 {
    UnkEffectRW3Param0()
        : field_0x0(0), field_0x1(0), field_0x2(0), field_0x3(0), field_0x4(0), field_0x5(0),
          field_0x6(0), field_0x8(0.0f), field_0xC(0.0f), field_0x10(0), field_0x14(0),
          field_0x18(0), field_0x1C(0) {}

    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x02 */ unsigned char field_0x2;
    /* 0x03 */ unsigned char field_0x3;
    /* 0x04 */ unsigned char field_0x4;
    /* 0x05 */ unsigned char field_0x5;
    /* 0x06 */ unsigned char field_0x6;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1C;
};
CHECK_SIZEOF(UnkEffectRW3Param0, 0x20);

struct UnkEffectRW3Param1 {
    // The original default construction leaves the last two floats uninitialized.
    UnkEffectRW3Param1() : field_0x0(0), field_0x1(0), field_0x4(1.0f) {}

    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8[2];
};
CHECK_SIZEOF(UnkEffectRW3Param1, 0x10);

struct UnkEffectRW3Param2 {
    UnkEffectRW3Param2(unsigned char field0 = 0, unsigned char field1 = 0,
                       unsigned char field2 = 0);

    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x02 */ unsigned char field_0x2;
    /* 0x03 */ RwRGBA field_0x3;
    /* 0x07 */ RwRGBA field_0x7;
};
CHECK_SIZEOF(UnkEffectRW3Param2, 0xB);

/* The original copy operations preserve two equal groups of six words. */
struct UnkEffectRW3Param3Part {
    /* 0x00 */ int field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
};
CHECK_SIZEOF(UnkEffectRW3Param3Part, 0x18);

struct UnkEffectRW3Param3 {
    UnkEffectRW3Param3(unsigned char field0 = 0, unsigned char field1 = 0xFF, int field8 = 0,
                       float fieldC = 0.0f, float field10 = 0.0f, float field14 = 0.0f,
                       float field18 = 0.0f, float field1C = 1.0f, int field20 = 0,
                       float field24 = 0.0f, float field28 = 0.0f, float field2C = 0.0f,
                       float field30 = 0.0f, float field34 = 1.0f);

    UnkEffectRW3Param3(const UnkEffectRW3Param3 &other);

    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x02 */ unsigned char field_0x2[3];
    /* 0x08 */ UnkEffectRW3Param3Part field_0x8;
    /* 0x20 */ UnkEffectRW3Param3Part field_0x20;
};
CHECK_SIZEOF(UnkEffectRW3Param3, 0x38);

struct UnkEffectRW3Param4 {
    UnkEffectRW3Param4(unsigned char field0 = 0, unsigned char field1 = 0,
                       unsigned char field2 = 0);
    /* 0x00 */ unsigned char field_0x0;
    /* 0x01 */ unsigned char field_0x1;
    /* 0x02 */ unsigned char field_0x2;
    /* 0x04 */ RwV3d m_position;
};
CHECK_SIZEOF(UnkEffectRW3Param4, 0x10);

} // namespace RW3
} // namespace Effect

#endif
