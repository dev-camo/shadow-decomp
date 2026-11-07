/* BkWorm, the BK_WORM enemy (class names from RTTI); original filename unknown. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/BkWorm.h"
#include "Game/AngleConversion.h"
#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/EnemyCharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/LockonTarget.h"
#include "Game/PlayerBase.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_801A35AC.h"
#include "Game/fn_80221BA8.h"
#include "Game/fn_8040_random.h"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;

/* The model frames (name inferred) each weapon type fires from. */
const int lbl_805F53B0[2] = {0xBCE, 0xBCC};
const int lbl_804C97A8[6] = {0xBCE, 0xBCC, 0xBCD, 0xBD8, 0xBD6, 0xBD7};
const int lbl_805F53B8[2] = {0xBCE, 0xBCD};

const UnkBkWormWeapon_804C97C0 lbl_804C97C0[3] = {
    {lbl_805F53B0, 2, 0x15},
    {lbl_804C97A8, 6, 0x16},
    {lbl_805F53B8, 2, 0x17},
};

/* Parameter groups passed to EnemyParamCommon's slot 0x40 (layout from its readers). */
const UnkEnemyParaEntry_800C9178 lbl_805F53C0[1] = {
    {4, -1, -1, 30, 1},
};
const UnkEnemyParaEntry_800C9178 lbl_805F53C8[1] = {
    {3, 160, 160, 0, 0},
};
const UnkEnemyParaEntry_800C9178 lbl_805F53D0[1] = {
    {1, -1, -1, 0, 0},
};
const UnkEnemyParaEntry_800C9178 lbl_805F53D8[1] = {
    {5, -1, -1, 30, 0},
};
const UnkEnemyParaEntry_800C9178 lbl_805F53E0[1] = {
    {2, -1, -1, 0, 0},
};
const UnkEnemyParaEntry_800C9178 lbl_805F53E8[1] = {
    {3, -1, -1, 30, 0},
};
const UnkEnemyParaGroup_800C961C lbl_804C97E4[6] = {
    {lbl_805F53C0, 1, -3}, {lbl_805F53D0, 1, -3}, {lbl_805F53C8, 1, -1},
    {lbl_805F53D8, 1, -3}, {lbl_805F53E0, 1, -3}, {lbl_805F53E8, 1, -1},
};
const UnkEnemyParaData_8017B8E4 lbl_805F53F0 = {lbl_804C97E4, 6};

/* The model part and the model resource index of each weapon type. */
const Unk801A35ACEntry lbl_804C9820[3] = {
    {3, -1},
    {4, -1},
    {5, -1},
};
const int lbl_804C982C[3] = {6, 7, 8};

/* The second vector BkWormBase passes to fn_801A1F54 with its effects (constructed at startup).
 * Its middle value is 90 degrees converted to radians, so it is probably a set of angles. */
static Fn8040Vector3 s_effectAngles(0.0f, fn_8000AB90(90.0f), 0.0f);

BkWormBase::BkWormBase() : EnemyWalkerCommon(8) {
    m_enemy = this;
    fn_8017B788(0);
    field_0x5C = false;
    field_0x2B0 = fn_8040EC80(8.0f, 12.0f);
}

BkWormBase::~BkWormBase() {
    boost::shared_ptr<EffectElement> effect = field_0x298.lock();
    if (effect) {
        effect->SetMatrixSource(0);
        effect->RequestStop();
    }
    for (int i = 0; i < 2; i++) {
        fn_80192744(i);
    }
    for (int i = 0; i < 6; i++) {
        field_0x2B4[i].reset();
    }
}

int BkWormBase::fn_80191A58() {
    return 92;
}

void BkWormBase::fn_801A7FA0() {
    const Fn8040Vector3 *position = fn_8017C690(m_enemy);
    if (m_enemy->m_boundary.fn_801A68C0(*position)) {
        fn_8017B710(1);
    }
}

void BkWormBase::fn_801A8020(AttackCallbackParam *param) {
    if (param->field_0x14 != 0 && param->field_0x14->field_0x20 == 0xBBF) {
        EnemyStatusCommon::fn_801A8020(param);
        return;
    }
    if (!param->field_0x0.expired()) {
        if (param->field_0x0.lock()->field_0x40 & 0x2B0) {
            EnemyStatusCommon::fn_801A8020(param);
            return;
        }
    }
    fn_8019AA40(this, 0x6025, true);
    param->field_0x54 = 5;
}

void BkWormBase::fn_801919C0() {
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    int key = Enemy::field_0x24;
    void *resourceData = fn_80012684(resources, key, lbl_804C982C[m_setData.weaponType]);
    u32 resourceValue = fn_80012608(resources, key, 10);
    Unk801A35ACDescriptor parts;
    parts.m_entries = &lbl_804C9820[m_setData.weaponType];
    parts.m_count = 1;
    EnemyDispCommon::field_0x14 = fn_802CDB08(key, &parts, resourceData, resourceValue);
}

void BkWormBase::fn_801A84DC() {
    // Declared ahead of their first use as the original compile needs: initialising them at their
    // declarations (same statement order) swaps their registers, r29 and r30, in four words.
    int weaponType;
    Fn8040Matrix34 *frame;

    EnemyStatusCommon::fn_801A84DC();
    frame = fn_8019448C(0xBBF);
    weaponType = m_setData.weaponType;
    fn_80221BA8(fn_800496F8(frame), *fn_8017C684(this), Fn8040Vector3(0.0f, 40.0f, 0.0f),
                lbl_804C97C0[weaponType].kind, -1);
}

void BkWormBase::fn_8019B3A8(float dt) {
    EnemyWalkerCommon::fn_8019B3A8(dt);
    if (!fn_8017B750() && fn_8017B738() && !fn_8017B760()) {
        fn_801D54F4(fn_80053164(), 0x602A, fn_8017C690(m_enemy), 0, 1);
    } else {
        const Fn8040Vector3 *position = m_enemy->m_control->fn_8017C3C8();
        if (m_enemy->m_boundary.fn_801A68C0(*position)) {
            if ((field_0x2B0 -= dt) < 0.0f) {
                field_0x2B0 = fn_8040EC80(8.0f, 12.0f);
                fn_8019A954(this, 0x6029, true);
            }
        }
    }
    if (m_enemy->m_status->fn_8017B728()) {
        boost::shared_ptr<EffectElement> effect = field_0x298.lock();
        if (effect) {
            effect->RequestStop();
        }
    } else if (field_0x298.expired()) {
        Fn8040Matrix34 *frame = fn_8019448C(0xBBF);
        field_0x298 = fn_801A1E48(frame, 0x6D);
    }
}

int BkWormBase::fn_8017E14C() const {
    return m_enemy->m_status->fn_8017B720() ? 0 : 1;
}

int BkWormBase::fn_8019B4D0() {
    return 4;
}

void BkWormBase::fn_801834D4() {
    Fn8040Matrix34 *frame = fn_8019448C(m_enemy->m_parameter->fn_80192B58());
    if (frame) {
        fn_801D54F4(fn_80053164(), 0x6009, fn_8017C690(m_enemy), 0, 1);
        fn_801D54F4(fn_80053164(), 0x602B, fn_8017C690(m_enemy), 0, 1);
        fn_801A1ED8(frame, 0x6E);
    }
}

bool BkWormBase::fn_8019B40C(UnkEnemyDispEvent_8019B40C *event) {
    if (EnemyDispCommon::fn_8019B40C(event)) {
        return true;
    }
    switch (event->field_0x0) {
    case 5:
        fn_8000D8EC(fn_80009548(), 0.5f, 0, 0);
        fn_8034DE08(8, fn_8017C690(this), -2);
        fn_8019AA40(this, 0x6023, true);
        if (lbl_8057D6A0.field_0xA8.field_0x0 == 0x259) {
            fn_801A1F54(fn_8017C690(m_enemy), &s_effectAngles, 0x293);
        } else {
            fn_801A1F54(fn_8017C690(m_enemy), &s_effectAngles, 0x85);
        }
        break;
    case 19:
        fn_8019AA40(this, 0x6027, true);
        break;
    case 20:
        fn_8019AA40(this, 0x6026, true);
        fn_80192394();
        break;
    case 9:
        fn_80192268();
        break;
    case 10:
        fn_8000D8EC(fn_80009548(), 0.5f, 0, 0);
        fn_8034DE08(8, fn_8017C690(this), -2);
        fn_80192744(1);
        fn_801D54F4(fn_80053164(), 0x6023, &m_enemy->field_0x6C, 0, 1);
        if (lbl_8057D6A0.field_0xA8.field_0x0 == 0x259) {
            fn_801A1F54(&m_enemy->field_0x6C, &s_effectAngles, 0x293);
        } else {
            fn_801A1F54(&m_enemy->field_0x6C, &s_effectAngles, 0x85);
        }
        break;
    }
    return true;
}

void BkWormBase::fn_801927CC(const Fn8040Vector3 *position, int index) {
    fn_80192744(index);
    field_0x2A0[index] = fn_801A1F54(position, &s_effectAngles, 0x83);
}

void BkWormBase::fn_80192744(int index) {
    boost::shared_ptr<EffectElement> effect = field_0x2A0[index].lock();
    if (effect) {
        effect->RequestStop();
    }
}

void BkWormBase::fn_80192608(Fn8040Matrix34 *frame,
                             const boost::shared_ptr<Weapon::LockonTarget> &target, int kind) {
    if (frame) {
        Fn8040Vector3 position;
        fn_8040FAA0(&position, frame);
        Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);
        direction = fn_8040B618(*frame, direction);
        UnkParams_80051D10 params(boost::weak_ptr<CharInfo>(fn_8017D014(m_enemy)), target, 1,
                                  position, direction, lbl_805E07A0);
        fn_80051C34(kind, &params);
    }
}

void BkWormBase::fn_80192394() {
    const UnkBkWormWeapon_804C97C0 &weapon = lbl_804C97C0[m_setData.weaponType];
    for (int i = 0; i < weapon.count; i++) {
        field_0x2B4[i] = boost::shared_ptr<Weapon::LockonTarget>(new Weapon::LockonTarget());
        boost::weak_ptr<CharInfo> target = fn_800A8B74(fn_80074194(), lbl_805EE588);
        boost::shared_ptr<CharInfo> character = target.lock();
        if (character) {
            boost::shared_ptr<Player::PlayerBase> player =
                boost::dynamic_pointer_cast<Player::PlayerBase>(character);
            if (player && player->field_0x24C != 0) {
                field_0x2B4[i]->fn_80061054(player->field_0x24C->fn_80206574());
                field_0x2B4[i]->fn_80060F70();
            }
        }
    }
}

void BkWormBase::fn_80192268() {
    const UnkBkWormWeapon_804C97C0 &weapon = lbl_804C97C0[m_setData.weaponType];
    for (int i = 0; i < weapon.count; i++) {
        Fn8040Matrix34 *frame = fn_8019448C(weapon.frames[i]);
        if (frame) {
            fn_80192608(frame, field_0x2B4[i], weapon.kind);
            field_0x2B4[i].reset();
            if (i & 1) {
                fn_8019AA40(this, 0x6028, true);
            }
        }
    }
}

bool BkWorm::fn_80187CBC() {
    if (TEnemySetTask::field_0x2C != 0 && fn_800C9E80(TEnemySetTask::field_0x2C, 0x400000)) {
        return false;
    }
    return field_0x5F;
}

static const char *s_weaponTypeLabels[3] = {
    "WeaponType : NORMAL",
    "WeaponType : WIDE",
    "WeaponType : BIG",
};

/* Label callback of the WeaponType entry: for entry 7 (WeaponType) it returns the label of the
 * entry's value, for any other entry an empty string. */
extern "C" const char *fn_801921E8(UnkParamCursor_800CBC20 *cursor) {
    switch (cursor->m_index) {
    case 7:
        return s_weaponTypeLabels[cursor->m_value];
    }
    return "";
}

BkWorm::BkWorm(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : EnemyTemplate<BkWormBase, BkWormAI>(parent, owner) {
    m_enemy = this;
    m_name = s_name;
    fn_8019B4BC(&lbl_805F53F0);
}

BkWorm::~BkWorm() {}

void BkWorm::fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner) {
    int weaponType = m_setData.weaponType;
    fn_8019B360(owner);
    fn_8019B404(*fn_8017C690(m_enemy), *fn_8017C684(m_enemy));
    m_control->fn_8017C680();
    if (weaponType != m_setData.weaponType) {
        fn_801919C0();
    }
}

void BkWorm::fn_8019B360(UnkGadgetOwner_800C9CD0 *owner) {
    TEnemySetBase::fn_8019B360(owner);
    fn_800C9DBC(owner, &m_setData, sizeof(m_setData));
}

extern "C" void fn_80191D78() {
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    fn_8001299C(resources, 15, new ResourceOneFile("enemy/BkWormData.one", false));
}

extern "C" void fn_80191D50() {
    fn_800127F8(fn_800122D4(), 15);
}

void BkWorm::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BkWorm(fn_801EB304(fn_8001C4F8(), 14), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "MoveRange", 0, 0, 0, 200.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchRange", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchAngle", 0, 0, 0, 0.0f, -360.0f, 360.0f, fn_800CBC20, 0},
    {4, "SearchWidth", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchHeight", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchHeightOffset", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveSpeedRatio", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "WeaponType", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_801921E8, 0},
    {1, "AttackTimes", 1, 1, 100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "IntervalSec", 0, 0, 0, 3.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, 0},
};

GadgetTypeInfo lbl_80534AE4 = {
    "BK_WORM", fn_80191D78, fn_80191D50, BkWorm::Create, 0, 0, 0x90, 2, 0, 5, s_paramInfo,
};

const char *BkWorm::s_name = "BkWorm";

void BkWormBase::fn_801A128C() {}

void BkWormBase::fn_8019B418() {}
