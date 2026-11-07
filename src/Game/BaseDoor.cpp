/* The BASE DOOR gadget: Door::SetBaseDoor (behaviour in the class comment). File name inferred
 * from the class name. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The constructor binds OnCollision to the box probe's handle target with
 * boost::bind(&SetBaseDoor::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/BaseDoor.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliBase.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The model file of both models (name inferred). */
static const char *s_modelFile = "BASEDOOR.BIN";
/* The motion model's entry is the "model" parameter's entry plus this (name inferred). */
enum { MOTION_MODEL_OFFSET = 1000 };

namespace Door {

const char *SetBaseDoor::s_name = "SetBaseDoor";

SetBaseDoor::SetBaseDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CD90(0x22, 4);
    m_modelIndex = m_params.model;
    LoadModels(m_modelIndex);
    ShowModels(0);

    /* The probe's local reference ends before ReadOwner, as in the original (0x8025773C). */
    {
        Fn8040Vector3 center(m_params.x, m_params.y + m_params.height, m_params.z);
        Fn8040Vector3 size(m_params.width, m_params.height, m_params.depth);
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), size, 0x40000, 10, 0,
                        &center, &lbl_805E07B8, 0);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&SetBaseDoor::OnCollision, this, _1)));
    }

    ReadOwner(owner);
    UpdateModelMatrix();
}

SetBaseDoor::~SetBaseDoor() {}

void SetBaseDoor::UpdateBody(float dt) {
    m_flags.Reset(0);
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_motionModel.get() != 0) {
        m_motionModel->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
    if (m_flags.Test(1)) {
        fn_801D53C4(fn_80053164(), 0x7849, &m_charInfo->m_position, this, 3, 0, 2);
    }

    switch (m_state) {
    case 0:
        if (m_params.type == 1) {
            m_state = 1;
        } else {
            m_state = 2;
        }

        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        if (m_motionModel.get() != 0) {
            m_motionTarget = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_motionTarget;
            m_motionModel->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        break;

    case 1:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = 5;
            ShowModels(1);
            if (m_motionModel.get() != 0) {
                m_motionModel->fn_80160A84(m_modelIndex + MOTION_MODEL_OFFSET, 1);
            }
            if (m_motionTarget.get() != 0) {
                m_motionTarget->fn_80167448(-1);
            }
            if (m_target.get() != 0) {
                m_target->fn_80166C70();
            }
            fn_801D54F4(fn_80053164(), 0x783C, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 2:
        if (m_flags.Test(0)) {
            ShowModels(1);
            if (m_motionModel.get() != 0) {
                m_motionModel->fn_80160A84(m_modelIndex + MOTION_MODEL_OFFSET, 1);
            }
            if (m_motionTarget.get() != 0) {
                m_motionTarget->fn_80167448(-1);
            }
            if (m_target.get() != 0) {
                m_target->fn_80166C70();
            }
            m_state = 3;
            m_flags.Set(1);
        }
        break;

    case 3:
        if (m_motionModel->fn_80160040(m_modelIndex + MOTION_MODEL_OFFSET)) {
            m_state = 4;
            m_flags.Reset(1);
            fn_801D54F4(fn_80053164(), 0x7847, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 4:
        if (!m_flags.Test(0)) {
            if (m_motionModel.get() != 0) {
                m_motionModel->fn_80160A84(m_modelIndex + MOTION_MODEL_OFFSET, 2);
            }
            m_state = 6;
            m_flags.Set(1);
        }
        break;

    case 6:
        if (m_motionModel->fn_80160040(m_modelIndex + MOTION_MODEL_OFFSET)) {
            m_state = 2;
            ShowModels(0);

            boost::weak_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(m_modelIndex);

            if (!motion.expired()) {
                if (boost::shared_ptr<_stMtnPlayWork> work = motion.lock()) {
                    stMtnRequestStartFrame(work.get(), 0, -1.0f);
                }
            }
            if (m_target.get() != 0) {
                m_target->fn_80166B70();
            }
            m_flags.Reset(1);
            fn_801D54F4(fn_80053164(), 0x7847, &m_charInfo->m_position, 0, 1);
        }
        break;
    }

    UpdateModelMatrix();
}

void SetBaseDoor::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_params.model != m_modelIndex) {
        m_modelIndex = m_params.model;
        m_target.reset();
        ReleaseModels();
        LoadModels(m_modelIndex);
        ShowModels(0);
        UpdateModelMatrix();
    }

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }

    if (m_handle.get() != 0) {
        Fn8040Vector3 center(m_params.x, m_params.y + m_params.height, m_params.z);

        m_handle->m_probe->UnkSlot17(&center, -2);
        m_handle->m_probe->SetSize(m_params.width, -2, m_params.height, m_params.depth);
        fn_8015BE64(m_handle.get());
    }
}

void SetBaseDoor::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_flags.Clear();
}

void SetBaseDoor::UpdateModelMatrix() {
    if (m_model.get() != 0 && m_motionModel.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
        m_motionModel->fn_801633DC(&matrix);
    }
}

void SetBaseDoor::OnCollision(ColliDetectArgs &args) {
    if (fn_8015AC84(args)) {
        m_flags.Set(0);
    }
}

void SetBaseDoor::LoadModels(int index) {
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(index, 0);
    if (m_model->fn_80160C28(index)) {
        m_model->fn_8015FF60();
        m_model->fn_80160A84(index, 1);
    }
    m_model->fn_80161FC8(index, boost::weak_ptr<CharInfo>(m_charInfo));

    int motionIndex = index + MOTION_MODEL_OFFSET;

    m_motionModel = fn_8015F800(1, s_modelFile);
    m_motionModel->fn_80162A68(motionIndex, 0);
    m_motionModel->fn_80160C28(motionIndex);
    m_model->fn_8015FF60();
    m_motionModel->fn_80161BD8(motionIndex, boost::weak_ptr<CharInfo>(m_charInfo), motionIndex);
    m_motion = m_motionModel->fn_801602B4(motionIndex);
}

void SetBaseDoor::ReleaseModels() {
    if (m_motionModel.get() != 0) {
        m_motionModel.reset();
    }
    if (m_model.get() != 0) {
        m_model.reset();
    }
    m_motion.reset();
}

void SetBaseDoor::ShowModels(int open) {
    switch (open) {
    case 0:
        if (m_model.get() != 0) {
            m_model->fn_8016271C();
            m_model->fn_8016111C(m_modelIndex, -1);
        }
        if (m_motionModel.get() != 0) {
            m_motionModel->fn_80162790();
            m_motionModel->fn_80161230(m_modelIndex + MOTION_MODEL_OFFSET, -1);
        }
        break;
    case 1:
        if (m_model.get() != 0) {
            m_model->fn_80162790();
            m_model->fn_80161230(m_modelIndex, -1);
        }
        if (m_motionModel.get() != 0) {
            m_motionModel->fn_8016271C();
            m_motionModel->fn_8016111C(m_modelIndex + MOTION_MODEL_OFFSET, -1);
        }
        break;
    }
}

} // namespace Door

/* The names of the values of the "type" parameter. */
static const char *s_typeLabels[2] = {"type : normal", "type : key"};

/* The label callback of the descriptor's first parameter (behaviour inferred). */
static const char *fn_802566F8(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

/* The descriptor's first hook: passes "BASEDOOR.BIN" and 1 to fn_8006B474. Its signature is
 * inferred from the descriptor's hook type (GadgetTypeInfo::hookA). */
extern "C" void fn_802566CC(void) {
    fn_8006B474("BASEDOOR.BIN", 1);
}

/* The descriptor's second hook, an empty function (only blr). Its signature is inferred: an empty
 * body cannot show the parameters or the result. */
extern "C" void fn_802566C8(void) {}

void Door::SetBaseDoor::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetBaseDoor(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_802566F8, 0},
    {1, "model", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "width", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "height", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "depth", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80549D44 = {
    "BASE DOOR", fn_802566CC, fn_802566C8, Door::SetBaseDoor::Create, 0, 0, 0x13F1, 1,
    0,           10,          s_paramInfo,
};
