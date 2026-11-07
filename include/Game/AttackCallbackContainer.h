#ifndef GAME_ATTACKCALLBACKCONTAINER_H
#define GAME_ATTACKCALLBACKCONTAINER_H

#include "Game/CharaColliAttack.h"
#include "layout_check.h"

/* AttackCallbackContainer (class name from RTTI, with no bases): a base class whose only member is
 * its vtable pointer. The six virtual functions are declared in the order of its vtable
 * (0x8055D348); the class has no virtual destructor, and the names are placeholders. The first
 * three implementations are empty (0x80233574, 0x80233578, 0x8023357C). Each of the last three
 * (0x80335FFC, 0x80335D6C, 0x80335A80) binds one of the first three to this (the pointers to
 * member at 0x8055D324, 0x8055D330 and 0x8055D33C select vtable offsets 8, 0xC and 0x10) through
 * the same binding function (0x80336734) and returns the binding as a 0xC-byte object, whose
 * constructor (0x80335B1C) clears three words and then stores the binding; the AttackCallback
 * return type is inferred from that. The RTTI name of the binding's type in the DOL contains
 * boost::_mfi::mf1<void, AttackCallbackContainer,
 * SonicteamUSA::System::CharaColli::AttackCallbackParam &>, which gives the signature of the first
 * three; the overrides of slots 0 and 2 in HeavyDogCtrlUnit_AttackCallbackCanopy (0x8024D330,
 * 0x8024D0D8) read their argument as an AttackCallbackParam. */
class AttackCallbackContainer {
  public:
    virtual void fn_80233574(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    virtual void fn_80233578(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    virtual AttackCallback fn_80335FFC();
    virtual AttackCallback fn_80335D6C();
    virtual AttackCallback fn_80335A80();
};
CHECK_SIZEOF(AttackCallbackContainer, 0x4);

#endif
