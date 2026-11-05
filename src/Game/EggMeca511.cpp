/* EggMeca511 (class name from RTTI; the gadget's descriptor is "EGG_MECA_0511"): the boss task of an
 * Egg Mecha battle. Its Init sets up the variables and hands seventeen control units to them (those
 * of EggMeca411 without EggMecaCtrlUnit_DispAura, plus two EggMecaCtrlUnit_Laser units). The
 * descriptor's first hook registers the boss resource file "enemy/boss/EggMecaOne.one" under key
 * 0x16 and fetches the launcher, the second releases both (behaviour inferred). This file also
 * defines the EggMecaVarParam destructor, so EggMecaVarParam's vtable and RTTI are emitted here.
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/BossCtrl.h"
#include "Game/BossCtrlUnit.h"
#include "Game/EggMeca511.h"
#include "Game/EggMecaCtrlUnit.h"
#include "Game/EggMecaVarParam.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PJSLauncher.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSphere_80035CEC.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

static UnkSphere_80035CEC s_sphere(Fn8040Vector3(0.0f, 0.0f, 0.0f), 25.0f);
static Fn8040Vector3 s_unitY(0.0f, 1.0f, 0.0f);

/* The boss parameter tables that Init hands to the variables (placeholder types, see
 * BossVarParam.h). */
static const UnkBossParamRecordData s_recordData[] = {
    {0, &s_sphere, 0x80, 30},
    {0, 0, 0, 0},
};

static UnkBossParamRecord s_records[] = {
    {s_recordData, -1, 4, 1.0f, 1, 1, 0, 1, 1},
    {0, 20, 4, 1.0f, 1, 1, 1, 1, 0},
    {0, -1, 0, 1.0f, 6, 0, 0, 6, 0},
};

static UnkBossParamRecord2 s_record2[] = {
    {0x2C00, 30, &s_unitY},
    {0, 0, 0},
};

static UnkBossParamTable s_paramTable = {6, 0x10, 0x13, 1, 2, s_records, s_record2, 0};

static GadgetParamInfo s_paramInfo[] = {
    {0},
};

extern "C" void fn_802EA284(void) {
    fn_8001299C(fn_800122D4(), 0x16, new ResourceOneFile("enemy/boss/EggMecaOne.one", false));
    lbl_805F0240 = fn_801D2CA4(3);
}

extern "C" void fn_802EA254(void) {
    fn_800127F8(fn_800122D4(), 0x16);
    lbl_805F0240.reset();
}

EggMeca511::EggMeca511(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag)
    : BossRootTask(parent, owner, flag, &m_varParam) {}

void EggMeca511::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new EggMeca511(fn_801EB304(fn_8001C4F8(), 14), owner, false);
}

extern "C" void fn_802E9D60(void) {}

void EggMeca511::Init() {
    ReadOwner();
    m_varParam.field_0x4 = &s_paramTable;
    new BossCtrl(this, &m_varParam);

    m_varParam.field_0x8 = 200.0f;
    m_varParam.m_params.field_0x30 = 200.0f;
    m_varParam.field_0x24 = m_varParam.field_0x8;
    m_varParam.field_0x6C = lbl_805F0240;
    fn_800091D0(&m_varParam.field_0x194,
                fn_800091D0(&m_varParam.field_0x1A0, &m_varParam.m_charInfo->m_position));

    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_AttackCallback(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_GetStatus(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaModeCtrlUnit(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_Move(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_CheckMotionAttribute(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_DispGauge(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckEvent(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_FixLookCharCamera(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckFirstDamage(&m_varParam, 212, 0));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_DispCanopy(&m_varParam, 13));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_DispEggman(&m_varParam, 10));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_DispEye(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_IronBall(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_Laser(&m_varParam, 0));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_Laser(&m_varParam, 1));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_RollUpper(&m_varParam));
    m_varParam.AddCtrlUnit(new EggMecaCtrlUnit_AdjustCircleArea(&m_varParam));

    if (field_0x31) {
        m_varParam.field_0x14 = 2;
        m_varParam.field_0x1A = 1;
    } else {
        m_varParam.field_0x14 = 14;
    }

    fn_801FAF00(fn_80109D64(), 211, 0, 0, 0);
    m_varParam.m_params.field_0x110 = 22;
    m_varParam.m_params.field_0x114 = 20;
}

EggMecaVarParam::~EggMecaVarParam() {}

EggMeca511::~EggMeca511() {}

GadgetTypeInfo lbl_8055980C = {
    "EGG_MECA_0511", fn_802EA284, fn_802EA254, EggMeca511::Create, fn_802E9D60, 0, 0xBB, 2, 0, 10,
    s_paramInfo,
};
