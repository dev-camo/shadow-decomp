#ifndef GAME_WEAPONSTICKBASE_H
#define GAME_WEAPONSTICKBASE_H

#include "Game/WeaponBase.h"
#include "boost/scoped_ptr.hpp"
#include "layout_check.h"

namespace Weapon {

class WeaponStickBaseImpl;

/* Weapon::WeaponStickBase (class name from RTTI): the base class of the melee weapons that strike
 * along a line (Weapon::Stick derives from it). The work is done by a Weapon::WeaponStickBaseImpl
 * (class name from RTTI) that the stick owns; the stick itself forwards to it. Method and member
 * names marked inferred are not original. */
class WeaponStickBase : public WeaponBase {
  public:
    /* arg2 is stored in the implementation and passed on to the attack object (its meaning is
     * unknown). */
    WeaponStickBase(const boost::weak_ptr<CharInfo> &owner, int arg1, float arg2);
    virtual ~WeaponStickBase();
    /* Sweeps the attack while attacking and updates the model stretch (name inferred). */
    virtual void Update(float dt);
    /* Creates the attack object unless the stick is already attacking (name inferred). */
    virtual void StartAttack();
    /* Releases the attack object if the stick is attacking (name inferred). */
    virtual void EndAttack();
    /* Plays sound 0x2005 + index at the weapon's position (name inferred). */
    virtual void PlaySound(int index);

    /* Sets the line the stick strikes along and keeps the previous one (name inferred). */
    void SetLine(const Fn8040Vector3 &start, const Fn8040Vector3 &end);
    /* The number of hits counted since the last update (name inferred). */
    int GetHitCount() const;
    /* The stretch scale of the stick's model, 1 at rest (name inferred). */
    float GetScale() const;
    /* Whether the attack object exists (name inferred). */
    bool IsAttacking() const;

  private:
    /* 0x38 */ boost::scoped_ptr<WeaponStickBaseImpl> m_impl; /* name inferred */
};
CHECK_SIZEOF(Weapon::WeaponStickBase, 0x3C);

} // namespace Weapon

#endif
