/* The FOOTING gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/Footing.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

const char *Footing::s_name = "Footing";

Footing::Footing(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_modelParam, sizeof(m_modelParam));
    m_charInfo = fn_8015CD90(4, 4);
    m_modelIndex = m_modelParam;

    m_model = new GadgetModelInfo(1, "FOOTING.BIN");
    m_model->fn_80162A68(m_modelIndex, 0);
    m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

    ReadOwner(m_owner);
    UpdateModelMatrix();
}

Footing::~Footing() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void Footing::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void Footing::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model != 0) {
        if (m_modelParam != m_modelIndex) {
            int count = fn_8015FF44(m_model);

            m_modelIndex = m_modelParam;
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

            m_modelParam = m_modelIndex;
            fn_800C9D68(owner, &m_modelParam, sizeof(m_modelParam));
        }

        m_model->fn_8016108C();
    }
}

void Footing::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_modelParam, sizeof(m_modelParam));
}

void Footing::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_80149360(void) {
    fn_8006B474("FOOTING.BIN", 1);
}

extern "C" void fn_8014935C(void) {}

void Footing::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Footing(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80529D68 = {
    "FOOTING", fn_80149360, fn_8014935C, Footing::Create, 0, 0, 0x1130, 1, 0, 10, s_paramInfo,
};
