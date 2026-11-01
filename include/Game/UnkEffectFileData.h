#ifndef GAME_UNKEFFECTFILEDATA_H
#define GAME_UNKEFFECTFILEDATA_H

#include "layout_check.h"

namespace Effect {
namespace System {

/* Accessed record prefix; the remaining serialized layout is not yet known. */
struct UnkEffectElementData {
    /* 0x00 */ unsigned char field_0x0[0x20];
    /* 0x20 */ short field_0x20;
    /* 0x24 */ int id;
    /* 0x28 */ UnkEffectElementData *next;
};
CHECK_SIZEOF(UnkEffectElementData, 0x2C);

/* Accessed file prefix. Unknown byte arrays are serialized storage, not padding. */
struct UnkEffectFileData {
    /* 0x00 */ char signature[4];
    /* 0x04 */ short field_0x4;
    /* 0x06 */ short field_0x6;
    /* 0x08 */ unsigned char field_0x8[0x24];
    /* 0x2C */ UnkEffectElementData *first;
};
CHECK_SIZEOF(UnkEffectFileData, 0x30);

struct UnkCommandData;

/* Accessed variant prefixes, not complete serialized record sizes. Unknown arrays represent
 * uninterpreted encoded storage; automatic alignment gaps are left to the compiler. */
struct UnkEffectElementData2 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ unsigned char field_0x38[8];
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ const UnkCommandData *field_0x48;
    /* 0x4C */ unsigned int field_0x4C;
    /* 0x50 */ int field_0x50;
    /* 0x54 */ unsigned char field_0x54[4];
    /* 0x58 */ void *field_0x58;
    /* 0x5C */ unsigned int field_0x5C;
    /* 0x60 */ short field_0x60;
    /* 0x62 */ short field_0x62;
    /* 0x64 */ unsigned char field_0x64[4];
    /* 0x68 */ const UnkCommandData *field_0x68;
    /* 0x6C */ float field_0x6C;
    /* 0x70 */ void *field_0x70;
};
CHECK_SIZEOF(UnkEffectElementData2, 0x74);

struct UnkEffectElementData3 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ short field_0x30;
    /* 0x32 */ short field_0x32;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ void *field_0x38;
};
CHECK_SIZEOF(UnkEffectElementData3, 0x3C);

struct UnkEffectElementData4 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ int field_0x30;
    /* 0x34 */ unsigned char field_0x34[4];
    /* 0x38 */ unsigned char field_0x38;
    /* 0x39 */ unsigned char field_0x39[3];
    /* 0x3C */ void *field_0x3C;
    /* 0x40 */ const UnkCommandData *field_0x40;
};
CHECK_SIZEOF(UnkEffectElementData4, 0x44);

struct UnkEffectElementData5 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ unsigned char field_0x38[8];
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ const UnkCommandData *field_0x48;
    /* 0x4C */ unsigned int field_0x4C;
    /* 0x50 */ unsigned char field_0x50[8];
    /* 0x58 */ void *field_0x58;
    /* 0x5C */ void *field_0x5C;
    /* 0x60 */ unsigned int field_0x60;
    /* 0x64 */ short field_0x64;
    /* 0x66 */ short field_0x66;
    /* 0x68 */ unsigned char field_0x68[4];
    /* 0x6C */ const UnkCommandData *field_0x6C;
    /* 0x70 */ float field_0x70;
    /* 0x74 */ void *field_0x74;
};
CHECK_SIZEOF(UnkEffectElementData5, 0x78);

struct UnkEffectElementData6 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ unsigned char field_0x30[0x14];
    /* 0x44 */ float field_0x44;
    /* 0x48 */ unsigned int field_0x48;
    /* 0x4C */ unsigned char field_0x4C[8];
    /* 0x54 */ const UnkCommandData *field_0x54;
    /* 0x58 */ unsigned int field_0x58;
    /* 0x5C */ short field_0x5C;
    /* 0x5E */ short field_0x5E;
    /* 0x60 */ unsigned char field_0x60[4];
    /* 0x64 */ const UnkCommandData *field_0x64;
    /* 0x68 */ float field_0x68;
    /* 0x6C */ void *field_0x6C;
};
CHECK_SIZEOF(UnkEffectElementData6, 0x70);

struct UnkEffectElementData7 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ int field_0x34;
    /* 0x38 */ unsigned int field_0x38;
    /* 0x3C */ int field_0x3C;
    /* 0x40 */ unsigned char field_0x40[4];
    /* 0x44 */ unsigned int field_0x44;
    /* 0x48 */ short field_0x48;
    /* 0x4A */ short field_0x4A;
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ const UnkCommandData *field_0x50;
    /* 0x54 */ const UnkCommandData *field_0x54;
};
CHECK_SIZEOF(UnkEffectElementData7, 0x58);

struct UnkEffectElementData8 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ void *field_0x34;
};
CHECK_SIZEOF(UnkEffectElementData8, 0x38);

struct UnkEffectElementData9 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ unsigned char field_0x30;
    /* 0x31 */ unsigned char field_0x31;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ void *field_0x44;
    /* 0x48 */ void *field_0x48;
    /* 0x4C */ void *field_0x4C;
};
CHECK_SIZEOF(UnkEffectElementData9, 0x50);

struct UnkEffectElementData10 : UnkEffectElementData {
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ unsigned char field_0x30;
    /* 0x32 */ short field_0x32;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ void *field_0x38;
    /* 0x3C */ void *field_0x3C;
};
CHECK_SIZEOF(UnkEffectElementData10, 0x40);

} // namespace System
} // namespace Effect

#endif
