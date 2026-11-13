/* The Magma gadget (class described in Game/Magma.h; the gadget's descriptor is "MAGMA"). File
 * name inferred from the namespace. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/Magma.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

namespace Magma {

const char *SetMagma::s_name = "SetMagma";

SetMagma::SetMagma(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x7C, 4);
    ReadOwner(owner);

    m_modelEntry = m_params.model;
    m_model = fn_8015F800(1, "MAGMA.BIN");
    m_model->fn_80162A68(m_modelEntry, 0);
    m_model->fn_80161FC8(m_modelEntry, m_charInfo);
    UpdateModelMatrix();
}

SetMagma::~SetMagma() {}

void SetMagma::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_WAIT;
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        break;

    case STATE_WAIT:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = STATE_MOVE;
        }
        break;

    case STATE_MOVE:
        if (UpdateMove(dt)) {
            m_state = STATE_END;
        }
        UpdateModelMatrix();
        break;

    case STATE_END:
        break;
    }
}

void SetMagma::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousModel = m_params.model;

    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0 && previousModel != m_params.model) {
        m_target.reset();
        m_model->fn_80161880();
        m_model->fn_80162804();

        m_modelEntry = m_params.model;
        m_model->fn_80162A68(m_modelEntry, 0);
        m_model->fn_80161FC8(m_modelEntry, m_charInfo);
    }

    Fn8040Vector3 start = m_charInfo->m_position;

    Fn8040Vector3 end(0.0f, m_params.y, 0.0f);
    fn_8040CFF8(&end, &start);
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
}

void SetMagma::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_startPosition = m_charInfo->m_position;
    m_state = STATE_START;
}

void SetMagma::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

bool SetMagma::UpdateMove(float dt) {
    if (m_params.y < 0.0f) {
        float end = m_startPosition.y + m_params.y;

        m_charInfo->m_position.y -= m_params.speed * dt;
        if (m_charInfo->m_position.y < end) {
            m_charInfo->m_position.y = end;
            return true;
        }
    } else {
        float end = m_startPosition.y + m_params.y;

        m_charInfo->m_position.y += m_params.speed * dt;
        if (m_charInfo->m_position.y > end) {
            m_charInfo->m_position.y = end;
            return true;
        }
    }
    return false;
}

} // namespace Magma

extern "C" void fn_8027B968(void) {
    fn_8006B474("MAGMA.BIN", 1);
}

extern "C" void fn_8027B964(void) {}

void Magma::SetMagma::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetMagma(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 9, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "speed", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054DC60 = {
    "MAGMA", fn_8027B968, fn_8027B964, Magma::SetMagma::Create, 0, 0, 0x1839, 1, 0, 10, s_paramInfo,
};
