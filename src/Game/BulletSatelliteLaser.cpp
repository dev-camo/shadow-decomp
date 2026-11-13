/* BulletSatelliteLaser::SatelliteLaser (class described in Game/BulletSatelliteLaser.h) and its two
 * bullet records. File name inferred from the namespace. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/BulletSatelliteLaser.h"
#include "Game/CommonBomb.h"
#include "Game/LockonTarget.h"
#include "Game/Target.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8005E418.h"
#include "boost/shared_ptr.hpp"

namespace BulletSatelliteLaser {

SatelliteLaser::SatelliteLaser(const UnkParams_80051D10 &params, int kind)
    : Task(fn_801EB304(fn_8001C4F8(), 11)) {
    m_name = "SatelliteLaser";
    m_owner = params.m_owner;
    field_0x38 = params.field_0x34;
    m_kind = kind;
    m_timer = 1.0f;

    boost::shared_ptr<Target> target;

    if (params.m_target.get() != 0) {
        target = params.m_target->fn_80061050().lock();
    }

    boost::shared_ptr<CharInfo> owner = m_owner.lock();

    if (owner.get() == 0 || target.get() == 0) {
        m_flags |= 1;
        return;
    }

    m_position = target->fn_801E73EC(&owner->m_position);

    m_effect = fn_8005E3D0(0x28B, owner);
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect = m_effect.lock();

        fn_80425E4C(effect.get(), &m_position);
    }

    fn_801D54F4(fn_80053164(), 0xE027, &m_position, 0, 1);
}

SatelliteLaser::~SatelliteLaser() {}

void SatelliteLaser::Update(float dt) {
    m_timer -= dt;
    if (m_timer <= 0.0f) {
        Explode();
        m_flags |= 1;
    }
}

void SatelliteLaser::Explode() {
    int mask = 4;

    switch (field_0x38) {
    case 0:
        mask |= 2;
        break;
    default:
        mask |= 1;
        break;
    }

    int type;
    float value;

    switch (m_kind) {
    case 0:
        type = 0;
        value = 10.0f;
        break;
    /* The original tests 0 and 1 before the default, so both labels exist. */
    case 1:
    default:
        type = 1;
        value = 18.0f;
        break;
    }

    fn_801DD040(m_owner, &m_position, type, mask, value);
    fn_801D54F4(fn_80053164(), 0xE028, &m_position, 0, 1);
    fn_8034DF48(7, -2);
}

void SatelliteLaser::CreateKind0(const UnkParams_80051D10 *params) {
    new SatelliteLaser(*params, 0);
}

void SatelliteLaser::CreateKind1(const UnkParams_80051D10 *params) {
    new SatelliteLaser(*params, 1);
}

} // namespace BulletSatelliteLaser

/* Entries 0x3B and 0x3C of the table lbl_8051D720.
 * Matching workaround: the original stores the 300 at 0x10 into both records with the unit's
 * start-up function in .ctors, and the rest of each record is initialised data; a constant
 * initialiser gives no store. UnkStartupFloat_800E2570 stands in for the original initialiser
 * expression, which is unknown. It is written in each record because a named static float gets a
 * store of its own, which the original does not have. Revisit when the original initialiser is
 * found. */
UnkFactory_8051DDC8 lbl_8054DB28 = {
    0,
    0,
    BulletSatelliteLaser::SatelliteLaser::CreateKind0,
    0,
    UnkStartupFloat_800E2570(300.0f).value,
};

UnkFactory_8051DDC8 lbl_8054DB3C = {
    0,
    0,
    BulletSatelliteLaser::SatelliteLaser::CreateKind1,
    0,
    UnkStartupFloat_800E2570(300.0f).value,
};
