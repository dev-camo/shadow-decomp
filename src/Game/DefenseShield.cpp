/* DefenseShield (class name from RTTI): the DEFENSE_SHIELD gadget, a shield (model file
 * DEFENSESHIELD.BIN) whose health attacks use up; the health is kept in a DefenseShieldSetState
 * (class name from RTTI) that the owner remembers (behaviour inferred). File name inferred from
 * the class. Method and member names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/DefenseShield.h"

#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *DefenseShield::s_name = "DefenseShield";

DefenseShield::DefenseShield(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_setState(0), m_state(0), m_flags(0), m_comId(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x60, 4);
    ReadOwner(owner);
    m_object = fn_801E72DC(0xA800);
    m_model = fn_8015F800(1, "DEFENSESHIELD.BIN");

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new DefenseShieldSetState);
    }

    m_setState = dynamic_cast<DefenseShieldSetState *>(owner->m_adapterState);

    if (fn_8016904C(fn_800CB69C(), m_comId) == 1 || m_setState->m_health < 0.0f ||
        fn_8000A950(m_setState->m_health)) {
        ChangeState(m_state, 3);
        m_object->ClearFlags(1);
    } else if (m_setState->m_health < 5.0f || fn_801C455C(m_setState->m_health, 5.0f)) {
        ChangeState(m_state, 2);
    } else {
        ChangeState(m_state, 1);
    }
}

DefenseShield::~DefenseShield() {}

extern "C" void fn_80223000(void) {
    fn_8006B474("DEFENSESHIELD.BIN", 1);
}

extern "C" void fn_80222FFC(void) {}

void DefenseShield::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DefenseShield(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void DefenseShield::UpdateBody(float dt) {
    if (m_state != 3 && fn_8016904C(fn_800CB69C(), m_comId) == 1) {
        m_flags.Set(1);
    }

    if (m_flags.Test(1)) {
        fn_801D54F4(fn_80053164(), 0x7734, &m_charInfo->m_position, 0, 1);
        ChangeState(m_state, 3);
        m_flags.Reset(1);
        m_flags.Reset(0);
    } else if (m_flags.Test(0)) {
        ChangeState(m_state, 2);
        m_flags.Reset(0);
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void DefenseShield::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void DefenseShield::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
}

void DefenseShield::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), m_charInfo->m_position);
}

void DefenseShield::ChangeState(int previousState, int state) {
    switch (previousState) {
    case 1:
    case 2:
    case 3:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }

        m_model->fn_80162804();
        m_model->fn_80161880();
        break;
    }

    switch (state) {
    case 1: {
        m_model->fn_80162A68(0, 0);
        UpdateModelMatrix();
        m_target = fn_80166AF4();

        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167010(-1);
        }

        m_model->fn_80161FC8(0, m_charInfo);

        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(0).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&DefenseShield::OnAttack, this, _1)));
        }
        break;
    }

    /* The original repeats state 1's setup here, with its own copy of each call and of the member
     * pointer constant that the binding uses. */
    case 2: {
        m_model->fn_80162A68(1, 0);
        UpdateModelMatrix();
        m_target = fn_80166AF4();

        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167010(-1);
        }

        m_model->fn_80161FC8(0, m_charInfo);

        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(0).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&DefenseShield::OnAttack, this, _1)));
        }
        break;
    }

    case 3:
        m_model->fn_80162A68(2, 0);
        UpdateModelMatrix();
        m_model->fn_80161FC8(1, m_charInfo);
        m_object->ClearFlags(1);
        break;
    }

    m_state = state;
}

void DefenseShield::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_state != 3) {
        m_setState->m_health -= param.field_0x50;

        if (m_setState->m_health < 0.0f || fn_8000A950(m_setState->m_health)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x48);
            }

            m_flags.Set(1);
        } else if (m_state == 1 && !m_flags.Test(1)) {
            if (m_setState->m_health < 5.0f || fn_801C455C(m_setState->m_health, 5.0f)) {
                m_flags.Set(0);
            }

            if (param.field_0x50 >= 1.0f) {
                fn_801D54F4(fn_80053164(), 0x7733, &m_charInfo->m_position, 0, 1);
            }
        }
    }
}

GadgetTypeInfo lbl_8054629C = {
    "DEFENSE_SHIELD", fn_80223000, fn_80222FFC, DefenseShield::Create, 0, 0, 0x1389, 1, 0, 10, 0,
};
