/* Weapon::Bullet::ChaosSpear::ChaosSpear (class name from RTTI): the spear that the chaos spear gun
 * fires. File name inferred from the class. Method names marked "(name inferred)" in the header are
 * not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The attack object's two callbacks are boost::function objects built with
 * boost::bind(&ChaosSpear::OnAttackCallback0, this, _1) and the same for OnAttackCallback1; their
 * manager, invoker and copy functions come from the boost headers. */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/BulletChaosSpear.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/UnkResult_80051B58.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8005E418.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;

/* The spear's speed, which is also stored in field_0x4C of the attack callback's parameters, and
 * the distance after which it destroys itself (names inferred).
 * Matching workaround: the original sets these floats with a start-up function in .ctors (a
 * constant initialiser would be emitted as initialised .sdata data instead).
 * UnkStartupFloat_800E2570 stands in for the original initialiser expression and gives that
 * start-up function. Revisit when the original initialiser is found. */
static const float s_speed = UnkStartupFloat_800E2570(500.0f).value;
static const float s_range = UnkStartupFloat_800E2570(400.0f).value;

static UnkBulletMotion_80052390 AdvanceMotion(const UnkBulletMotion_80052390 &motion, float dt);

namespace Weapon {
namespace Bullet {
namespace ChaosSpear {

ChaosSpear::ChaosSpear(const UnkParams_80051D10 &params,
                       const boost::shared_ptr<Effect::PJS::Launcher> &launcher, float charge)
    : BulletBase(params), m_launcher(launcher), m_charge(charge) {
    m_motion = fn_8005A228(params, s_speed);

    if (m_charge >= 0.9995f) {
        m_charge = 1.0f;
        m_fullCharge = true;
    } else {
        m_fullCharge = false;
    }

    field_0x5C = 14.0f * m_charge + 6.0f;

    m_attack = fn_8005A300(params.field_0x34, params.m_owner,
                           AttackCallback(boost::bind(&ChaosSpear::OnAttackCallback0, this, _1)),
                           AttackCallback(boost::bind(&ChaosSpear::OnAttackCallback1, this, _1)),
                           field_0x5C);

    StartEffect();
}

ChaosSpear::~ChaosSpear() {
    if (m_attack.get() != 0) {
        m_attack.reset();
    }

    StopEffect();
}

void ChaosSpear::Update(float dt) {
    UnkBulletMotion_80052390 motion = AdvanceMotion(m_motion, dt);

    m_distance += fn_8040D1B4(&motion.position, &m_motion.position);

    UnkResult_80051B58 hit =
        fn_8005AFE4(field_0x34, &m_motion.position, &motion.position, m_attack);

    if (hit.field_0x34) {
        PlayHitEffect(hit);
        motion.position = hit.field_0x0;
    }

    m_motion = motion;
    UpdateEffectMatrix();

    if (hit.field_0x34 || m_distance >= s_range) {
        delete this;
    }
}

} // namespace ChaosSpear
} // namespace Bullet
} // namespace Weapon

/* Returns motion with its position moved by velocity * dt (name inferred). The original has local
 * copies of the same function in three other files (USA (GUPE8P) 0x800561E4, 0x80059F5C and
 * 0x80220B94). Like every definition in this file it is placed by the reverse address order:
 * the original object has it between PlayHitEffect and Update. This is that order, not a matching
 * workaround. Measured: defined at the top of the file, or as a static inline function in a
 * header, it is emitted after Update instead. The namespaces are closed around it because its
 * symbol is at global scope. */
static UnkBulletMotion_80052390 AdvanceMotion(const UnkBulletMotion_80052390 &motion, float dt) {
    return UnkBulletMotion_80052390(motion.position + motion.velocity * dt, motion.velocity,
                                    motion.rotation);
}

namespace Weapon {
namespace Bullet {
namespace ChaosSpear {

void ChaosSpear::PlayHitEffect(const UnkResult_80051B58 &hit) {
    fn_8030536C(m_launcher.get(), &hit.field_0x0, &hit.field_0xC, m_fullCharge ? 0x13 : 0x2AB,
                fn_8005E4B8(m_owner));
}

void ChaosSpear::StartEffect() {
    int id;

    if (m_fullCharge) {
        m_effectScale = 1.0f;
        id = 10;
    } else {
        m_effectScale = 1.0f + m_charge;
        id = 3;
    }

    m_effect = fn_803056F8(m_launcher.get(), id, fn_8005E4B8(m_owner));
    UpdateEffectMatrix();
}

void ChaosSpear::StopEffect() {
    if (!m_effect.expired()) {
        m_effect.lock()->RequestStop();
        m_effect.reset();
    }
}

void ChaosSpear::UpdateEffectMatrix() {
    boost::shared_ptr<EffectElement> effect(m_effect.lock());

    if (effect.get() != 0) {
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        fn_8040B034(&translation, &m_motion.position);
        fn_8040C148(&matrix, &translation);

        Fn8040Vector2 angles = fn_8040F664(fn_80016284(m_motion.velocity));

        fn_8040B884(&matrix, angles.x);
        fn_8040B848(&matrix, angles.y);
        fn_8040B720(&matrix, m_effectScale, m_effectScale, m_effectScale);
        fn_80425E68(effect.get(), &matrix);
    }
}

void ChaosSpear::OnAttackCallback0(AttackCallbackParam &param) {
    param.field_0x34 = fn_8040CEE8(m_motion.velocity);
    param.field_0x4C = s_speed;
}

void ChaosSpear::OnAttackCallback1(AttackCallbackParam &param) {
    m_attack->fn_800532F8(param.field_0x8);

    UnkResult_80051B58 *result = param.field_0x58;

    result->field_0x38 = param.field_0x54;
    /* Kinds 1, 3 and 5 set the result's field_0x34, which makes Update stop the spear. Kind 8 is a
     * real value (ThreatObj and the Chao store it, see AttackCallbackParam) that leaves the result
     * as it is.
     * Matching workaround: the explicit empty case 8 is inferred. The original's jump table (USA
     * (GUPE8P) 0x80559F58) has nine entries, 0 to 8, and kind 8's entry shares the default exit, so
     * the table does not prove the label existed; without the label this compiler tests the kinds
     * with compares and emits no table (the function is 140 bytes against the original's 136).
     * Revisit when another form that reproduces the table is found. */
    switch (param.field_0x54) {
    case 1:
    case 3:
    case 5:
        result->field_0x34 = 1;
        break;
    case 8:
        break;
    }
}

} // namespace ChaosSpear
} // namespace Bullet
} // namespace Weapon

extern "C" float fn_802F1C08(void) {
    return s_range;
}

extern "C" void fn_802F1BB0(const UnkParams_80051D10 &params,
                            const boost::shared_ptr<Effect::PJS::Launcher> &launcher,
                            float charge) {
    new Weapon::Bullet::ChaosSpear::ChaosSpear(params, launcher, charge);
}
