/* DevilDoom (class name from RTTI; the gadget's descriptor is "DEVIL_DOOM"): the boss task of the
 * DEVIL_DOOM gadget. Its Init sets up the variables and hands eighteen control units to them. The
 * descriptor's first hook registers the boss resource file "enemy/boss/DevilDoomOne.one" under key
 * 0x16 and fetches the launcher, the second releases both (behaviour inferred). The linker keeps
 * this file's copies of the inline constructors and destructor of the variables and of their
 * parameter block and array entries, of the inline DevilDoomModeCtrlUnit constructor, and of the
 * inline constructors of four of the units Init creates and of their base
 * DevilDoomCtrlUnit_AttackCallbackEye. File name inferred from the class. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/BossCtrl.h"
#include "Game/BossCtrlUnit.h"
#include "Game/DevilDoom.h"
#include "Game/DevilDoomCtrlUnit.h"
#include "Game/DevilDoomModeCtrlUnit.h"
#include "Game/DevilDoomVarParam.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PJSLauncher.h"
#include "Game/ResourceOneFile.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkBossStatics.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSphere_80035CEC.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

/* Matching workaround: placeholder storage before the spheres and after the vectors, see
 * UnkBossStatics.h. */
static UnkBossStaticHead s_unkHead;
static UnkSphere_80035CEC s_sphere0(Fn8040Vector3(0.0f, 0.0f, 0.0f), 10.0f);
static UnkSphere_80035CEC s_sphere1(Fn8040Vector3(0.0f, 0.0f, 0.0f), 35.0f);
static Fn8040Vector3 s_vector0(0.0f, 0.0f, 0.0f);
static UnkBossStaticTail s_unkTail;

/* The boss parameter tables that Init hands to the variables (placeholder types, see
 * BossVarParam.h). */
static const UnkBossParamRecordData s_recordData0[] = {
    {0, &s_sphere0, 0x100000, 1027},
    {0, 0, 0, 0},
};

static const UnkBossParamRecordData s_recordData1[] = {
    {0, &s_sphere0, 0x100000, 1017},
    {0, 0, 0, 0},
};

static const UnkBossParamRecordData s_recordData2[] = {
    {0, &s_sphere1, 0x80, 1001},
    {0, 0, 0, 0},
};

static UnkBossParamRecord s_records[] = {
    {s_recordData0, -1, 4, 1.0f, 1, 1, 0, 1, 1},
    {s_recordData1, -1, 4, 1.0f, 1, 1, 0, 1, 1},
    {s_recordData2, -1, 4, 1.0f, 1, 1, 0, 1, 1},
    {0, -1, 0, 1.0f, 6, 0, 0, 6, 0},
};

static UnkBossParamRecord2 s_record2[] = {
    {0x2C00, 1027, &s_vector0},
    {0x2C00, 1017, &s_vector0},
    {0, 0, 0},
};

static UnkBossParamTable s_paramTable = {3, 5, 13, 1, 2, s_records, s_record2, 0};

static GadgetParamInfo s_paramInfo[] = {
    {0},
};

extern "C" void fn_8030CACC(void) {
    fn_8001299C(fn_800122D4(), 0x16, new ResourceOneFile("enemy/boss/DevilDoomOne.one", false));
    lbl_805F0240 = fn_801D2CA4(3);
}

extern "C" void fn_8030CA9C(void) {
    fn_800127F8(fn_800122D4(), 0x16);
    lbl_805F0240.reset();
}

DevilDoom::DevilDoom(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag)
    : BossRootTask(parent, owner, flag, &m_varParam) {}

void DevilDoom::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DevilDoom(fn_801EB304(fn_8001C4F8(), 14), owner, false);
}

extern "C" void fn_8030C568(void) {}

void DevilDoom::Init() {
    ReadOwner();
    m_varParam.field_0x4 = &s_paramTable;
    new BossCtrl(this, &m_varParam);

    /* Two statements: the original stores field_0x8 and loads it back for field_0x24, which a
     * chained assignment does not do. */
    m_varParam.field_0x8 = 200.0f;
    m_varParam.field_0x24 = m_varParam.field_0x8;
    m_varParam.field_0x6C = lbl_805F0240;
    m_varParam.field_0x1BC = lbl_805E07A0;

    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AttackCallbackEyeR(&m_varParam, 0));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AttackCallbackEyeL(&m_varParam, 1));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AttackCallbackChest(&m_varParam, 2));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_GetStatus(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomModeCtrlUnit(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_Move(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_DispGauge(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckEvent(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_FixLookCharCamera(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AdjustEye(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AdjustNeck(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_CheckInterrupt(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_BallControl(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_WingControl(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_Cloud(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckFirstDamage(&m_varParam, 421, 0));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_CheckMotionAttribute(&m_varParam));
    m_varParam.AddCtrlUnit(new DevilDoomCtrlUnit_AdjustTarget(&m_varParam));

    if (field_0x31) {
        m_varParam.field_0x14 = 2; /* mode 2: WAIT */
    } else {
        m_varParam.field_0x14 = 12; /* mode 12: START */
    }
}

DevilDoom::~DevilDoom() {}

GadgetTypeInfo lbl_8055B7A4 = {
    "DEVIL_DOOM", fn_8030CACC, fn_8030CA9C, DevilDoom::Create, fn_8030C568, 0, 0xBE, 2, 0,
    10,           s_paramInfo,
};
