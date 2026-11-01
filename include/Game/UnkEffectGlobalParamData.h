#ifndef GAME_UNKEFFECTGLOBALPARAMDATA_H
#define GAME_UNKEFFECTGLOBALPARAMDATA_H

#include "layout_check.h"
#include "rw/rwcore.h"

namespace Effect {
namespace System {

/* Common accessed prefix of the serialized parameter records; original name unknown. */
struct UnkEffectGlobalParamData {
    /* 0x00 */ int type;
    /* 0x04 */ unsigned int flags;
};
CHECK_SIZEOF(UnkEffectGlobalParamData, 0x8);

/* Accessed prefixes only; the complete serialized record extents are unknown. */
struct UnkEffectGlobalParamData0 : UnkEffectGlobalParamData {
    /* 0x08 */ unsigned char field_0x8;
    /* 0x09 */ unsigned char field_0x9;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ int field_0x20;
};
CHECK_SIZEOF(UnkEffectGlobalParamData0, 0x24);

struct UnkEffectGlobalParamData1 : UnkEffectGlobalParamData {
    /* 0x08 */ unsigned char field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
};
CHECK_SIZEOF(UnkEffectGlobalParamData1, 0x18);

struct UnkEffectGlobalParamData2 : UnkEffectGlobalParamData {
    /* 0x08 */ RwRGBA field_0x8;
    /* 0x0C */ RwRGBA field_0xC;
};
CHECK_SIZEOF(UnkEffectGlobalParamData2, 0x10);

struct UnkEffectGlobalParamData3 : UnkEffectGlobalParamData {
    /* These serialized bytes have no identified reader or recovered type. */
    /* 0x08 */ unsigned char field_0x8[4];
    /* 0x0C */ int field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ float field_0x24;
    /* 0x28 */ float field_0x28;
    /* 0x2C */ float field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
};
CHECK_SIZEOF(UnkEffectGlobalParamData3, 0x3C);

} // namespace System
} // namespace Effect

#endif
