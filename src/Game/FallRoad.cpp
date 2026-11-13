/* FallRoad (class name from RTTI; the gadget's descriptor is "FALL_LARGE"): a road section that
 * starts the parameter Height above its owner's position, waits until the state of its tag in the
 * manager of fn_800CB69C reads 1, starts a motion of its model and, once that motion's work flag 3
 * is set, falls until it reaches the owner's height. On landing it calls fn_8000D664, plays sound
 * 0x7155, creates an object of kind 7 with fn_8034DD24 and then bounces about its resting height
 * (behaviour inferred). File name inferred from the class. Method names marked "(name inferred)"
 * in the header are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/FallRoad.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/weak_ptr.hpp"

const char *FallRoad::s_name = "FallRoad";

FallRoad::FallRoad(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_height, sizeof(m_height));
    m_charInfo = fn_8015CB88(0x95, 4, "FallRoad");
    m_model = new GadgetModelInfo(1, "FALLROAD.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    ReadOwner(m_owner);

    UnkWorld_80021B08 *world = fn_80021B08();
    m_probe = fn_800C7D04(world, &m_charInfo->m_position, Fn8040Vector3(200.0f, 25.0f, 80.0f),
                          UnkRotation3(0.0f, m_charInfo->m_rotation.y, 0.0f), 0, 1, 7, 0,
                          boost::weak_ptr<CharInfo>(m_charInfo), Fn8040Vector3(0.0f, -25.0f, 0.0f),
                          &lbl_805E07B8, 0);

    m_position = m_charInfo->m_position;
    m_position.y += m_height;
    m_velocity = lbl_805E07A0;
    UpdateModelMatrix();

    if (m_height == 0.0f) {
        m_state = 3;
    } else {
        m_state = 0;
    }
    m_bounce = 0.0f;
    m_prevBounce = 0.0f;
}

FallRoad::~FallRoad() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void FallRoad::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (fn_8016904C(fn_800CB69C(), m_tag) == 1) {
            m_model->fn_80160B18(0);
            m_state = 1;
        }
        break;

    case 1: {
        boost::weak_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(0);

        if (!motion.expired()) {
            /* The locked pointer is released before the call, as in the original. */
            _stMtnPlayWork *work = motion.lock().get();

            if (stMtnControlWorkFlag(work, 0, 3, 1)) {
                m_state = 2;
            }
        }
        break;
    }

    case 2: {
        Fn8040Vector3 acceleration(0.0f, -320.0f, 0.0f);

        fn_804103E8(&m_position, &m_velocity, &acceleration, 1.0f, dt);
        if (m_position.y <= m_charInfo->m_position.y) {
            m_position.y = m_charInfo->m_position.y;
            m_state = 3;
            m_bounce = -5.0f;

            m_target = fn_80166AF4();
            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167448(-1);

            fn_8000D664(fn_80009548(), 0, 0.35f, 2.2f, 10.0f);
            fn_801D54F4(fn_80053164(), 0x7155, &m_charInfo->m_position, 0, 1);
            fn_8034DD24(7, 2.0f, &m_charInfo->m_position, -2);
        }
        break;
    }

    case 3: {
        /* A damped oscillation of the model's height offset (behaviour inferred). */
        float delta;
        float bounce;

        bounce = m_bounce;
        delta = bounce - m_prevBounce;
        m_prevBounce = bounce;
        m_bounce = 0.8f * delta + -0.2f * bounce;
        break;
    }
    }

    UpdateModelMatrix();
}

void FallRoad::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    m_probe->SetPosition(&m_charInfo->m_position);
    m_probe->SetRotation(UnkRotation3(0.0f, m_charInfo->m_rotation.y, 0.0f));

    m_position = m_charInfo->m_position;
    m_position.y += m_height;
    UpdateModelMatrix();

    if (m_model != 0) {
        m_model->fn_80162EF4();
    }
    if (m_probe.get() != 0) {
        m_probe->UnkSlot26();
    }
}

void FallRoad::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_charInfo->m_rotation.x = m_charInfo->m_rotation.z = 0.0f;
    fn_800C9DBC(owner, &m_height, sizeof(m_height));
    fn_800C9E60(owner, &m_tag);
}

void FallRoad::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix(fn_8040B034(m_position));
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);

        Fn8040Vector3 offset(0.0f, m_bounce, 0.0f);
        fn_8040B59C(&matrix, offset);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_80329730(void) {
    fn_8006B474("FALLROAD.BIN", 1);
}

extern "C" void fn_8032972C(void) {}

void FallRoad::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new FallRoad(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo lbl_8055CF38[] = {
    {4, "Height", 0, 0, 0, 150.0f, 0.0f, 1000.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055CF88 = {
    "FALL_LARGE", fn_80329730, fn_8032972C, FallRoad::Create, 0, 0, 0x89C, 1, 0, 10, lbl_8055CF38,
};
