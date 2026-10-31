#ifndef GAME_CHARACOLLIBASE_H
#define GAME_CHARACOLLIBASE_H

namespace SonicteamUSA {
namespace System {
namespace CharaColli {

/* CharaColliBase (class name from RTTI, with no bases). Its 0x88-byte vtable holds 32 slots after
 * the RTTI pointer and offset; the last non-null one is slot 28 and many are null (pure virtual
 * functions). Only the first three slots are declared here: a null slot followed by the functions
 * at 0x800378FC and 0x800378E8. The class layout is not mapped, so there is no size check. */
class CharaColliBase {
  public:
    virtual void UnkSlot0() = 0;
    virtual void fn_800378FC();
    virtual void fn_800378E8();
};

/* CharaColliBaseCharInfo (class name from RTTI, with CharaColliBase as its only base): the base of
 * CharaColliBody and CharaColliGeneral, named by RTTI. Its members are not mapped. */
class CharaColliBaseCharInfo : public CharaColliBase {};

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
 * is assumed to target it. */
inline CharaColliBaseCharInfo *CastToCharaColliBaseCharInfo(CharaColliBase *colli) {
    return dynamic_cast<CharaColliBaseCharInfo *>(colli);
}

} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

#endif
