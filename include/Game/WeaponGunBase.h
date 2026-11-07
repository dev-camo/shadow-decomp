#ifndef GAME_WEAPONGUNBASE_H
#define GAME_WEAPONGUNBASE_H

#include "Game/WeaponBase.h"
#include "layout_check.h"

namespace Weapon {

/* Weapon::WeaponGunBase (class name from RTTI, with Weapon::WeaponBase as its only base): the base
 * class of the weapons that shoot. The layout comes from its constructor and destructor, which are
 * defined elsewhere with its other functions. Member and method names marked inferred are not
 * original; the virtual functions that have a base implementation keep placeholder names after
 * it. The slots below receive their vectors by address; their const reference spellings are
 * inferred. */
class WeaponGunBase : public WeaponBase {
  public:
    /* Passes owner, arg1 and 1 to WeaponBase's constructor and clears field_0x38. */
    WeaponGunBase(const boost::weak_ptr<CharInfo> &owner, int arg1);
    virtual ~WeaponGunBase();
    /* Clears field_0x38. */
    virtual void Update(float dt);
    /* Pure in this class. Weapon::HeavyShot's implementations fire a bullet from its model's frame
     * 0, the first along the frame's direction, the second towards target, and return 0.125;
     * Weapon::ChaosSpear::ChaosSpear's do nothing and return -1 (names inferred; HeavyShot passes
     * arg on as arg4 of the bullet parameters, whose meaning is unknown). */
    virtual float Shoot(const Fn8040Vector3 &arg) = 0;
    virtual float ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target) = 0;
    /* The base implementation returns 0; Weapon::ChaosSpear::ChaosSpear's fires its
     * charged spear and passes arg on as arg4 of the bullet parameters. */
    virtual float fn_8005C0CC(const Fn8040Vector3 &arg);
    /* The base implementations do nothing; Weapon::ChaosSpear::ChaosSpear's start and stop its
     * charge. */
    virtual void fn_8005C0D4();
    virtual void fn_8005C0D8();

    /* Adds count to field_0x38 (Weapon::HeavyShot passes 1 for each shot). */
    void fn_8005EB94(int count);

    /* 0x38 */ int field_0x38;
};
CHECK_SIZEOF(Weapon::WeaponGunBase, 0x3C);

} // namespace Weapon

#endif
