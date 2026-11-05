/* The DefenseWall gadget (class and behaviour described in Game/DefenseWall.h). File name inferred
 * from the class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The probe's and the model attack object's callbacks are boost::function objects built with
 * boost::bind(&SetDefenseWall::OnCollision, this, _1) and boost::bind(&SetDefenseWall::OnAttack,
 * this, _1); their managers, invokers and copy functions come from the boost headers. */
#include "Game/DefenseWall.h"
#include "Game/CharaColliBase.h"
#include "Game/CharInfo.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The height above the position at which the wall starts (name inferred).
 * Matching workaround: the original sets this const float with a start-up function in .ctors (a
 * constant initialiser would be emitted as initialised .sdata data instead).
 * UnkStartupFloat_800E2570 stands in for the original initialiser expression and gives the
 * start-up function; the const qualifier is what lets ReadOwner load it before the store to
 * m_recoveryTime, as the original does (a non-const object may alias that store). Revisit when
 * the original initialiser is found. */
static const float s_startOffset = UnkStartupFloat_800E2570(80.0f).value;

namespace DefenseWall {

const char *SetDefenseWall::s_name = "SetDefenseWall";

SetDefenseWall::SetDefenseWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_probeRotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x7E, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(1, "CONCRETEDOOR.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetDefenseWall::OnAttack, this, _1));
    }

    UpdateModelMatrix();
    SetupProbe();
}

SetDefenseWall::~SetDefenseWall() {}

void SetDefenseWall::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }

    m_recoveryTime -= dt;
    if (m_recoveryTime <= 0.0f) {
        m_recoveryTime = 0.0f;
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        break;

    case 2:
        m_offset -= m_params.speed * dt;
        if (m_offset <= 0.0f) {
            m_offset = 0.0f;
        }

        fn_800091D0(&m_charInfo->m_position, &m_position);
        m_charInfo->m_position.y += m_offset;

        if (m_offset <= 0.0f) {
            m_state = 3;
            fn_801D54F4(fn_80053164(), 0x7847, &m_charInfo->m_position, 0, 1);
        } else {
            fn_801D53C4(fn_80053164(), 0x7849, &m_charInfo->m_position, this, 3, 0, 2);
        }
        break;

    case 3:
        break;

    case 4:
        fn_801D54F4(fn_80053164(), 0x7848, &m_charInfo->m_position, 0, 1);
        StartCrashEffect();
        fn_8015A794(this);
        m_state = 5;
        break;

    case 5:
        m_flags |= 1;
        break;
    }

    UpdateModelMatrix();
}

void SetDefenseWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_handle.get() != 0) {
        m_handle->m_probe->SetPosition(&m_probePosition);
        m_handle->m_probe->SetRotation(m_probeRotation);
        m_handle->m_probe->SetSize(m_params.width, -2, m_params.height, m_params.depth);
        fn_8015BE64(m_handle.get());
    }
}

void SetDefenseWall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_state = 0;
    m_health = 4.0f;
    m_recoveryTime = 0.0f;
    m_offset = s_startOffset;

    fn_800091D0(&m_charInfo->m_position, &m_position);
    m_charInfo->m_position.y += m_offset;

    fn_800091D0(&m_probePosition, &m_position);
    m_probeRotation = m_charInfo->m_rotation;
}

void SetDefenseWall::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

void SetDefenseWall::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_recoveryTime > 0.0f) {
        return;
    }
    if (!IsDamageable()) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        m_health -= param.field_0x50;
        if (m_health <= 0.0f) {
            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                fn_8020642C(attacker, 0);
            }
            param.field_0x54 = 3;
            m_state = 4;
        } else if (param.field_0x50 > 0.9f) {
            m_model->fn_8016268C(0.6f);
            m_recoveryTime = 0.2f;
            param.field_0x54 = 1;
        } else {
            param.field_0x54 = 5;
        }
    }
}

void SetDefenseWall::SetupProbe() {
    Fn8040Vector3 sizes(m_params.width, m_params.height, m_params.depth);

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7D04(fn_80021B08(), &m_probePosition, sizes, m_probeRotation, 2, 0x40100, 10, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), lbl_805E07A0, &lbl_805E07B8, 0);
    m_handle = fn_8015B8C4(probe);

    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&SetDefenseWall::OnCollision, this, _1)));
}

void SetDefenseWall::StartCrashEffect() {
    UnkResourceManager_800122D4 *manager = fn_800122D4();
    UnkResource_80012700 *resource =
        static_cast<UnkResource_80012700 *>(fn_80012700(manager, 1, "CONCRETEDOOR.BIN"));
    boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> crashInfo =
        GadgetBuilder::fn_80208978(resource, 0);

    if (crashInfo.get() != 0) {
        const UnkCrashEntry_802086C0 *entry = crashInfo->fn_802086C0();

        if (entry != 0) {
            RpClump *clump = static_cast<RpClump *>(fn_80012700(manager, 1, entry->fileName));

            if (clump != 0) {
                UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

                parameters.field_0x4.x += entry->field_0x18.x;
                parameters.field_0x4.y += entry->field_0x18.y;
                parameters.field_0x4.z += entry->field_0x18.z;
                parameters.field_0x34 = 1.3f;
                parameters.field_0x44 = clump;
                parameters.field_0x54 = 1;
                parameters.field_0x4C = 150;
                parameters.field_0x40 = m_charInfo->m_position.y;
                parameters.field_0x4A = 1;
                fn_8015D93C(&parameters);
            }
        }
    }
}

void SetDefenseWall::OnCollision(ColliDetectArgs &args) {
    if (m_state == 1) {
        m_state = 2;
    }
}

} // namespace DefenseWall

extern "C" void fn_80229764(void) {
    fn_8006B474("CONCRETEDOOR.BIN", 1);
}

extern "C" void fn_80229760(void) {}

void DefenseWall::SetDefenseWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetDefenseWall(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "width", 0, 0, 0, 40.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "height", 0, 0, 0, 40.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "depth", 0, 0, 0, 40.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "speed", 0, 0, 0, 4.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80546A28 = {
    "DEFENSE WALL",
    fn_80229764,
    fn_80229760,
    DefenseWall::SetDefenseWall::Create,
    0,
    0,
    0x1772,
    1,
    0,
    10,
    s_paramInfo,
};
