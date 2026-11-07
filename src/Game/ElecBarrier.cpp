/* ElecBarrier (class name from RTTI; the gadget's descriptor is "ELEC_BARRIER"): a barrier of the
 * electric circuit area. It shows the part of BARRIER.BIN selected by its parameter "BarrierType"
 * and, once the state of its com id becomes 1, plays a sound, fades the colour of that part over a
 * second and finishes (behaviour inferred). File name inferred from the class. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/ElecBarrier.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The three colours of the fade (names inferred). */
static UnkDebugColor_802E0E84 s_fadeColorStart(0xFF, 0x4C, 0x47, 0x00);
static UnkDebugColor_802E0E84 s_fadeColorMiddle(0xFF, 0xFF, 0xFF, 0xFF);
static UnkDebugColor_802E0E84 s_fadeColorEnd(0xFF, 0x00, 0x00, 0x00);

const char *ElecBarrier::s_name = "ElecBarrier";

ElecBarrier::ElecBarrier(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_barrierType, sizeof(m_barrierType));
    m_charInfo = fn_8015CB88(0x56, 4, "ElecBarrier");
    m_model = new GadgetModelInfo(1, "BARRIER.BIN");
    m_modelType = m_barrierType;
    m_model->fn_80162A68(m_modelType, 0);
    m_model->fn_80162424(m_modelType);
    ReadOwner(m_owner);
    m_model->fn_80161FC8(m_modelType, m_charInfo);

    boost::weak_ptr<CharaColliBase> probe = m_model->fn_80161614(m_modelType);

    if (!probe.expired()) {
        boost::shared_ptr<CharaColliBase> target = probe.lock();

        switch (m_modelType) {
        case 0:
        case 1:
            target->UnkSlot20(0x30302, -1);
            break;
        case 2:
        default:
            target->UnkSlot20(0x30301, -1);
            break;
        }
    }

    if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
        m_flags |= 1;
        return;
    }

    m_state = 0;
    UpdateModelMatrix();
}

ElecBarrier::~ElecBarrier() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecBarrier::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = 1;
            m_timer = 0.0f;
            fn_801D560C(fn_80053164(), 0x7242, 0, 0);
        }
        break;
    case 1: {
        m_timer += dt;
        if (m_timer > 1.0f) {
            fn_8015A794(this);
            m_flags |= 1;
            return;
        }

        UnkModelAtomicState *state = m_model->fn_80162298(m_modelType);

        if (state != 0) {
            if (m_timer < 0.1f) {
                fn_80154C6C(state, m_timer, s_fadeColorStart, s_fadeColorMiddle, 0.0f, 0.1f);
            } else {
                fn_80154C6C(state, m_timer, s_fadeColorMiddle, s_fadeColorEnd, 0.1f, 1.0f);
            }
        }
        break;
    }
    }

    UpdateModelMatrix();
}

void ElecBarrier::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    m_state = 0;
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        if (m_barrierType != m_modelType) {
            m_model->fn_80161880();
            m_model->fn_8016232C();
            m_model->fn_80162804();
            m_modelType = m_barrierType;
            m_model->fn_80162A68(m_modelType, 0);
            m_model->fn_80162424(m_modelType);
            m_model->fn_80161FC8(m_modelType, m_charInfo);
            m_barrierType = m_modelType;
            fn_800C9D68(owner, &m_barrierType, sizeof(m_barrierType));
        }
        m_model->fn_8016108C();
    }
}

void ElecBarrier::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
    fn_800C9DBC(owner, &m_barrierType, sizeof(m_barrierType));
}

void ElecBarrier::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_80263824(void) {
    fn_8006B474("BARRIER.BIN", 1);
}

extern "C" void fn_80263820(void) {}

void ElecBarrier::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecBarrier(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "BarrierType", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054BF48 = {
    "ELEC_BARRIER", fn_80263824, fn_80263820, ElecBarrier::Create, 0, 0, 0x7D8, 1, 0, 10,
    s_paramInfo,
};
