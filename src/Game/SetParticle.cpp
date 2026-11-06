/* The SetParticle gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/SetParticle.h"
#include "Game/GadgetTypeInfo.h"

using Effect::System::EffectElement;

const char *SetParticle::s_name = "SetParticle";

SetParticle::SetParticle(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CE20(0x39);
    /* The original passes the stored owner (m_owner), not the parameter. */
    CopyFromOwner(m_owner);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

    m_effect = fn_80168C48(&m_charInfo->m_position, &direction, m_params.number);

    if (!m_effect.expired()) {
        /* Here and below a named local tested with get(), not an if-declaration: that form
         * converts to bool before lock()'s temporary is destroyed and keeps the result in a
         * saved register across that call, which the original does not do. */
        boost::shared_ptr<EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->SetMatrixSource(&m_matrix);
        }
    }
}

SetParticle::~SetParticle() {
    if (!m_effect.expired()) {
        boost::shared_ptr<EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }
}

void SetParticle::UpdateBody(float dt) {}

void SetParticle::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 number = m_params.number;

    CopyFromOwner(owner);

    if (number != m_params.number) {
        if (!m_effect.expired()) {
            boost::shared_ptr<EffectElement> effect(m_effect.lock());

            if (effect.get() != 0) {
                effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            }

            m_effect.reset();
        }

        Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

        m_effect = fn_80168C48(&m_charInfo->m_position, &direction, m_params.number);

        if (!m_effect.expired()) {
            boost::shared_ptr<EffectElement> effect(m_effect.lock());

            if (effect.get() != 0) {
                effect->SetMatrixSource(&m_matrix);
            }
        }
    }
}

void SetParticle::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    fn_8040B6F8(&m_matrix);
    fn_8040B59C(&m_matrix, &m_charInfo->m_position);
    fn_8040B848(&m_matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&m_matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&m_matrix, m_charInfo->m_rotation.z);
    fn_8040B720(&m_matrix, m_params.scaleX, m_params.scaleY, m_params.scaleZ);
}

extern "C" void fn_8011DC94(void) {}

extern "C" void fn_8011DC90(void) {}

void SetParticle::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetParticle(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "number", 0, 0xFFFF0000, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "scale X", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805275B8 = {
    "PARTICLE", fn_8011DC94, fn_8011DC90, SetParticle::Create, 0, 0, 0x258A, 1, 0, 10, s_paramInfo,
};
