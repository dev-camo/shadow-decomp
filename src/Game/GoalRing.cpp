/* The GoalRing gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&GoalRing::OnCollision, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/GoalRing.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

/* The colour effect id of each colour, -1 for none (name inferred). */
static const s32 s_colorEffectIds[] = {0x2BB, 0x2B5, 0x2BE, 0x2C1, 0x2B8, 0x2B2, 0x2AF, -1};

/* The callback that fn_8015AA84 calls for each character info in the collision results
 * (placeholder name). context is the address of OnCollision's flag: sets it and passes the
 * manager of fn_8006D6A8 to fn_8016DA34 (behaviour inferred). */
static void fn_80106F48(boost::shared_ptr<CharInfo> hit, void *context) {
    bool *touched = static_cast<bool *>(context);

    if (touched != 0) {
        *touched = true;
        fn_8016DA34(fn_8006D6A8());
    }
}

namespace GoalRing {

const char *GoalRing::s_name = "GoalRing";

GoalRing::GoalRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_color(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(7, 4);
    CopyFromOwner(m_owner);
    m_model = fn_8015F800(0, "GOALRING.BIN");
    m_shownColor = m_color;
    m_model->fn_80162A68(m_shownColor, 0);
    m_model->fn_80161F08("ring", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&GoalRing::OnCollision, this, _1));
    m_model->fn_80160BA0("ring");
    m_model->fn_801607A8("ring");
    m_model->fn_801609F8("ring");
    m_model->fn_8015FF60();
    m_effect = fn_8015D794(&m_charInfo->m_position, 0x8E);

    if (s_colorEffectIds[m_shownColor] != -1) {
        m_colorEffect = fn_801D2B38(0, &m_charInfo->m_position, &m_charInfo->m_rotation,
                                    s_colorEffectIds[m_shownColor], 0);
    }

    m_sound = fn_801D3DC8(0xE00D, &m_charInfo->m_position, this);
    SetColorEffect(m_shownColor);
    UpdatePositions();
}

GoalRing::~GoalRing() {
    m_sound.fn_801D455C();

    if (!m_colorEffect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_colorEffect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_colorEffect.reset();
    }

    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }
}

void GoalRing::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    UpdatePositions();
}

void GoalRing::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdatePositions();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        if (m_color != m_shownColor) {
            m_shownColor = m_color;
            SetColorEffect(m_shownColor);
        }
    }
}

void GoalRing::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_color, sizeof(m_color));
}

void GoalRing::UpdatePositions() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        m_model->fn_801633DC(&matrix);
    }

    if (!m_colorEffect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_colorEffect.lock());

        if (effect.get() != 0) {
            fn_80425E4C(effect.get(), &m_charInfo->m_position);
        }
    }

    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            fn_80425E4C(effect.get(), &m_charInfo->m_position);
        }
    }

    if (m_sound.field_0x0 != 0) {
        m_sound.fn_800CCE14(&m_charInfo->m_position);
    }
}

void GoalRing::OnCollision(ColliDetectArgs &args) {
    bool touched = false;

    fn_8015AA84(&args, fn_80106F48, &touched);

    if (touched) {
        if (m_model.get() != 0) {
            m_model->fn_801612E0();
        }

        m_sound.fn_801D455C();

        if (m_color != 7) {
            lbl_8057D6A0.field_0xC1 = 1;
        }
    }
}

void GoalRing::SetColorEffect(int color) {
    if (m_model.get() != 0) {
        if (!m_colorEffect.expired()) {
            boost::shared_ptr<Effect::System::EffectElement> effect(m_colorEffect.lock());

            if (effect.get() != 0) {
                effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            }
        }

        if (s_colorEffectIds[m_shownColor] != -1) {
            m_colorEffect = fn_801D2B38(0, &m_charInfo->m_position, &m_charInfo->m_rotation,
                                        s_colorEffectIds[color], 0);
        }
    }
}

} // namespace GoalRing

extern "C" void fn_80106324(void) {
    fn_8006B474("GOALRING.BIN", 1);
}

extern "C" void fn_80106320(void) {}

void GoalRing::GoalRing::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GoalRing(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "color", 0, 0, 7, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805261AC = {
    "GOALRING", fn_80106324, fn_80106320, GoalRing::GoalRing::Create, 0, 0, 0x14, 1,
    0,          10,          s_paramInfo,
};
