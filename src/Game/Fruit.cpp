/* The Fruit gadget (class described in Game/Fruit.h). File name inferred from the descriptor and
 * the namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The callbacks it hands to the attack objects are boost::function objects built with
 * boost::bind(&FruitTask::method, this, _1); their manager, invoker and copy functions come from
 * the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Fruit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

namespace Fruit {

FruitTask::FruitTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_charInfo = fn_8015CB88(0x75, 4, "Fruit");
    m_model = fn_8015F9D8(0, 0xD3);
    m_model->fn_80162A68(0, 0);

    if (m_model->fn_80161FC8(0, m_charInfo)) {
        boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(0);

        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->field_0x54 = 1;
            attack->field_0x38 = 1;
            attack->field_0x34 = 1.0f;
            attack->field_0x10.field_0x0 = AttackCallback(boost::bind(&FruitTask::OnHit, this, _1));
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&FruitTask::OnAttack, this, _1));
        }
    }

    ReadOwner(owner);
    UpdateModelMatrix(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

FruitTask::~FruitTask() {}

void FruitTask::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_RESET:
        m_state = STATE_WAITING;

        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        if (m_target.get() != 0) {
            m_target->fn_80167180(-1);
        }
        break;
    case STATE_HIT:
        StartBreakEffect();

        if (m_target.get() != 0) {
            m_target.reset();
        }

        if (SetupPart2000()) {
            m_state = STATE_PART_2000;
        } else {
            if (m_owner != 0) {
                fn_800C9B34(m_owner);
            }
            m_flags |= 1;
        }
        break;
    case STATE_PART_2000:
        /* The STATE_HIT case sets this state once part 2000 is set up, and the gadget has nothing
         * more to do in it than the model update above.
         * Matching workaround: the state is listed as an empty case. With it the switch has the
         * original's shape, a compare with 2 first, then with 0, and an unreachable branch to the
         * end after the default branch (USA (GUPE8P) 0x80105B6C-0x80105B84); without it the
         * switch compares with 1 first and UpdateBody is 77 words instead of 75. STATE_WAITING
         * needs nothing here either and has no case; adding one gives the same instructions.
         * Revisit if the original's handling of these states is found. */
        break;
    }
}

void FruitTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix(&m_charInfo->m_position, &m_charInfo->m_rotation);

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
    }
}

void FruitTask::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
    }

    m_state = STATE_RESET;
}

void FruitTask::UpdateModelMatrix(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation) {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, position);
        fn_8040B848(&matrix, rotation->y);
        fn_8040B884(&matrix, rotation->x);
        fn_8040B80C(&matrix, rotation->z);
        m_model->fn_801633DC(&matrix);
    }
}

void FruitTask::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 2) != 0) {
                param.field_0x54 = 5;
                return;
            }

            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                fn_8020642C(attacker, 0x16);
            }

            param.field_0x54 = 3;
            m_state = STATE_HIT;
            return;
        }
    }

    param.field_0x54 = 5;
}

void FruitTask::OnHit(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        m_state = STATE_HIT;
    }
}

void FruitTask::StartBreakEffect() {
    if (m_model.get() != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1000);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.3f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 0, entry->name));
            parameters.field_0x54 = 1;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
            fn_801D54F4(fn_80053164(), 0x1022, &m_charInfo->m_position, 0, 1);
        }
    }
}

bool FruitTask::SetupPart2000() {
    if (m_model->fn_80162EA8(2000)) {
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model->fn_80162A68(2000, 0);

        if (m_model->fn_80161FC8(2000, m_charInfo)) {
            boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(2000);

            if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
                attack->field_0x54 = 0;
                attack->field_0x55 = 1;
                attack->field_0x1C.field_0x0 =
                    AttackCallback(boost::bind(&FruitTask::OnAttack, this, _1));
            }
        }

        UpdateModelMatrix(&m_charInfo->m_position, &m_charInfo->m_rotation);
        return true;
    }

    return false;
}

} // namespace Fruit

extern "C" void fn_80105368(void) {
    fn_8006B50C(0xD3, 1);
}

extern "C" void fn_80105364(void) {}

void Fruit::FruitTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new FruitTask(parent, owner);
}

GadgetTypeInfo lbl_805260A4 = {
    "Fruit", fn_80105368, fn_80105364, Fruit::FruitTask::Create, 0, 0, 0x22, 1, 0, 10, 0,
};
