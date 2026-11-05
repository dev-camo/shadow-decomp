/* The DamageBlock gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/DamageBlock.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

const char *DamageBlock::s_name = "DamageBlock";

DamageBlock::DamageBlock(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_kind, sizeof(m_kind));
    m_charInfo = fn_8015CD90(0x21, 4);
    m_modelIndex = m_kind;

    m_model = new GadgetModelInfo(1, "DAMAGEBLOCK.BIN");
    m_model->fn_80162A68(m_modelIndex, 0);
    m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

    ReadOwner(m_owner);
    UpdateModelMatrix();
}

DamageBlock::~DamageBlock() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void DamageBlock::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void DamageBlock::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);

    if (m_model != 0) {
        if (m_kind != m_modelIndex) {
            int count = fn_8015FF44(m_model);

            m_modelIndex = m_kind;
            if (m_modelIndex < 0) {
                m_modelIndex = 0;
            }
            if (m_modelIndex > count - 1) {
                m_modelIndex = count - 1;
            }

            m_model->fn_80161880();
            m_model->fn_80162804();
            m_model->fn_80162A68(m_modelIndex, 0);
            m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

            m_kind = m_modelIndex;
            fn_800C9D68(owner, &m_kind, sizeof(m_kind));
        }

        UpdateModelMatrix();
        m_model->fn_8016108C();
    }
}

void DamageBlock::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_kind, sizeof(m_kind));
}

void DamageBlock::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_801430A4(void) {
    fn_8006B474("DAMAGEBLOCK.BIN", 1);
}

extern "C" void fn_801430A0(void) {}

void DamageBlock::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new DamageBlock(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "kind", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805297F8 = {
    "DAMAGE BLOCK", fn_801430A4, fn_801430A0, DamageBlock::Create, 0, 0, 0x1134, 1, 0, 10,
    s_paramInfo,
};
