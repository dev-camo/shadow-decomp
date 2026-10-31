#ifndef GAME_TNUMEDIT_H
#define GAME_TNUMEDIT_H

/* TNumEdit (class name from RTTI, with no bases) and three classes derived from it.
 *
 * ElecCircuitEntrance.cpp and many gadget and weapon files that are not split yet have three
 * 0xC-byte RTTI base lists in their .data that each name TNumEdit as the only base, at offset 0,
 * with TNumEdit's RTTI record in their .sdata and its name in their .rodata. Nothing in those files
 * uses them. MWCC builds a class's RTTI record, name and base list as soon as it parses a
 * dynamic_cast or typeid naming that class, even inside an inline function that is never called;
 * the linker discards the unreferenced records and names but keeps the base lists.
 *
 * Matching workaround: the three CastToTNumEdit* functions are never called. They stand in for the
 * original's unknown inline functions so that the RTTI base lists and name strings are emitted.
 * Revisit when the original inline functions or a real caller are found.
 *
 * The names of the derived classes come from RTTI name strings that survive in the .rodata of
 * other files: 18 copies, each next to TNumEdit's in the order TNumEditHex, TNumEdit, TNumEditInt,
 * TNumEditFloat, which is the order in which the casts below first use a derived name and then its
 * base's name. Their members and vtables and the original inline functions are unknown: the casts
 * are placeholders that produce the same data.
 *
 * Three copies of TNumEdit's vtable, each pointing to a TNumEdit RTTI record, start with two null
 * slots (pure virtual functions) followed by two defined functions at different addresses in each
 * copy. The rest of the vtable and the class layout are not mapped, so the classes have no size
 * checks. */
class TNumEdit {
  public:
    virtual void UnkSlot0() = 0;
    virtual void UnkSlot1() = 0;
    virtual void UnkSlot2();
    virtual void UnkSlot3();
};

class TNumEditHex : public TNumEdit {};
class TNumEditInt : public TNumEdit {};
class TNumEditFloat : public TNumEdit {};

inline TNumEditHex *CastToTNumEditHex(TNumEdit *edit) {
    return dynamic_cast<TNumEditHex *>(edit);
}

inline TNumEditInt *CastToTNumEditInt(TNumEdit *edit) {
    return dynamic_cast<TNumEditInt *>(edit);
}

inline TNumEditFloat *CastToTNumEditFloat(TNumEdit *edit) {
    return dynamic_cast<TNumEditFloat *>(edit);
}

#endif
