#ifndef GAME_EFFECTMANAGERPJS_H
#define GAME_EFFECTMANAGERPJS_H

#include "Game/EffectManagerRW3.h"
#include "Game/Singleton.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace SonicteamUSA {
namespace System {
namespace Effect {

/* SonicteamUSA::System::Effect::EffectManagerUSA (class name and bases from RTTI): vtable
 * 0x8051CA58, whose destructor slot holds an out-of-line copy (0x80028C3C) of the inline destructor
 * that Effect::PJS::EffectManagerPJS's destructor expands. Its constructor is inline too. Only the
 * two RW3 slots it overrides are declared (implementations 0x80027770 and 0x80028BA4); the member
 * at 0x54, which neither the constructor nor the destructor touches, is not mapped. */
class EffectManagerUSA : public ::Effect::RW3::EffectManagerRW3 {
  public:
    EffectManagerUSA() {}
    virtual ~EffectManagerUSA() {}
    virtual ::Effect::System::EffectElement *
    CreateElement(::Effect::System::UnkEffectElementData *definition,
                  ::Effect::System::EffectLaunchInfo *info);
    virtual void fn_80437C14(RpClump *clump, unsigned char flag, int count, const RwRGBA &color);

    /* 0x54 */ u8 field_0x54[0x58 - 0x54];

  private:
    EffectManagerUSA(const EffectManagerUSA &);
    EffectManagerUSA &operator=(const EffectManagerUSA &);
};
CHECK_SIZEOF(EffectManagerUSA, 0x58);

} // namespace Effect
} // namespace System
} // namespace SonicteamUSA

namespace Effect {
namespace PJS {

/* Effect::PJS::EffectManagerPJS (class name and bases from RTTI): the game's effect manager, of
 * which its singleton base gives the one instance (the accessor, USA (GUPE8P) 0x800A31F0, is what
 * other files call to reach the manager). Its RTTI base list names the singleton base
 * Sonicteam::System::Singleton<Effect::PJS::EffectManagerPJS,
 * Sonicteam::System::CreateStatic<Effect::PJS::EffectManagerPJS>> first, at offset 0x58, and then
 * EffectManagerUSA's bases at 0: this compiler gives that list and offset for an empty base
 * declared after EffectManagerUSA (measured). The instance (USA 0x805768CC) reaches the end of
 * Game/PlayerEffect.cpp's .bss at 0x80576928, 0x5C bytes on. The next object in .bss starts
 * there, on an 8-byte boundary, so the last four bytes may be alignment padding rather than part
 * of the class. They are kept as an unmapped member at 0x58, which the constructor does not
 * initialise. The constructor and destructor are inline (out-of-line copies at 0x800A32E4 and
 * 0x800A327C). Its vtable (0x8053C578, 0x1C bytes) has the same five slots as EffectManagerUSA's;
 * it overrides the destructor and CreateElement (0x801D0AFC) and adds none. */
class EffectManagerPJS
    : public SonicteamUSA::System::Effect::EffectManagerUSA,
      public Sonicteam::System::Singleton<EffectManagerPJS,
                                          Sonicteam::System::CreateStatic<EffectManagerPJS> > {
  public:
    EffectManagerPJS() {}
    virtual ~EffectManagerPJS() {}
    virtual System::EffectElement *CreateElement(System::UnkEffectElementData *definition,
                                                 System::EffectLaunchInfo *info);

    /* 0x58 */ u8 field_0x58[0x5C - 0x58];

  private:
    EffectManagerPJS(const EffectManagerPJS &);
    EffectManagerPJS &operator=(const EffectManagerPJS &);
};
CHECK_SIZEOF(EffectManagerPJS, 0x5C);

} // namespace PJS
} // namespace Effect

#endif
