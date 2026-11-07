/* The SuperComputer gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model attack object's callback is a boost::function object built with
 * boost::bind(&SetSuperComputer::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/SuperComputer.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

static const char *s_modelFile = "SUPERCOMPUTER.BIN";

/* The heights of the second object and of the Target above the character info's position (names
 * inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original sets them with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_object2Height(100.0f);
static UnkStartupFloat_800E2570 s_objectHeight(50.0f);

namespace SuperComputer {

const char *SetSuperComputer::s_name = "SetSuperComputer";

SetSuperComputer::SetSuperComputer(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x79, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, s_modelFile));
    m_crashInfo = GadgetBuilder::fn_80208978(m_resource, 0);
    m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);
    m_model->fn_80161F08(m_crashInfo->fn_802087BC(), boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(m_crashInfo->fn_802087BC());
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetSuperComputer::OnAttack, this, _1));
    }

    m_object2 = fn_802B4254(0, 0);
    fn_802B4510(m_object2.get(), 1);
    m_object = fn_801E72DC(0x800);
    UpdateModelMatrix();
}

SetSuperComputer::~SetSuperComputer() {
    fn_801E7108(m_object);
}

void SetSuperComputer::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    m_recoveryTime -= dt;
    if (m_recoveryTime <= 0.0f) {
        m_recoveryTime = 0.0f;
    }

    if (m_flags.Test(1)) {
        fn_801D53C4(fn_80053164(), 0x7845, &m_charInfo->m_position, this, 3, 0, 2);
    }
    if (!m_flags.Test(0)) {
        fn_801D53C4(fn_80053164(), 0x785F, &m_charInfo->m_position, this, 3, 0, 2);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 1:
        if (m_health < 37.5f) {
            EnterDamageStage(0);
            m_state = 2;
        }
        break;
    case 2:
        if (m_health < 25.0f) {
            EnterDamageStage(1);
            m_state = 3;
        }
        break;
    case 3:
        if (m_health < 12.5f) {
            EnterDamageStage(2);
            m_state = 4;
        }
        break;
    case 4:
        if (m_health <= 0.0f) {
            EnterDamageStage(3);
            m_state = 5;
        }
        break;
    case 5: {
        /* The inner scope releases the mission before the calls below. */
        {
            boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 0).lock();

            if (mission.get() != 0) {
                mission->fn_8016B224(1, 0);
            }
        }

        s32 id;

        fn_800C9E50(m_owner, &id);
        fn_8016907C(fn_800CB69C(), id, 1);
        fn_8015A768(this);
        m_state = 6;
        m_flags.Reset(1);
        m_flags.Set(0);
        fn_801E7108(m_object);
        break;
    }
    case 6:
        break;
    }

    UpdateModelMatrix();
}

void SetSuperComputer::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
}

void SetSuperComputer::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
    m_health = 50.0f;
    m_recoveryTime = 0.0f;
}

void SetSuperComputer::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
    if (m_object2.get() != 0) {
        Fn8040Vector3 object2Position = m_charInfo->m_position;
        object2Position.y += s_object2Height.value;
        fn_802B44C8(m_object2.get(), &object2Position);
    }
    if (m_object.get() != 0) {
        Fn8040Vector3 objectPosition = m_charInfo->m_position;
        objectPosition.y += s_objectHeight.value;
        fn_800CCE38(m_object.get(), &objectPosition);
    }
}

/* The if/return form: returning the condition directly compiles to a different compare
 * sequence. */
inline bool SetSuperComputer::IsDamageable() const {
    if (m_state == 1 || m_state == 2 || m_state == 3 || m_state == 4) {
        return true;
    }
    return false;
}

void SetSuperComputer::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_recoveryTime > 0.0f) {
        return;
    }
    if (!IsDamageable()) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3D0) == 0) {
            return;
        }

        m_health -= param.field_0x50;
        m_recoveryTime = 0.2f;
        if (m_health <= 0.0f) {
            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                fn_8020642C(attacker, 0);
            }
            param.field_0x54 = 3;
        } else {
            fn_801D54F4(fn_80053164(), 0x7841, &m_charInfo->m_position, 0, 1);
            param.field_0x54 = 1;
        }
    }
}

void SetSuperComputer::EnterDamageStage(int stage) {
    /* The original has its own copy of the part-target setup in stages 0 and 3 and of the sound
     * call in every stage; an inline helper for the setup is emitted out of line instead. */
    switch (stage) {
    case 0:
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167180(25);
        }
        fn_801D54F4(fn_80053164(), 0x7843, &m_charInfo->m_position, 0, 1);
        break;
    case 1:
        if (m_target.get() != 0) {
            m_target->fn_80166E20(25);
            m_target->fn_80167180(50);
        }
        fn_801D54F4(fn_80053164(), 0x7843, &m_charInfo->m_position, 0, 1);
        break;
    case 2:
        if (m_target.get() != 0) {
            m_target->fn_80166E20(50);
            m_target->fn_80167180(75);
        }
        fn_801D54F4(fn_80053164(), 0x7843, &m_charInfo->m_position, 0, 1);
        break;
    case 3:
        StartCrashEffect();
        m_target.reset();
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model->fn_801629D8(m_crashInfo->fn_802087A0(), 0);
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167180(-1);
        }
        UpdateModelMatrix();
        fn_801D54F4(fn_80053164(), 0x7843, &m_charInfo->m_position, 0, 1);
        break;
    }

    m_flags.Set(1);
}

void SetSuperComputer::StartCrashEffect() {
    if (m_crashInfo.get() != 0) {
        const UnkCrashEntry_802086C0 *entry = m_crashInfo->fn_802086C0();

        if (entry != 0) {
            RpClump *clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->fileName));

            if (clump != 0) {
                UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

                parameters.field_0x4.x += entry->field_0x18.x;
                parameters.field_0x4.y += entry->field_0x18.y;
                parameters.field_0x4.z += entry->field_0x18.z;
                parameters.field_0x34 = 1.3f;
                parameters.field_0x44 = clump;
                parameters.field_0x54 = 1;
                parameters.field_0x4C = 150;
                fn_8015D93C(&parameters);
            }
        }
    }
}

} // namespace SuperComputer

/* Uses the literal: the original takes the string's address here instead of loading s_modelFile. */
extern "C" void fn_8022D9F4(void) {
    fn_8006B474("SUPERCOMPUTER.BIN", 1);
}

extern "C" void fn_8022D9F0(void) {}

void SuperComputer::SetSuperComputer::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetSuperComputer(parent, owner);
}

GadgetTypeInfo lbl_80546EA4 = {
    "SUPER COMPUTER",
    fn_8022D9F4,
    fn_8022D9F0,
    SuperComputer::SetSuperComputer::Create,
    0,
    0,
    0x1771,
    1,
    0,
    10,
    0,
};
