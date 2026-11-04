/* The BAShield gadget (class name from RTTI; file name inferred from the class). Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/BAShield.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_80301F4C.h"

static const char *s_modelFile = "BASHIELD.BIN";

namespace BAShield {

const char *BAShield::s_name = "BAShield";

BAShield::BAShield(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_type(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8A, 4);
    CopyFromOwner(owner);
    m_modelType = m_type;
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(m_modelType, 0);
    m_model->fn_80161FC8(m_modelType, m_charInfo);
    CreateTarget();
    if (m_target.get() != 0) {
        m_target->fn_80167010(1);
    }
    UpdateModelMatrix();
}

BAShield::~BAShield() {}

void BAShield::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    /* States: 0 after construction or a reset, 1 while following the switch, 2 as the sibling
     * BDHologram gadget sets it (never set here) (inferred). */
    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 1:
        /* The original has its own copy of the flag and sound blocks in each branch. */
        if (m_type == 0) {
            if (fn_80301F4C(m_comId)) {
                if (!m_flags.Test(0)) {
                    TurnOff();
                    m_flags.Set(0);
                }
            } else {
                if (m_flags.Test(0)) {
                    TurnOn();
                    m_flags.Reset(0);
                }
                fn_801D53C4(fn_80053164(), 0x793D, &m_charInfo->m_position, this, 3, 0, 2);
            }
        } else {
            if (fn_80301F4C(m_comId)) {
                if (m_flags.Test(0)) {
                    TurnOn();
                    m_flags.Reset(0);
                }
                fn_801D53C4(fn_80053164(), 0x793D, &m_charInfo->m_position, this, 3, 0, 2);
            } else {
                if (!m_flags.Test(0)) {
                    TurnOff();
                    m_flags.Set(0);
                }
            }
        }
        break;
    case 2:
        break;
    }
}

void BAShield::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousType = m_type;

    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        if (previousType != m_type) {
            m_target.reset();
            m_model->fn_80161880();
            m_model->fn_80162804();
            m_modelType = m_type;
            m_model->fn_80162A68(m_modelType, 0);
            m_model->fn_80161FC8(m_modelType, m_charInfo);
            CreateTarget();
            if (m_target.get() != 0) {
                m_target->fn_80167010(1);
            }
        }
        m_model->fn_8016108C();
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(0.0f);
    }
}

void BAShield::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_type, sizeof(m_type));
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_flags.Clear();
}

void BAShield::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void BAShield::TurnOff() {
    if (m_target.get() != 0) {
        m_target.reset();
        CreateTarget();
        m_target->fn_80167448(0);
    }
    if (m_model.get() != 0) {
        m_model->fn_80161230(m_modelType, -1);
    }
    fn_801D54F4(fn_80053164(), 0x793C, &m_charInfo->m_position, 0, 1);
}

void BAShield::TurnOn() {
    if (m_target.get() != 0) {
        m_target.reset();
        CreateTarget();
        m_target->fn_80167010(1);
    }
    if (m_model.get() != 0) {
        m_model->fn_8016111C(m_modelType, -1);
    }
    fn_801D54F4(fn_80053164(), 0x793B, &m_charInfo->m_position, 0, 1);
}

void BAShield::CreateTarget() {
    if (m_model.get() != 0) {
        m_target = fn_80166AF4();

        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
}

} // namespace BAShield

extern "C" void fn_80301504(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_80301500(void) {}

void BAShield::BAShield::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BAShield(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055ACF8 = {
    "BA Shield", fn_80301504, fn_80301500, BAShield::BAShield::Create, 0, 0, 0x1901, 1,
    0,           10,          s_paramInfo,
};
