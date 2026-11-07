/* DefenseSystem (class name from RTTI; the gadget's descriptor is "DEFENSE_SYSTEM"). File name
 * inferred from the class. Method names marked "(name inferred)" in the header are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
/* The unit's .rodata starts with the type names that TNumEdit.h and CharaColliBaseCharInfo.h
 * produce, in that order (see those headers). The headers also add RTTI records and base lists of
 * TNumEdit and CharaColliBase to .data and .sdata that the original unit does not have.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/DefenseSystem.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/AngleConversion.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The id that UpdateBody passes to fn_800740E4 and the two break models; the load hook selects the
 * A10500 files when lbl_8057D6A0.field_0xA8.field_0x4 is 17 and the A10503 files otherwise (member
 * names inferred, the file members' from the file names). */
struct DefenseSystemBreakModels {
    /* 0x0 */ int missionId;
    /* 0x4 */ const char *brkFile;    /* shown on entering the damaged state */
    /* 0x8 */ const char *aftBrkFile; /* shown on destruction */
};
CHECK_SIZEOF(DefenseSystemBreakModels, 0xC);

static const DefenseSystemBreakModels s_breakModelsA10500 = {0, "A10500SHIELDCONT_BRK.DFF",
                                                             "A10500SHIELDCONTAFT_BRK.DFF"};
static const DefenseSystemBreakModels s_breakModelsA10503 = {0, "A10503SHIELDCONT_BRK.DFF",
                                                             "A10503SHIELDCONTAFT_BRK.DFF"};

/* The setting in use, chosen when the gadget type loads (name inferred). */
static const DefenseSystemBreakModels *s_breakModels = &s_breakModelsA10500;

/* The number of Targets around the gadget (name inferred). */
static const u32 kObjectCount = 4;

const char *DefenseSystem::s_name = "DefenseSystem";

/* Values that the original sets in its static initialiser (names inferred from their uses): the
 * distance of the Targets from the gadget, the height of the second object above it and the value
 * passed to its fn_802B4498.
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original initialiser expression
 * and gives the start-up stores; the const qualifier is what lets UpdateModelMatrix load the
 * distance once before its loop, as the original does (with a non-const object the loop reloads it
 * after every call: 0x180 bytes instead of 0x190). The constructor's two values give the same code
 * with or without const; they share the distance's form. Revisit when the original initialiser is
 * found. */
static const float s_objectDistance = UnkStartupFloat_800E2570(15.0f).value;
static const float s_object2Height = UnkStartupFloat_800E2570(60.0f).value;
static const float s_object2Value = UnkStartupFloat_800E2570(800.0f).value;

DefenseSystem::DefenseSystem(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_setState(0), m_attackFlags(0), m_comId(0), m_angle(0.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x5F, 4);
    ReadOwner(owner);

    m_object = fn_801E7128(m_charInfo, 0xA400);
    m_objects.reserve(kObjectCount);
    for (u32 i = 0; i < kObjectCount; i++) {
        m_objects.push_back(fn_801E7128(m_charInfo, 0x8800));
    }

    m_object2 = fn_802B4254(0, 0);
    fn_802B4498(m_object2.get(), s_object2Value);
    fn_802B4510(m_object2.get(), 1);

    Fn8040Vector3 above(0.0f, s_object2Height, 0.0f);

    const Fn8040Vector3 &object2Position = above + m_charInfo->m_position;
    fn_802B44C8(m_object2.get(), &object2Position);

    m_model = fn_8015F800(1, "DEFENSESYSTEM.BIN");

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new DefenseSystemSetState);
    }
    m_setState = dynamic_cast<DefenseSystemSetState *>(owner->m_adapterState);

    m_sound = fn_801D3E58(0x7730, &m_charInfo->m_position, this);
    ChangeState(DefenseSystemSetState::STATE_NONE, m_setState->m_state);
}

DefenseSystem::~DefenseSystem() {
    m_sound.fn_801D455C();
}

extern "C" void fn_8022440C(void) {
    if (lbl_8057D6A0.field_0xA8.field_0x4 == 17) {
        s_breakModels = &s_breakModelsA10500;
    } else {
        s_breakModels = &s_breakModelsA10503;
    }
    fn_8006B474("DEFENSESYSTEM.BIN", 1);
}

extern "C" void fn_80224408(void) {}

void DefenseSystem::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DefenseSystem(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void DefenseSystem::UpdateBody(float dt) {
    if (m_attackFlags.Test(1)) {
        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x44 =
            static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakModels->aftBrkFile));
        effect.field_0x34 = 1.0f;
        effect.field_0x4A = 0;
        effect.field_0x54 = 1;
        fn_8015D93C(&effect);

        fn_8016907C(fn_800CB69C(), m_comId, 1);

        {
            boost::shared_ptr<Mission::Mission> mission =
                fn_800740E4(fn_8006D6A8(), s_breakModels->missionId).lock();

            if (mission.get() != 0) {
                mission->fn_8016B224(1, 0);
            }
        }

        fn_802B4510(m_object2.get(), 0);
        fn_801D54F4(fn_80053164(), 0x7732, &m_charInfo->m_position, 0, 1);
        fn_800C9B04(m_owner);
        ChangeState(m_setState->m_state, DefenseSystemSetState::STATE_DESTROYED);
        m_attackFlags.Reset(1);
        m_attackFlags.Reset(0);
        m_flags |= 1;
        return;
    }

    if (m_attackFlags.Test(0)) {
        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x44 =
            static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakModels->brkFile));
        effect.field_0x34 = 1.0f;
        effect.field_0x4A = 0;
        effect.field_0x54 = 1;
        fn_8015D93C(&effect);

        fn_801D54F4(fn_80053164(), 0x7732, &m_charInfo->m_position, 0, 1);
        ChangeState(m_setState->m_state, DefenseSystemSetState::STATE_DAMAGED);
        m_attackFlags.Reset(0);
    }

    m_angle += dt * DegreesToRadians(12.0f);
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void DefenseSystem::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);

    if (m_setState->m_state == DefenseSystemSetState::STATE_DESTROYED) {
        m_setState->Reset();
        ChangeState(DefenseSystemSetState::STATE_DESTROYED, m_setState->m_state);
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void DefenseSystem::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
}

void DefenseSystem::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    fn_8040B80C(&matrix, m_angle);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);

    /* The Targets stand around the gadget at even angles. */
    std::vector<boost::shared_ptr<Target> >::iterator it = m_objects.begin();
    const float step = DegreesToRadians(360.0f) / kObjectCount;

    for (u32 i = 0; it != m_objects.end(); ++i, ++it) {
        Fn8040Vector3 offset(0.0f, s_objectDistance, 0.0f);

        offset = fn_8040D044(offset, step * i);
        offset = matrix * offset;
        fn_800CCE38(it->get(), &offset);
    }
}

void DefenseSystem::ChangeState(int previous, int next) {
    switch (previous) {
    case DefenseSystemSetState::STATE_INTACT:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_801604FC();
        m_model->fn_80162804();
        m_model->fn_80161880();
        break;

    case DefenseSystemSetState::STATE_DAMAGED:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80162804();
        m_model->fn_80161880();
        break;
    }

    switch (next) {
    case DefenseSystemSetState::STATE_INTACT: {
        m_model->fn_80162A68(0, 0);
        m_model->fn_80160C28(0);
        m_model->fn_8016082C(0);
        m_model->fn_8015FF60();
        m_model->fn_80160B18(0);
        UpdateModelMatrix();

        m_target = fn_80166AF4();
        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167180(-1);
        }

        m_model->fn_80161FC8(0, m_charInfo);

        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(0).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&DefenseSystem::OnAttack, this, _1)));
        }
        break;
    }

    case DefenseSystemSetState::STATE_DAMAGED: {
        m_model->fn_80162A68(1, 0);
        UpdateModelMatrix();

        m_target = fn_80166AF4();
        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167180(-1);
        }

        m_model->fn_80161FC8(1, m_charInfo);

        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(1).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&DefenseSystem::OnAttack, this, _1)));
        }
        break;
    }
    }

    m_setState->m_state = next;
}

void DefenseSystem::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_setState->m_state == DefenseSystemSetState::STATE_DESTROYED) {
        return;
    }

    m_setState->m_health -= param.field_0x50;

    if (m_setState->m_health < 0.0f || fn_8000A950(m_setState->m_health)) {
        param.field_0x54 = 3;

        boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

        if (attacker.get() != 0) {
            fn_8020642C(attacker, 0x47);
        }
        m_attackFlags.Set(1);
    } else if (m_setState->m_state == DefenseSystemSetState::STATE_INTACT &&
               !m_attackFlags.Test(1)) {
        if (m_setState->m_health < 10.0f || fn_801C455C(m_setState->m_health, 10.0f)) {
            m_attackFlags.Set(0);
        }
    }

    if (param.field_0x50 >= 1.0f) {
        fn_801D54F4(fn_80053164(), 0x7731, &m_charInfo->m_position, 0, 1);
    }
}

GadgetTypeInfo lbl_80546334 = {
    "DEFENSE_SYSTEM", fn_8022440C, fn_80224408, DefenseSystem::Create, 0, 0, 0x1388, 1, 0, 10, 0,
};
