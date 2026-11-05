/* The MagmaGuard gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model attack object's callback is a boost::function object built with
 * boost::bind(&SetMagmaGuard::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/MagmaGuard.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The height of the second object above the character info's position (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_object2Height(94.0f);

/* The health of an undamaged gadget (name inferred). */
static const float s_fullHealth = 12.0f;

namespace MagmaGuard {

const char *SetMagmaGuard::s_name = "SetMagmaGuard";

SetMagmaGuard::SetMagmaGuard(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x7D, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, "MAGMAGUARD.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(0, boost::weak_ptr<CharInfo>(m_charInfo), 0);

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetMagmaGuard::OnAttack, this, _1));
    }

    m_object2 = fn_802B4254(0, 0);
    fn_802B4510(m_object2.get(), 1);
    UpdateModelMatrix();
}

SetMagmaGuard::~SetMagmaGuard() {}

void SetMagmaGuard::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    m_recoveryTime -= dt;
    if (m_recoveryTime < 0.0f) {
        m_recoveryTime = 0.0f;
    }

    switch (m_state) {
    case 0: {
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        u32 ownerFlags;

        fn_800C9EC4(m_owner, &ownerFlags);
        if (ownerFlags & 0x400000) {
            /* The owner keeps the mark that case 2 sets: start destroyed (inferred). */
            fn_801690D0(fn_800CB69C(), m_comId, 1);
            m_model->fn_80160A84(0, 1);
            if (m_target.get() != 0) {
                m_target->fn_80167010(1);
                m_target->fn_80167010(2);
                m_target->fn_80167010(3);
            }
            m_state = 3;
            m_flags.Set(3);
        } else {
            m_state = 1;
        }
        break;
    }
    case 1:
        break;
    case 2:
        fn_801690D0(fn_800CB69C(), m_comId, 1);

        /* The inner scope releases the actor before the calls below. */
        {
            boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

            if (actor.get() != 0) {
                actor->UnkSlot7(1, 0);
            }
        }

        m_model->fn_80160A84(0, 1);
        fn_800C9EB0(m_owner, 0x400000);
        m_state = 3;
        fn_800C985C(m_owner);
        break;
    case 3:
        break;
    }

    UpdateModelMatrix();

    if (m_flags.Test(3)) {
        fn_801D53C4(fn_80053164(), 0x784C, &m_charInfo->m_position, this, 3, 0, 2);
    }
}

void SetMagmaGuard::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
}

void SetMagmaGuard::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_health = s_fullHealth;
    m_recoveryTime = 0.0f;
    m_flags.Clear();
}

void SetMagmaGuard::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
    if (m_object2.get() != 0) {
        Fn8040Vector3 object2Position = m_charInfo->m_position;
        object2Position.y += s_object2Height.value;
        fn_802B44C8(m_object2.get(), &object2Position);
    }
}

void SetMagmaGuard::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_recoveryTime > 0.0f) {
        return;
    }
    if (m_state != 1) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3D1) == 0) {
            return;
        }

        param.field_0x54 = 1;
        m_health -= param.field_0x50;
        if (m_health <= 0.0f) {
            m_state = 2;
            m_flags.Set(3);
            fn_80169AEC(7, 2.0f, &m_charInfo->m_position);
        } else {
            m_model->fn_80160A84(0, 2);
            m_recoveryTime = 0.5f;
        }
        fn_801D54F4(fn_80053164(), 0x784A, &m_charInfo->m_position, 0, 1);

        /* The products are computed in single precision: the first is stored as 0x412CCCCC, one
         * unit below the nearest float to 10.8. */
        if (!m_flags.Test(0) && m_health <= s_fullHealth * 0.9f) {
            m_target->fn_80167010(1);
            m_flags.Set(0);
        }
        if (!m_flags.Test(1) && m_health <= s_fullHealth * 0.5f) {
            m_target->fn_80166E20(1);
            m_target->fn_80167010(2);
            m_flags.Set(1);
        }
        if (!m_flags.Test(2) && m_health <= 0.0f) {
            m_target->fn_80166E20(2);
            m_target->fn_80167010(3);
            m_flags.Set(2);
        }
    }
}

} // namespace MagmaGuard

extern "C" void fn_8022AD78(void) {
    fn_8006B474("MAGMAGUARD.BIN", 1);
}

extern "C" void fn_8022AD74(void) {}

void MagmaGuard::SetMagmaGuard::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetMagmaGuard(parent, owner);
}

GadgetTypeInfo lbl_80546A9C = {
    "MAGMA GUARD",
    fn_8022AD78,
    fn_8022AD74,
    MagmaGuard::SetMagmaGuard::Create,
    0,
    0,
    0x1838,
    1,
    0,
    10,
    0,
};
