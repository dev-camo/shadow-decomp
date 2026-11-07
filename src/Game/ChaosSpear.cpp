/* Weapon::ChaosSpear::ChaosSpear (class name from RTTI): the gun that charges and fires a chaos
 * spear. File name inferred from the class. Method names marked "(name inferred)" in the header are
 * not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/ChaosSpear.h"
#include "Game/BulletChaosSpear.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8005E418.h"
#include "Game/fn_8040_vector_types.h"

/* The longest charge in seconds (name inferred). */
static const float kMaxCharge = 3.0f;

namespace Weapon {
namespace ChaosSpear {

ChaosSpear::ChaosSpear(const boost::weak_ptr<CharInfo> &owner) : WeaponGunBase(owner, 0x47) {
    m_charge = 0.0f;
    m_chargeSound = false;
    m_charging = false;
    fn_801D2DE8(2);
    m_launcher = fn_801D2CA4(2);
}

ChaosSpear::~ChaosSpear() {
    fn_801D2D14(2);
    StopEffect();
    m_sound.fn_801D455C();
}

void ChaosSpear::Update(float dt) {
    WeaponGunBase::Update(dt);

    if (m_charging) {
        m_charge = fn_800704AC(m_charge + dt, kMaxCharge);

        if (!m_chargeSound && m_charge >= 2.4f) {
            m_chargeSound = true;
            fn_801D51A0(fn_80053164(), 0xF, -1, -1);
        }
    }

    UpdateMatrix();
}

float ChaosSpear::fn_8005CF14() {
    return fn_802F1C08();
}

Fn8040Vector3 ChaosSpear::fn_8005CEF8() {
    return Fn8040Vector3(0.0f, 0.0f, -3.1875f);
}

float ChaosSpear::Shoot(const Fn8040Vector3 &arg) {
    return -1.0f;
}

float ChaosSpear::ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target) {
    return -1.0f;
}

float ChaosSpear::fn_8005C0CC(const Fn8040Vector3 &arg) {
    Fn8040Vector3 position;
    UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

    fn_8005CF90(&position, &rotation);

    Fn8040Vector3 direction(0.0f, 0.0f, 1.0f);
    direction = fn_8040D044(direction, rotation.z);
    direction = fn_8040D118(direction, rotation.x);
    direction = fn_8040D0AC(direction, rotation.y);

    boost::shared_ptr<CharInfo> charInfo(GetOwner().lock());
    /* The block's arg2: 0 when the owner locks and its field_0x5C is 1, else 1 (as in
     * fn_80069424). */
    int arg2;
    if (charInfo.get() != 0 && charInfo->field_0x5C == 1) {
        arg2 = 0;
    } else {
        arg2 = 1;
    }

    float charge = m_charge / kMaxCharge;
    fn_802F1BB0(UnkParams_80051D10(charInfo, arg2, position, direction, arg), m_launcher, charge);
    m_sound.fn_801D455C();

    /* A full charge passes 0x10 to fn_801D51A0, requests sound 0x8130, passes 3 to fn_8034DF48
     * and requests effect 0xF at the scale 1; a shorter one requests sound 0x812E, passes 0 and
     * requests effect 0 at the scale 1 + charge. fn_8034DF48 is called only when the owner locks
     * again; fn_801D560C plays the sound only when it obtains a sound object, and fn_803055F8
     * applies the matrix only when the effect locks. */
    float scale;
    int object;
    int sound;
    int effect;
    if (charge >= 1.0f) {
        scale = 1.0f;
        sound = 0x8130;
        effect = 0xF;
        object = 3;
        fn_801D51A0(fn_80053164(), 0x10, -1, -1);
    } else {
        scale = 1.0f + charge;
        sound = 0x812E;
        effect = 0;
        object = 0;
    }

    {
        boost::shared_ptr<CharInfo> owner(GetOwner().lock());
        if (owner.get() != 0) {
            fn_8034DF48(object, owner->field_0x54);
        }
    }

    fn_801D560C(fn_80053164(), sound, 0, 0);

    Fn8040Matrix34 matrix;
    Fn8040Matrix34 translation;
    fn_8040B034(&translation, &position);
    fn_8040C148(&matrix, &translation);
    fn_8040B848(&matrix, rotation.y);
    fn_8040B884(&matrix, rotation.x);
    fn_8040B80C(&matrix, rotation.z);
    fn_8040B720(&matrix, scale, scale, scale);
    fn_803055F8(m_launcher.get(), &matrix, effect, fn_8005E4B8(charInfo));

    fn_8005EB94(1);
    m_charge = 0.0f;
    m_chargeSound = false;
    m_charging = false;
    StopEffect();

    return 0.0f;
}

void ChaosSpear::fn_8005C0D4() {
    m_charge = 0.0f;
    m_chargeSound = false;
    m_charging = true;
    m_sound = fn_801D3EF0(0x812F, this);
    StopEffect();
    StartEffect();
}

void ChaosSpear::fn_8005C0D8() {
    m_charge = 0.0f;
    m_chargeSound = false;
    m_charging = false;
    m_sound.fn_801D455C();
    StopEffect();
}

void ChaosSpear::UpdateMatrix() {
    boost::shared_ptr<CharInfo> charInfo(GetOwner().lock());

    if (charInfo.get() != 0) {
        Fn8040Matrix34 translation;
        MakeTranslationMatrix(&translation, charInfo->m_position);
        fn_8040C148(&m_matrix, &translation);

        Fn8040Matrix34 rotated;
        fn_8040FFBC(&rotated, &m_matrix, &charInfo->m_rotation, fn_801DBD00(charInfo->field_0x30));
        fn_8040C148(&m_matrix, &rotated);
    }
}

void ChaosSpear::StartEffect() {
    boost::shared_ptr<CharInfo> charInfo(GetOwner().lock());

    if (charInfo.get() != 0) {
        m_effect = fn_80305900(m_launcher.get(), &m_matrix, 9, fn_8005E4B8(charInfo));
    }
}

void ChaosSpear::StopEffect() {
    if (!m_effect.expired()) {
        m_effect.lock()->RequestStop();
        m_effect.reset();
    }
}

} // namespace ChaosSpear
} // namespace Weapon

extern "C" boost::shared_ptr<Weapon::ChaosSpear::ChaosSpear>
fn_802F0740(const boost::weak_ptr<CharInfo> &owner) {
    return boost::shared_ptr<Weapon::ChaosSpear::ChaosSpear>(
        new Weapon::ChaosSpear::ChaosSpear(owner));
}

namespace Weapon {
namespace ChaosSpear {

float ChaosSpear::fn_8005CEB0() {
    return 0.0f;
}

float ChaosSpear::fn_8005CED4() {
    return 1.0f;
}

} // namespace ChaosSpear
} // namespace Weapon
