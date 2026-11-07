/* BkWingSmall, the BK_WINGSMALL enemy (class names from RTTI); original filename unknown. */
#include "Game/BkWingSmall.h"
#include "Game/Gadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_801A35AC.h"
#include "Game/fn_8040_random.h"

/* Parameter groups passed to EnemyParamCommon's slot 0x40 (layout from its readers). */
const UnkEnemyParaEntry_800C9178 lbl_804C9480[2] = {
    {1, -1, -1, 30, 0},
    {1, -1, -1, 0, 1},
};
const UnkEnemyParaEntry_800C9178 lbl_805F5368[1] = {
    {3, -1, -1, 30, 0},
};
const UnkEnemyParaEntry_800C9178 lbl_805F5370[1] = {
    {4, -1, -1, 0, 1},
};
const UnkEnemyParaEntry_800C9178 lbl_805F5378[1] = {
    {2, -1, -1, 0, 0},
};
const UnkEnemyParaGroup_800C961C lbl_804C9490[4] = {
    {lbl_804C9480, 2, -3},
    {lbl_805F5368, 1, -3},
    {lbl_805F5370, 1, -3},
    {lbl_805F5378, 1, -3},
};
const UnkEnemyParaData_8017B8E4 lbl_805F5380 = {lbl_804C9490, 4};

const Unk801A35ACEntry lbl_805F5388[1] = {
    {3, -1},
};

BkWingSmallBase::BkWingSmallBase() : FlyerCommonBase(7) {
    m_enemy = this;
    fn_8019B4BC(&lbl_805F5380);
    m_status->fn_8017B788(0);
    field_0x348 = fn_8040EC80(8.0f, 12.0f);
}

BkWingSmallBase::~BkWingSmallBase() {}

int BkWingSmallBase::fn_80191A58() {
    return 90;
}

void BkWingSmallBase::fn_801919C0() {
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    int key = Enemy::field_0x24;
    void *resourceData = fn_80012684(resources, key, 4);
    u32 resourceValue = fn_80012608(resources, key, 6);
    Unk801A35ACDescriptor parts;
    parts.m_entries = lbl_805F5388;
    parts.m_count = 1;
    EnemyDispCommon::field_0x14 = fn_802CDB08(key, &parts, resourceData, resourceValue);
}

void BkWingSmallBase::fn_8019B3A8(float dt) {
    EnemyDispCommon::fn_8019B3A8(dt);
    if (fn_8017B750() && !fn_8017B778()) {
        fn_8019A954(this, 0x6041, true);
    }
    const Fn8040Vector3 *position = m_enemy->m_control->fn_8017C3C8();
    if (m_enemy->m_boundary.fn_801A68C0(*position)) {
        if ((field_0x348 -= dt) < 0.0f) {
            field_0x348 = fn_8040EC80(8.0f, 12.0f);
            fn_8019A954(this, 0x603E, true);
        }
    }
}

void BkWingSmallBase::fn_801812C8() {
    EnemyDispCommon::fn_801812C8();
    fn_8019AA40(this, 0x603B, true);
}

void BkWingSmallBase::fn_8019B414() {
    EnemyDispCommon::fn_8019B414();
    fn_8019AA40(this, 0x6040, true);
}

void BkWingSmallBase::fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner) {
    fn_8019B360(owner);
    fn_8019B404(*fn_8017C690(this), *fn_8017C684(this));
    m_control->fn_8017C680();
}

void BkWingSmallBase::fn_8019B360(UnkGadgetOwner_800C9CD0 *owner) {
    TEnemySetBase::fn_8019B360(owner);
    fn_800C9DBC(owner, &m_setData, sizeof(m_setData));
    fn_80193790(&m_setData);
}

bool BkWingSmallBase::fn_8019B40C(UnkEnemyDispEvent_8019B40C *event) {
    if (EnemyDispCommon::fn_8019B40C(event)) {
        return true;
    }
    switch (event->field_0x0) {
    case 6:
        fn_8019A9DC(this, 0x603C, true);
        break;
    }
    return true;
}

int BkWingSmallBase::fn_8017E14C() const {
    return 0;
}

int BkWingSmallBase::fn_8017EE98() {
    return fn_8017E14C();
}

int BkWingSmallBase::fn_8019B4C8() {
    return fn_8017E14C();
}

int BkWingSmallBase::fn_8018019C() {
    return 1;
}

int BkWingSmallBase::fn_8017FA78() {
    return 2;
}

int BkWingSmallBase::fn_8019B4D0() {
    return 3;
}

int BkWingSmallBase::fn_80180C58() {
    return 3;
}

void BkWingSmallBase::fn_80183AD4() {}

void BkWingSmallBase::fn_80183A58() {}

float BkWingSmallBase::fn_8019166C() {
    return 120.0f;
}

/* Label callback of the table's AppearType to AttackEnd entries: returns an empty label. */
extern "C" const char *fn_80191664(UnkParamCursor_800CBC20 *cursor) {
    return "";
}

BkWingSmall::BkWingSmall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : EnemyTemplate<BkWingSmallBase, BkWingSmallAI>(parent, owner) {
    m_enemy = this;
    m_name = s_name;
}

BkWingSmall::~BkWingSmall() {}

extern "C" void fn_8019130C() {
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    fn_8001299C(resources, 14, new ResourceOneFile("enemy/BkWingSmallData.one", false));
}

extern "C" void fn_801912E4() {
    fn_800127F8(fn_800122D4(), 14);
}

void BkWingSmall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BkWingSmall(fn_801EB304(fn_8001C4F8(), 14), owner);
}

GadgetParamInfo lbl_80533840[] = {
    {4, "MoveRange", 0, 0, 0, 200.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchRange", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchAngle", 0, 0, 0, 0.0f, -360.0f, 360.0f, fn_800CBC20, 0},
    {4, "SearchWidth", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchHeight", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchHeightOffset", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveSpeedRatio", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "AppearType", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_80191664, 0},
    {1, "ActionType", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_80191664, 0},
    {1, "", 0, 0, 8, 0.0f, 0.0f, 0.0f, fn_80191664, 0},
    {4, "FloatWidth", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_80191664, 0},
    {4, "AttackStart", 0, 0, 0, 0.4f, 0.0f, 1.0f, fn_80191664, 0},
    {4, "AttackEnd", 0, 0, 0, 0.6f, 0.0f, 1.0f, fn_80191664, 0},
    {1, "PathMirror", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, 0},
};

GadgetTypeInfo lbl_80533A98 = {
    "BK_WINGSMALL", fn_8019130C, fn_801912E4, BkWingSmall::Create, 0, 0, 0x8F, 2, 0, 5,
    lbl_80533840,
};

const char *BkWingSmall::s_name = "BkWingSmall";
