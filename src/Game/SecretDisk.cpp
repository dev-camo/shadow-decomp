/* The SecretDisk gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&SecretDisk::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/SecretDisk.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

static const char *s_modelFile = "SECRETDISK.BIN";

/* The height of the second object above the character info's position (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_objectHeight(10.0f);

namespace SecretDisk {

const char *SecretDisk::s_name = "SecretDisk";

SecretDisk::SecretDisk(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x40, 4);
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&SecretDisk::OnCollision, this, _1));
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80160A84(0, 1);
    m_object = fn_802B4254(2, 0);
    fn_802B4510(m_object.get(), 1);
    UpdateModelMatrix();
}

SecretDisk::~SecretDisk() {}

void SecretDisk::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        m_state = 1;
        break;
    case 2: {
        boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

        if (mission.get() != 0) {
            mission->fn_8016B224(1, 0);
        }

        Fn8040Vector3 effectPosition = m_charInfo->m_position;
        effectPosition.y += -6.0f;
        fn_80168B38(&effectPosition, &m_charInfo->m_rotation, 0x33);
        fn_801D560C(fn_80053164(), 0xE017, 0, 0);
        fn_8015A768(this);
        m_flags |= 1;
        break;
    }
    }

    UpdateModelMatrix();
}

void SecretDisk::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
}

void SecretDisk::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
}

void SecretDisk::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
    if (m_object.get() != 0) {
        Fn8040Vector3 objectPosition = m_charInfo->m_position;
        objectPosition.y += s_objectHeight.value;
        fn_802B44C8(m_object.get(), &objectPosition);
    }
}

void SecretDisk::OnCollision(ColliDetectArgs &args) {
    if (m_state == 1 && fn_8015AC84(args)) {
        m_state = 2;
    }
}

} // namespace SecretDisk

extern "C" void fn_802F9E44(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_802F9E40(void) {}

void SecretDisk::SecretDisk::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SecretDisk(parent, owner);
}

GadgetTypeInfo lbl_8055A5BC = {
    "SECRET DISK",
    fn_802F9E44,
    fn_802F9E40,
    SecretDisk::SecretDisk::Create,
    0,
    0,
    0x1453,
    1,
    0,
    10,
    0,
};
