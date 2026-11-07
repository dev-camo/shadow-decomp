#ifndef GAME_CHAOSSPEAR_H
#define GAME_CHAOSSPEAR_H

#include "Game/PJSLauncher.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/WeaponGunBase.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Weapon {
namespace ChaosSpear {

/* Weapon::ChaosSpear::ChaosSpear (class name from RTTI, with Weapon::WeaponGunBase as its only
 * direct base): the gun whose charge fires a Weapon::Bullet::ChaosSpear::ChaosSpear. The charge
 * starts with fn_8005C0D4, grows by dt in every update up to 3 and is fired by fn_8005C0CC. When
 * the owner locks at the start of the charge, a charge effect is requested with m_matrix as its
 * matrix source; each update refreshes m_matrix from the owner only while the owner locks. Member
 * names and the names of the non-virtual methods are inferred from the code. */
class ChaosSpear : public WeaponGunBase {
  public:
    /* Weapon type 0x47 (the integer WeaponGunBase passes to WeaponBase). */
    ChaosSpear(const boost::weak_ptr<CharInfo> &owner);
    virtual ~ChaosSpear();
    /* Updates the base, grows the charge while charging and, when the owner locks, updates
     * m_matrix. */
    virtual void Update(float dt);
    /* Returns fn_802F1C08 (a value of the bullet's file). */
    virtual float fn_8005CF14();
    /* Returns (0, 0, -3.1875). */
    virtual Fn8040Vector3 fn_8005CEF8();
    /* Return 1 and 0. */
    virtual float fn_8005CED4();
    virtual float fn_8005CEB0();
    /* Do nothing and return -1. */
    virtual float Shoot(const Fn8040Vector3 &arg);
    virtual float ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target);
    /* Fires the charged spear and ends the charge; returns 0. */
    virtual float fn_8005C0CC(const Fn8040Vector3 &arg);
    /* Starts the charge, requests the sound 0x812F (fn_801D3EF0) and, when the owner locks,
     * requests the charge effect. */
    virtual void fn_8005C0D4();
    /* Ends the charge without firing and stops its sound and effect. */
    virtual void fn_8005C0D8();

    /* Stops the charge effect when it still exists and releases it (name inferred). */
    void StopEffect();
    /* When the owner locks, requests the charge effect (id 9) at m_matrix through fn_80305900
     * (name inferred). */
    void StartEffect();
    /* Sets m_matrix to the owner's position and rotation when the owner locks (name inferred). */
    void UpdateMatrix();

    /* 0x3C */ boost::shared_ptr<Effect::PJS::Launcher> m_launcher; /* entry 2 of fn_801D2CA4 */
    /* 0x44 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x4C */ Fn8040Matrix34 m_matrix;
    /* 0x7C */ float m_charge;     /* seconds of charge, at most 3 */
    /* 0x80 */ bool m_chargeSound; /* set once the charge reached 2.4 (fn_801D51A0 with 0xF) */
    /* 0x81 */ bool m_charging;
    /* 0x84 */ UnkSoundHandle_801D3DC8 m_sound; /* the charge sound */
};
CHECK_SIZEOF(Weapon::ChaosSpear::ChaosSpear, 0xA0);

} // namespace ChaosSpear
} // namespace Weapon

/* Creates a Weapon::ChaosSpear::ChaosSpear for owner; the function at 0x8027EFC4 gives it to its
 * player. */
extern "C" boost::shared_ptr<Weapon::ChaosSpear::ChaosSpear>
fn_802F0740(const boost::weak_ptr<CharInfo> &owner);

#endif
