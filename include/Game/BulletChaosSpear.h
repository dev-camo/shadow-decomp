#ifndef GAME_BULLETCHAOSSPEAR_H
#define GAME_BULLETCHAOSSPEAR_H

#include "Game/BulletBase.h"
#include "Game/PJSLauncher.h"
#include "Game/UnkParams_80051D10.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkResult_80051B58;

namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class AttackCallbackParam;
class CharaColliAttack;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

namespace Weapon {
namespace Bullet {
namespace ChaosSpear {

/* Weapon::Bullet::ChaosSpear::ChaosSpear (class name from RTTI, with Weapon::Bullet::BulletBase as
 * its only direct base): the spear that Weapon::ChaosSpear::ChaosSpear fires. It flies along its
 * motion with an attack object and an effect, and destroys itself in Update when the result of its
 * attack test has field_0x34 set (see UnkResult_80051B58) or once m_distance has reached the
 * distance fn_802F1C08 returns. Member names and the names of the non-virtual methods are inferred
 * from the code. */
class ChaosSpear : public BulletBase {
  public:
    /* Clamps charge: from 0.9995 on, it becomes 1 and the charge counts as full. */
    ChaosSpear(const UnkParams_80051D10 &params,
               const boost::shared_ptr<Effect::PJS::Launcher> &launcher, float charge);
    virtual ~ChaosSpear();
    /* Moves the motion's position by velocity * dt, adds the step's length to m_distance and tests
     * the attack along the step with fn_8005AFE4. When the result's field_0x34 is set, it plays the
     * hit effect and moves the position to the result's field_0x0 instead. It then calls
     * UpdateEffectMatrix and deletes the spear when field_0x34 is set or m_distance >= 400. */
    virtual void Update(float dt);

    /* The two callbacks the attack object is created with, in the order of fn_8005A300's
     * callback0 and callback1 (names inferred). The first stores the normalised velocity in the
     * parameters' field_0x34 and the speed (500) in field_0x4C. The second passes a copy of the
     * parameters' field_0x8 to fn_800532F8 of the attack object (m_attack is not tested for null),
     * copies the attack kind field_0x54 to the result's field_0x38 and, for kinds 1, 3 and 5, sets
     * the result's field_0x34 to 1. */
    void OnAttackCallback0(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void OnAttackCallback1(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Only when the weak reference m_effect locks to a non-null effect (tested at USA (GUPE8P)
     * 0x802F1D24-0x802F1D2C; otherwise it writes nothing): builds a matrix at the motion's
     * position, applies fn_8040B884 with the first and fn_8040B848 with the second angle that
     * fn_8040F664 gives for the negated velocity, scales it by m_effectScale and passes it to
     * fn_80425E68 (name inferred). */
    void UpdateEffectMatrix();
    /* When the effect has not expired, requests its stop and releases it (name inferred). */
    void StopEffect();
    /* Sets m_effectScale (1 for a full charge, otherwise 1 + m_charge), requests the effect with
     * id 10 for a full charge, otherwise 3, stores the returned weak reference (which can be
     * empty) in m_effect and calls UpdateEffectMatrix, which updates the matrix only while that
     * reference locks (name inferred). */
    void StartEffect();
    /* Requests an effect with the result's field_0x0 as position and field_0xC as direction (id
     * 0x13 for a full charge, otherwise 0x2AB) and does not keep it (name inferred). */
    void PlayHitEffect(const UnkResult_80051B58 &hit);

    /* 0x3C */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x44 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x4C */ float m_effectScale;
    /* 0x50 */ boost::shared_ptr<Effect::PJS::Launcher> m_launcher;
    /* The constructor's charge, set to 1 from 0.9995 on. */
    /* 0x58 */ float m_charge;
    /* 14 * m_charge + 6, passed as fn_8005A300's last argument. */
    /* 0x5C */ float field_0x5C;
    /* 0x60 */ bool m_fullCharge;
};
CHECK_SIZEOF(Weapon::Bullet::ChaosSpear::ChaosSpear, 0x64);

} // namespace ChaosSpear
} // namespace Bullet
} // namespace Weapon

/* Functions of the same file that Weapon::ChaosSpear::ChaosSpear calls. Their names are
 * placeholders and the parameter names and const reference spellings are inferred. */
extern "C" {
/* Allocates 0x64 bytes and, when that succeeds, constructs a Weapon::Bullet::ChaosSpear::ChaosSpear
 * there with params, launcher and charge. The void return is inferred: the body leaves the
 * allocation or constructor result in r3, and the only caller discards it. */
void fn_802F1BB0(const UnkParams_80051D10 &params,
                 const boost::shared_ptr<Effect::PJS::Launcher> &launcher, float charge);
/* Returns the distance after which the spear destroys itself (400, set by the file's start-up
 * code). */
float fn_802F1C08(void);
}

#endif
