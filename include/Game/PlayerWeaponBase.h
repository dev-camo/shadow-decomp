#ifndef GAME_PLAYERWEAPONBASE_H
#define GAME_PLAYERWEAPONBASE_H

#include "Game/PlayerBase.h"
#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The 8-byte shared pointer that Player::WeaponBase's virtual functions at vtable offsets 0x14
 * and 0x18 take and return (placeholder name). The function at 0x800813B8 passes its hidden
 * result pointer to fn_8005BEDC, which clears two words; Player::Shadow::ShadowWeapon's override
 * (0x8023B1F0) copies its member at 0x34 into the result with a function that copies a pointer and
 * a boost shared count (0x8005BCA4), and the callers destroy the temporaries with fn_8005BD24.
 * The pointee type is not established, so the class is left incomplete. */
class UnkSharedPtr_8005BEDC;

namespace Player {

/* Player::WeaponBase (class name from RTTI, with Task as its only base; not Weapon::WeaponBase of
 * Game/WeaponBase.h): 0x30 bytes. Its only derived class in the RTTI records is
 * Player::Shadow::ShadowWeapon, whose constructor (USA (GUPE8P) 0x8023B5BC) starts its own
 * members at 0x30. The constructor stores the object in the word at 0x238 of its owner and the
 * destructor clears that word.
 *
 * The vtable (0x805223DC, 0x54 bytes) has 19 slots. Slots 1, 2 and 5 to 18 are null there, so
 * they are pure; slot 1 is Task's Update. The UnkSlot names are placeholders. Every signature
 * below is inferred from ShadowWeapon's override of the slot (its vtable is at 0x80547B3C) and
 * from the calls made through the pointer at 0x238 of a PlayerBase; the addresses are USA
 * (GUPE8P). Where the callers use no result and the override sets none, the result is void. */
class WeaponBase : public Task {
  public:
    WeaponBase(Task *parent, const boost::shared_ptr<PlayerBase> &owner);
    virtual ~WeaponBase();
    virtual void Update(float dt) = 0;
    /* The override (0x8023B29C) compares arg0 with a word as a signed value and tests arg1 for
     * a negative value; the caller at 0x8009B7C0 passes two words and uses no result. */
    virtual void UnkSlot2(int arg0, int arg1) = 0;
    /* Slots 3 and 4 are implemented in this class, in files that are not recovered yet, so they
     * are declared without bodies under placeholder names: the function at 0x800ADDD8 is empty,
     * and the one at 0x800813B8 returns an empty UnkSharedPtr_8005BEDC. The three callers of
     * slot 3 (0x800ADC70, 0x800ADCDC, 0x8027F11C) pass the address of a temporary shared pointer
     * and 999, and the override (0x8023B214) reads the pointer it holds and tests arg1 for a
     * negative value; a reference is assumed for the temporary. The callers of slot 4 (0x8008128C,
     * for example) pass only the result address and the object. */
    virtual void fn_800ADDD8(const UnkSharedPtr_8005BEDC &arg0, int arg1);
    virtual UnkSharedPtr_8005BEDC fn_800813B8();
    /* The overrides (0x8023B1B4, 0x8023B178) return the word at 0x14 or 0x18 of the object that
     * the member at 0x34 points to, or 0 without one; the callers compare the results as signed
     * words (0x8009B77C, 0x8008D670; 0x80081BF8 compares slot 6's with 5). */
    virtual int UnkSlot5() = 0;
    virtual int UnkSlot6() = 0;
    /* The override (0x8023B138) returns a float in f1: the result of a virtual function of the
     * object at 0x34, or the constant at 0x805F7318 without one. */
    virtual float UnkSlot7() = 0;
    /* Slots 8 to 14: the callers pass only the object and use no result (0x8009CAC4, 0x800AF838,
     * 0x800AA2DC, 0x8009B7A0, 0x800996D4, 0x800B50C8, 0x800B5174). The overrides of slots 8 to
     * 12 (0x8023B0E8 to 0x8023AFA4) read only the object; those of slots 13 and 14 (0x8023AF6C,
     * 0x8023AF34) pass the object at 0x34 on to one of its own virtual functions. */
    virtual void UnkSlot8() = 0;
    virtual void UnkSlot9() = 0;
    virtual void UnkSlot10() = 0;
    virtual void UnkSlot11() = 0;
    virtual void UnkSlot12() = 0;
    virtual void UnkSlot13() = 0;
    virtual void UnkSlot14() = 0;
    /* Slots 15 to 18 return a flag. The override of slot 15 (0x8023AF2C) returns 1, and those
     * of slots 16, 17 and 18 (0x8023AF04, 0x8023AEDC, 0x8023AEB4) return bits 2, 3 and 4 of the
     * flags at 0x30; the callers of slots 15, 16 and 17 test the low byte (0x800A0910,
     * 0x8007A7C8, 0x80081268). No call of slot 18 was found, so its type follows the override. */
    virtual bool UnkSlot15() = 0;
    virtual bool UnkSlot16() = 0;
    virtual bool UnkSlot17() = 0;
    virtual bool UnkSlot18() = 0;

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_owner; /* name inferred */
};
CHECK_SIZEOF(WeaponBase, 0x30);

} // namespace Player

#endif
