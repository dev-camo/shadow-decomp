/* The FireRing gadget (class described in Game/FireRing.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the attack object and to the probes are boost::function objects built
 * with boost::bind(&FireRing::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/FireRing.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Player.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80136B5C.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace FireRing {

const char *FireRing::s_name = "FireRing";

/* What the probe callbacks pass to fn_8015AE44 (placeholder name): fn_80136884 stores the
 * character it is called with and sets m_found. */
struct UnkHitContext_80135444 {
    UnkHitContext_80135444() : m_found(false) {}

    /* 0x0 */ boost::weak_ptr<CharInfo> m_hit;
    /* 0x8 */ bool m_found;
};
CHECK_SIZEOF(UnkHitContext_80135444, 0xC);

/* The callback that fn_8015AE44 calls for each character info in the hits (placeholder name). */
static void fn_80136884(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80135444 *context = static_cast<UnkHitContext_80135444 *>(arg);

    if (context != 0) {
        context->m_hit = hit;
        context->m_found = true;
    }
}

FireRing::FireRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x1A, 4, "FireRing");
    ReadOwner(m_owner);

    m_model = fn_8015F800(1, "FIRERING.BIN");
    m_model->fn_801629D8("ring", 0);
    m_model->fn_80161F08("ring", boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161334("ring");

    if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&FireRing::OnAttack, this, _1));
    }

    {
        Fn8040Vector3 offset(0.0f, 0.0f, 14.0f);
        UnkRotation3 rotation = lbl_805E07B8;
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 3.0f, 18.0f, 0x80000,
                        10, 0, &offset, &rotation, 1);

        m_handleA = fn_8015B8C4(probe);
        m_handleA->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&FireRing::OnHitA, this, _1)));
    }

    {
        Fn8040Vector3 offset(0.0f, 0.0f, -14.0f);
        UnkRotation3 rotation = lbl_805E07B8;
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 3.0f, 18.0f, 0x80000,
                        10, 0, &offset, &rotation, 1);

        m_handleB = fn_8015B8C4(probe);
        m_handleB->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&FireRing::OnHitB, this, _1)));
    }

    UpdateModelMatrix();
}

FireRing::~FireRing() {}

void FireRing::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    fn_8015BE9C(m_handleA.get(), dt);
    fn_8015BE9C(m_handleB.get(), dt);
    fn_801D53C4(fn_80053164(), 0x7444, &m_charInfo->m_position, this, 3, 0, 2);

    switch (m_state) {
    case 0:
        m_state = 1;

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
    case 1:
        if (m_passFlags.Test(2)) {
            if (!m_character.expired()) {
                if (fn_800B7AD0(m_character.lock())) {
                    m_character.reset();
                    m_passFlags.Clear();
                }
            } else {
                m_passFlags.Clear();
            }
        }
        break;
    case 2:
        if (!m_character.expired()) {
            boost::shared_ptr<CharInfo> character = m_character.lock();

            fn_8015F6EC(character, 0, 5, 0);
            fn_80136B5C(1);
            m_character.reset();
        }

        m_timer = 0.5f;
        m_state = 3;
        m_handleA->SetFlags(2);
        m_handleB->SetFlags(2);
        m_model->fn_8016268C(-1.0f);

        if (m_target.get() != 0) {
            m_target.reset();
        }
        break;
    case 3:
        m_timer -= dt;

        if (m_timer <= 0.0f) {
            fn_8015A794(this);
            m_flags |= 1;
        }
        break;
    }
}

void FireRing::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_8016108C();
        fn_8015BE64(m_handleA.get());
        fn_8015BE64(m_handleB.get());
    }
}

void FireRing::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
    m_timer = 0.0f;
    m_passFlags.Clear();
}

void FireRing::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void FireRing::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x10) == 0) {
                param.field_0x54 = 5;
                return;
            }
        }

        param.field_0x54 = 3;
        m_state = 3;
    } else {
        param.field_0x54 = 5;
    }
}

void FireRing::OnHitA(ColliDetectArgs &args) {
    UnkHitContext_80135444 context;

    fn_8015AE44(&args, fn_80136884, &context);

    if (context.m_found) {
        if (m_passFlags.Test(2)) {
            if (m_passFlags.Test(1)) {
                m_state = 2;
                m_passFlags.Clear();
            }
        } else {
            m_passFlags.Set(0);
            m_passFlags.Set(2);
            m_character = context.m_hit;
        }
    }
}

void FireRing::OnHitB(ColliDetectArgs &args) {
    UnkHitContext_80135444 context;

    fn_8015AE44(&args, fn_80136884, &context);

    if (context.m_found) {
        if (m_passFlags.Test(2)) {
            if (m_passFlags.Test(0)) {
                m_state = 2;
                m_passFlags = UnkBitFlags_80014E04(0);
            }
        } else {
            m_passFlags.Set(1);
            m_passFlags.Set(2);
            m_character = context.m_hit;
        }
    }
}

} // namespace FireRing

extern "C" void fn_801352FC(void) {
    fn_8006B474("FIRERING.BIN", 1);
}

extern "C" void fn_801352F8(void) {}

void FireRing::FireRing::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new FireRing(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80528B74 = {
    "FIRE_RING", fn_801352FC, fn_801352F8, FireRing::FireRing::Create, 0, 0, 0xC85, 1, 0, 10, 0,
};
