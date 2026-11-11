/* The TargetSwitch gadget (class described in Game/TargetSwitch.h; the gadget's descriptor is
 * "TARGETSWITCH"). File name inferred from the class. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first).
 *
 * The attack callback is a boost::function object built with
 * boost::bind(&TargetSwitch::OnAttack, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/TargetSwitch.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *TargetSwitch::s_name = "TargetSwitch";

TargetSwitch::TargetSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(4, 4);
    m_model = fn_8015F800(0, "TARGETSWITCH.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&TargetSwitch::OnAttack, this, _1));
    }

    m_model->fn_80160BA0("mtn");
    m_model->fn_8015FF60();
    m_target = fn_801E72DC(0x800);
    m_state = STATE_START;
    CopyFromOwner(m_owner);
    UpdateModelMatrix();
    SetColor(m_hitpoints);
}

TargetSwitch::~TargetSwitch() {}

void TargetSwitch::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_GO;
        break;

    case STATE_GO: {
        m_timer += dt;

        float angle = m_angleSpeed * m_timer;

        if (angle >= 180.0f) {
            m_state = STATE_WAIT;
            m_waitTimer = m_params.pause;
        }
        UpdatePosition(angle);
        break;
    }

    case STATE_RETURN: {
        m_timer += dt;

        float angle = m_angleSpeed * m_timer;

        if (m_timer > m_params.sec) {
            m_timer = 0.0f;
            m_state = STATE_WAIT;
            m_waitTimer = m_params.pause;
        }
        UpdatePosition(angle);
        break;
    }

    case STATE_WAIT:
        m_waitTimer -= dt;
        if (m_waitTimer <= 0.0f) {
            if (m_timer > 0.0f) {
                m_state = STATE_RETURN;
            } else {
                m_state = STATE_GO;
            }
        }
        break;

    case STATE_DESTROYED:
        if (m_owner != 0) {
            u8 comId = 0;

            fn_800C9E60(m_owner, &comId);
            fn_801690D0(fn_800CB69C(), comId, 1);
            fn_800C9B34(m_owner);
        }
        fn_8015D794(&m_charInfo->m_position, 0xA9);
        m_flags |= 1;
        break;
    }

    UpdateModelMatrix();
}

void TargetSwitch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();

        Fn8040Vector3 start = m_charInfo->m_position;

        Fn8040Vector3 end(m_params.x, m_params.y, m_params.z);
        fn_8040CFF8(&end, &start);
        fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    }
}

void TargetSwitch::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_timer = 0.0f;
    m_angleSpeed = m_params.sec > 0.0f ? 360.0f / m_params.sec : 0.0f;
    m_hitpoints = m_params.hitpoint;
}

void TargetSwitch::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

void TargetSwitch::UpdatePosition(float angle) {
    Fn8040Vector3 start;
    /* The far end of the movement; computed here but not used (original behaviour). */
    Fn8040Vector3 end(m_params.x, m_params.y, m_params.z);

    fn_800C9F14(m_owner, &start);
    fn_8040CFF8(&end, &start);

    Fn8040Vector3 offset(m_params.x, m_params.y, m_params.z);
    float halfLength = 0.5f * fn_8040CF50(offset);

    offset = fn_8040CEE8(offset);

    fn_8040CE38(&offset, halfLength - halfLength * std::cos(fn_8000AB90(angle)));

    m_charInfo->m_position = offset + start;
}

void TargetSwitch::OnAttack(AttackCallbackParam &param) {
    if (m_state != STATE_DESTROYED) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 2) {
                param.field_0x54 = 5;
                return;
            }
        }

        m_hitpoints -= param.field_0x50;
        if (m_hitpoints <= 0.0f) {
            param.field_0x54 = 3;
            m_state = STATE_DESTROYED;
            fn_801D54F4(fn_80053164(), 0x100F, &m_charInfo->m_position, 0, 1);
        } else {
            param.field_0x54 = 1;
            fn_801D54F4(fn_80053164(), 0x100E, &m_charInfo->m_position, 0, 1);
        }

        if (m_model.get() != 0) {
            m_model->fn_80160B18(0);
        }
        SetColor(m_hitpoints);
    } else {
        param.field_0x54 = 5;
    }
}

void TargetSwitch::SetColor(float hitpoints) {
    if (m_model.get() != 0) {
        m_model->fn_8016232C();

        if (hitpoints >= 8.0f) {
            m_model->fn_8016239C("purple");
        } else if (hitpoints >= 6.0f) {
            m_model->fn_8016239C("blue");
        } else if (hitpoints >= 4.0f) {
            m_model->fn_8016239C("green");
        } else if (hitpoints >= 2.0f) {
            m_model->fn_8016239C("yellow");
        } else {
            m_model->fn_8016239C("red");
        }
    }
}

extern "C" void fn_801245F4(void) {
    fn_8006B474("TARGETSWITCH.BIN", 1);
}

extern "C" void fn_801245F0(void) {}

void TargetSwitch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new TargetSwitch(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "sec", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "pause", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "hitpoint", 2, 2, 10, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80527D0C = {
    "TARGETSWITCH", fn_801245F4, fn_801245F0, TargetSwitch::Create, 0, 0, 0x16, 1, 0, 10,
    s_paramInfo,
};
