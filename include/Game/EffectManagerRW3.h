#ifndef GAME_EFFECTMANAGERRW3_H
#define GAME_EFFECTMANAGERRW3_H

#include "Game/EffectManager.h"
#include "layout_check.h"

struct RpClump;
struct RwRaster;
struct RwRGBA;

namespace Effect {
namespace RW3 {

struct UnkEffectRW3Param0;
struct UnkEffectRW3Param1;
struct UnkEffectRW3Param2;
struct UnkEffectRW3Param3;
struct UnkEffectManagerRW3Data2C;
struct UnkEffectManagerRW3Data3C;
struct UnkEffectManagerRW3Data40;
struct UnkEffectManagerRW3Data44;
struct UnkEffectManagerRW3Data48;
struct UnkEffectManagerRW3Data4C;
struct UnkEffectManagerRW3Data50;

/* Class and base names come from RTTI. Parameter types and qualifiers are inferred from use. */
class EffectManagerRW3 : public System::EffectManager {
  public:
    EffectManagerRW3();
    virtual ~EffectManagerRW3();
    virtual System::EffectElement *CreateElement(const System::UnkEffectElementData *definition,
                                                 System::EffectLaunchInfo *info);
    virtual void fn_80437C14(RpClump *clump, unsigned char flag, int count, const RwRGBA &color);

    UnkEffectRW3Param0 fn_80430844();
    void fn_804308C0(const UnkEffectRW3Param0 &parameters);
    UnkEffectRW3Param1 fn_8042E034();
    void fn_8042E098(const UnkEffectRW3Param1 &parameters);
    UnkEffectRW3Param2 fn_804388A4();
    void fn_8043891C(const UnkEffectRW3Param2 &parameters);
    void fn_80438948();
    void fn_80438974();
    void fn_804389F0(const UnkEffectRW3Param2 *parameters);
    UnkEffectRW3Param3 fn_80436F2C();
    void fn_80436FC0(const UnkEffectRW3Param3 &parameters);
    void fn_80436FEC();
    void fn_80437018(float deltaTime);
    void fn_80437044();
    void fn_80437088(const UnkEffectRW3Param3 *parameters);

    /* 0x2C */ UnkEffectManagerRW3Data2C *field_0x2C;
    /* 0x30 */ int field_0x30;
    /* 0x34 */ RwRaster *field_0x34;
    /* 0x38 */ RwRaster *field_0x38;
    /* 0x3C */ UnkEffectManagerRW3Data3C *field_0x3C;
    /* 0x40 */ UnkEffectManagerRW3Data40 *field_0x40;
    /* 0x44 */ UnkEffectManagerRW3Data44 *field_0x44;
    /* 0x48 */ UnkEffectManagerRW3Data48 *field_0x48;
    /* 0x4C */ UnkEffectManagerRW3Data4C *field_0x4C;
    /* 0x50 */ UnkEffectManagerRW3Data50 *field_0x50;
};
/* Extent inferred from the final pointer at 0x50 and the derived manager's field at 0x54. */
CHECK_SIZEOF(EffectManagerRW3, 0x54);

} // namespace RW3
} // namespace Effect

#endif
