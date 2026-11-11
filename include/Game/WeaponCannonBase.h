#ifndef GAME_WEAPONCANNONBASE_H
#define GAME_WEAPONCANNONBASE_H

#include "Game/WeaponBase.h"
#include "layout_check.h"

class UnkResult_80051B58;
struct UnkWeaponModelDesc_8006465C;

namespace Weapon {

/* Weapon::WeaponCannonBase (class name from RTTI, with Weapon::WeaponBase as its only base): the
 * base class of the cannons. Its functions are defined elsewhere (0x8005E014-0x8005E26C in USA
 * (GUPE8P)). The virtual functions come from its vtable at 0x8051E92C, whose last slot is null.
 * The size comes from the word at 0x38 that Update and fn_8005E07C use and from the heal cannon's
 * factories, which allocate 0x3C bytes for a class whose own functions use no further member.
 * The virtual functions that have a base implementation keep placeholder names after it; their
 * bodies read no argument register besides the one holding this (and, in fn_8005E014, the
 * address of the result), and their empty parameter lists are inferred from that. fn_8005E07C
 * and fn_8005E014 are at 0x30 and 0x34 of the vtable, where Weapon::WeaponGunBase (WeaponGunBase.h)
 * has Shoot and ShootAt; they keep their placeholder names because their parameters and the
 * second one's return type differ from those. */
class WeaponCannonBase : public WeaponBase {
  public:
    /* Passes owner, type and 2 to WeaponBase's constructor and stores a new model built from owner
     * and desc at field_0x1C, or null when desc is null (0x8005E1EC; the const on desc and its
     * pointer spelling are inferred as for UnkWeaponModel_800631FC's constructor). */
    WeaponCannonBase(const boost::weak_ptr<CharInfo> &owner, int type,
                     const UnkWeaponModelDesc_8006465C *desc);
    /* Deletes the model at field_0x1C (0x8005E17C). */
    virtual ~WeaponCannonBase();
    /* Also passes position and rotation to fn_800632E8 of the model when there is one
     * (0x8005E130). */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Clears field_0x38 and passes dt to fn_800632A0 of the model when there is one
     * (0x8005E0FC). */
    virtual void Update(float dt);
    /* Passes the parameters that fn_80069540 builds from the weapon and the matrix of the model's
     * frame 0 to fn_80051C34 with field_0x14 as the kind, calls PlayShotEffects, adds 1 to
     * field_0x38 and returns 0.5. */
    virtual float fn_8005E07C();
    /* Returns the result of fn_80051AF0 for field_0x14, parameters built the same way and 2.0. */
    virtual UnkResult_80051B58 fn_8005E014();
    /* Pure in this class; fn_8005E07C calls it after requesting the shot. The heal cannon's
     * requests a sound at the position of the model's frame 0; the six other implementations
     * found in USA (GUPE8P) (0x8005D308, 0x8005D5C0, 0x8005D838, 0x8005EAD0, 0x80069008 and
     * 0x801F3D8C) also create effects with fn_8005E418 (name inferred from them). */
    virtual void PlayShotEffects() = 0;

    /* Cleared by Update and incremented by fn_8005E07C. */
    /* 0x38 */ int field_0x38;
};
CHECK_SIZEOF(Weapon::WeaponCannonBase, 0x3C);

} // namespace Weapon

#endif
