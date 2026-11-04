#ifndef GAME_CHARACOLLIBASECHARINFO_H
#define GAME_CHARACOLLIBASECHARINFO_H

#include "Game/CharaColliBase.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;

namespace SonicteamUSA {
namespace System {
namespace CharaColli {

class CharaColliAttack;

/* CharaColliBaseCharInfo (class name from RTTI, with CharaColliBase as its only base): the base of
 * CharaColliBody and CharaColliGeneral, named by RTTI. Its constructor (0x800C0720) passes arg0 and
 * arg1 to CharaColliBase's, stores the vtable at 0x80522694 and constructs the two members; its
 * destructor (0x800C06A0) passes the object to 0x800C46DC with the world object of fn_80021B08,
 * then destroys the members and the base. The next derived constructor (0x800C15A8) starts its own
 * members at 0x40. */
class CharaColliBaseCharInfo : public CharaColliBase {
  public:
    /* owner is passed by value: both callers (0x800C1578, 0x800C370C) copy their weak pointer into
     * a temporary for the call. */
    CharaColliBaseCharInfo(s16 arg0, s16 arg1, boost::weak_ptr<CharInfo> owner);
    virtual ~CharaColliBaseCharInfo();
    /* Slots 32 and 33 are null in this class's vtable (0x80522694) and implemented by
     * CharaColliBody (0x800C08B4, 0x800C132C) and CharaColliGeneral (0x800C1938, 0x800C30F0).
     * 0x800C13B8-0x800C13D4 and 0x800C31C4-0x800C31E0 call them one after the other without
     * arguments and use no result; the implementations return no value on every path. */
    virtual void UnkSlot32() = 0;
    virtual void UnkSlot33() = 0;

    /* 0x30 */ boost::weak_ptr<CharInfo> m_owner; /* name inferred from the constructor argument */
    /* 0x38 */ boost::shared_ptr<CharaColliAttack> field_0x38;
};
CHECK_OFFSETOF(CharaColliBaseCharInfo, m_owner, 0x30);
CHECK_OFFSETOF(CharaColliBaseCharInfo, field_0x38, 0x38);
CHECK_SIZEOF(CharaColliBaseCharInfo, 0x40);

/* Matching workaround: as in TNumEdit.h, this never-called inline function stands in for the
 * original's unknown inline function so that the same data is emitted. Revisit when the original
 * inline function or a real caller is found.
 *
 * ElecCircuitEntrance.cpp and many files that are not split yet have an unused 0xC-byte RTTI base
 * list in their .data that names CharaColliBase as the only base, at offset 0; it comes from a
 * dynamic_cast or typeid in the same way as the TNumEdit data (see TNumEdit.h).
 * CharaColliBaseCharInfo is the only class named in the game's RTTI records whose base list is
 * exactly that one, and each copy of its name string in the .rodata of other files comes directly
 * before CharaColliBase's, the order in which the cast below first uses the two names, so the cast
 * is assumed to target it. The same cast is made at 0x800C45C4-0x800C45D8 in 0x800C4568, which
 * passes the RTTI of CharaColliBase as the source and that of CharaColliBaseCharInfo as the
 * target. It is kept apart from CharaColliBase.h so that only the files with that data include
 * it. */
inline CharaColliBaseCharInfo *CastToCharaColliBaseCharInfo(CharaColliBase *colli) {
    return dynamic_cast<CharaColliBaseCharInfo *>(colli);
}

} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

#endif
