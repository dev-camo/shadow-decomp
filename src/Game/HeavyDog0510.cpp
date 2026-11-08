/* HeavyDog0510 (class name from RTTI; the gadget's descriptor is "HEAVY_DOG_0510"): the boss task
 * of a Heavy Dog battle. Its Init sets up the variables and hands twenty-five control units to
 * them. The descriptor's first hook registers the boss resource file
 * "enemy/boss/HeavyDog0510One.one" under key 0x16 and fetches the launcher, the second releases
 * both (behaviour inferred). The linker keeps this file's copies of the inline constructor and
 * destructor of the variables and of the inline HeavyDog0510ModeCtrlUnit constructor. File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/BossCtrl.h"
#include "Game/BossCtrlUnit.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/HeavyDog0510.h"
#include "Game/HeavyDog0510CtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
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
static UnkSphere_80035CEC s_sphere0(Fn8040Vector3(0.0f, 0.0f, 0.0f), 35.0f);
static UnkSphere_80035CEC s_sphere1(Fn8040Vector3(0.0f, 0.0f, 0.0f), 10.0f);
static Fn8040Vector3 s_vector0(0.0f, 0.0f, 0.0f);
static UnkBossStaticTail s_unkTail;

/* The boss parameter tables that Init hands to the variables (placeholder types, see
 * BossVarParam.h). */
static const UnkBossParamRecordData s_recordData0[] = {
    {0, &s_sphere0, 0x80, 4010},
    {0, 0, 0, 0},
};

static const UnkBossParamRecordData s_recordData1[] = {
    {0, &s_sphere1, 0x100000, 4900},
    {0, 0, 0, 0},
};

static UnkBossParamRecord s_records[] = {
    {s_recordData0, -1, 4, 1.0f, 1, 1, 1, 1, 1},
    {s_recordData1, -1, 4, 1.0f, 1, 1, 1, 1, 1},
    {0, -1, 0, 1.0f, 6, 0, 0, 6, 0},
};

static UnkBossParamRecord2 s_record2[] = {
    {0x2C00, 4900, &s_vector0},
    {0, 0, 0},
};

static UnkBossParamTable s_paramTable = {3, 4, 5, 1, 1, s_records, s_record2, 0};

static GadgetParamInfo s_paramInfo[] = {
    {0},
};

extern "C" void fn_80254248(void) {
    fn_8001299C(fn_800122D4(), 0x16, new ResourceOneFile("enemy/boss/HeavyDog0510One.one", false));
    lbl_805F0240 = fn_801D2CA4(3);
}

extern "C" void fn_80254218(void) {
    fn_800127F8(fn_800122D4(), 0x16);
    lbl_805F0240.reset();
}

HeavyDog0510::HeavyDog0510(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag)
    : BossRootTask(parent, owner, flag, &m_varParam) {}

void HeavyDog0510::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new HeavyDog0510(fn_801EB304(fn_8001C4F8(), 14), owner, false);
}

extern "C" void fn_802540F4(void) {}

void HeavyDog0510::Init() {
    ReadOwner();
    m_varParam.field_0x4 = &s_paramTable;
    new BossCtrl(this, &m_varParam);

    /* field_0x8 is stored and loaded back for field_0x24, which a chained assignment does not
     * do. */
    m_varParam.field_0x8 = 140.0f;
    m_varParam.m_params.field_0x10 = 0.5f * m_varParam.m_params.field_0x60;
    m_varParam.field_0x24 = m_varParam.field_0x8;
    m_varParam.field_0xE8 = m_varParam.m_params.field_0x48;
    m_varParam.field_0x6C = lbl_805F0240;
    m_varParam.field_0xF8 = m_varParam.field_0xEC = m_varParam.m_charInfo->m_position;
    m_varParam.field_0x116 = 0;

    Fn8040Vector3 horizontal(m_varParam.field_0xEC);
    horizontal.y = 0.0f;
    m_varParam.field_0x1F4 = fn_8040CF50(&horizontal);
    m_varParam.field_0x110 = fn_80016260(-horizontal.x, -horizontal.z);
    m_varParam.field_0x104 = m_varParam.field_0x110;
    m_varParam.field_0x200 = this;

    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_AttackCallbackBody(&m_varParam, 0));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_AttackCallbackCanopy(&m_varParam, 1));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_GetStatus(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDog0510ModeCtrlUnit(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDog0510CtrlUnit_Move(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_Rotate(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4400));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4410));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4420));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4450));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4440));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4430));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_RocketLauncher(&m_varParam, 4460));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_Engine(&m_varParam, 4100));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_Engine(&m_varParam, 4200));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_Engine(&m_varParam, 4300));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_DispGauge(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckEvent(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDog0510CtrlUnit_FixLookCharCamera(&m_varParam));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_CheckFirstDamage(&m_varParam, 237, 0));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_CheckLauncherNum(&m_varParam, 3064));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_CheckMotionAttribute(&m_varParam));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_DispCanopy(&m_varParam, 6));
    m_varParam.AddCtrlUnit(new HeavyDogCtrlUnit_DispEnergy(&m_varParam, 7));
    m_varParam.AddCtrlUnit(new BossCtrlUnit_Shadow(&m_varParam, m_varParam.m_params.field_0x70));

    if (field_0x31) {
        m_varParam.field_0x14 = 2;
    } else {
        m_varParam.field_0x14 = 6;
    }
}

HeavyDog0510::~HeavyDog0510() {}

GadgetTypeInfo lbl_80549A34 = {
    "HEAVY_DOG_0510",
    fn_80254248,
    fn_80254218,
    HeavyDog0510::Create,
    fn_802540F4,
    0,
    0xB7,
    2,
    0,
    10,
    s_paramInfo,
};
