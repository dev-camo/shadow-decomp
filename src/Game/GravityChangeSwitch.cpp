/* The GravityChangeSwitch gadget (behaviour in the class comment). File name inferred from the
 * class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&GravityChangeSwitch::OnCollision, this, _1); its manager, invoker and copy functions
 * come from the boost headers. */
#include "Game/GravityChangeSwitch.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGravity_80293928.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

const char *GravityChangeSwitch::s_name = "GravityChangeSwitch";

/* The direction passed to fn_80168C48 with the effect's position (0, 0, -1), copied from a
 * temporary as the original's start-up function does. */
static Fn8040Vector3 s_effectDirection = Fn8040Vector3(0.0f, 0.0f, -1.0f);

GravityChangeSwitch::GravityChangeSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_hitFlags(0) {
    m_name = s_name;
    m_effect.reset();
    m_charInfo = fn_8015CD90(0x66, 4);
    CopyFromOwner(owner);

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xA400);

    m_model = fn_8015F800(1, "GRAVITYCHANGESWITCH.BIN");
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&GravityChangeSwitch::OnCollision, this, _1));

    UpdateState();
}

GravityChangeSwitch::~GravityChangeSwitch() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }
}

extern "C" void fn_80246DF4(void) {
    fn_80293B1C();
    fn_8006B474("GRAVITYCHANGESWITCH.BIN", 1);
}

extern "C" void fn_80246DD4(void) {
    fn_80293AE4();
}

void GravityChangeSwitch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GravityChangeSwitch(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void GravityChangeSwitch::UpdateBody(float dt) {
    m_model->fn_80162FB0(dt);

    if (m_hitFlags.Test(0)) {
        if (m_effect.expired()) {
            m_effect = fn_80168C48(&m_charInfo->m_position, &s_effectDirection, 0xE);
        }

        fn_80293928(m_params.gravity);
        fn_801D54F4(fn_80053164(), 0x773B, &m_charInfo->m_position, 0, 1);
        fn_8034DF48(5, -2);

        if (m_comId != 0) {
            fn_801690D0(fn_800CB69C(), m_comId, 1);
        }

        m_hitFlags.Reset(0);
    } else {
        UpdateState();
    }
}

void GravityChangeSwitch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws a debug line from the character info's position to the point 50 times the gravity
     * table's vector for the GRAVITY index away in the opposite direction. */

    Fn8040Vector3 direction = *fn_80293B60(m_params.gravity);
    fn_8040CE38(&direction, -50.0f);
    const Fn8040Vector3 &end = m_charInfo->m_position + direction;

    UnkSegment_80020A20 segment(m_charInfo->m_position, end);

    fn_802E0AA0(segment, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 1.0f);
}

void GravityChangeSwitch::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
    m_state = 0;
}

void GravityChangeSwitch::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_target.get(), &m_charInfo->m_position);
}

void GravityChangeSwitch::UpdateState() {
    if (fn_80293AC8() == m_params.gravity) {
        if (m_state != 2) {
            SetState(2);
        }
    } else {
        if (m_state != 1) {
            SetState(1);
        }
    }
}

void GravityChangeSwitch::SetState(int state) {
    m_model->fn_80162804();

    switch (state) {
    case 1:
        m_model->fn_80162A68(0, 0);
        UpdateModelMatrix();
        m_model->fn_801612B8();
        m_target->SetFlags(1);
        break;
    case 2:
        m_model->fn_80162A68(1, 0);
        UpdateModelMatrix();
        m_model->fn_801612E0();
        m_target->ClearFlags(1);
        break;
    }

    m_state = state;
}

void GravityChangeSwitch::OnCollision(ColliDetectArgs &args) {
    if (fn_8015D2EC(args.m_hits)) {
        m_hitFlags.Set(0);
    }
}

static GadgetParamInfo lbl_805488B4[] = {
    {1, "GRAVITY", 0, 0, 5, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80548904 = {
    "GravityChangeSwitch",
    fn_80246DF4,
    fn_80246DD4,
    GravityChangeSwitch::Create,
    0,
    0,
    0x14B4,
    1,
    0,
    10,
    lbl_805488B4,
};
