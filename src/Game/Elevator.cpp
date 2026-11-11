/* The ELEVATOR gadget (class described in Game/Elevator.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler and the model's collision callback are boost::function
 * objects built with boost::bind(&Elevator::method, this, _1); their managers, invokers and copy
 * functions come from the boost headers. */
#include <cstdio>
#include "Game/Elevator.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The model file of the elevator. */
static const char *s_modelFile = "ELEVATOR.BIN";

namespace Elevator {

const char *Elevator::s_name = "Elevator";

Elevator::Elevator(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x24, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Elevator::OnCommand, this, _1));

    ReadOwner(m_owner);
    m_loadedModelIndex = m_params.model;
    m_model = fn_8015F800(1, s_modelFile);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, s_modelFile));
    if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }

    SetupModel();
    UpdateModelMatrix();
}

Elevator::~Elevator() {}

void Elevator::UpdateBody(float dt) {
    m_hitFlags.Reset(FLAG_TOUCHED);

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case STATE_START:
        if (m_comId > 0) {
            m_state = STATE_WAITING;
        } else {
            m_state = STATE_AT_START;
        }

        if (m_target.get() == 0 && m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        break;

    case STATE_WAITING:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            if (m_hitFlags.Test(FLAG_TOUCHED)) {
                m_state = STATE_TO_DESTINATION;
            } else {
                m_state = STATE_AT_START;
            }
        }
        break;

    case STATE_AT_DESTINATION:
        if (IsNearestAway()) {
            m_state = STATE_TO_START;
        }
        break;

    case STATE_TO_DESTINATION: {
        if (IsNearestAway()) {
            m_state = STATE_TO_START;
            break;
        }

        if (m_params.distance < 0.0f) {
            m_charInfo->m_position.y -= dt * (30.0f * m_params.speed);

            float destination = m_start.y + m_params.distance;
            if (m_charInfo->m_position.y < destination) {
                m_charInfo->m_position.y = destination;
                m_state = STATE_AT_DESTINATION;
                fn_801D54F4(fn_80053164(), m_endSound, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D53C4(fn_80053164(), m_loopSound, 0, this, 2, 0, lbl_805F4A80);
            }
        } else {
            m_charInfo->m_position.y += dt * (30.0f * m_params.speed);

            float destination = m_start.y + m_params.distance;
            if (m_charInfo->m_position.y > destination) {
                m_charInfo->m_position.y = destination;
                m_state = STATE_AT_DESTINATION;
                fn_801D54F4(fn_80053164(), m_endSound, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D53C4(fn_80053164(), m_loopSound, 0, this, 2, 0, lbl_805F4A80);
            }
        }
        UpdateModelMatrix();
        break;
    }

    case STATE_TO_START:
        if (m_params.distance < 0.0f) {
            m_charInfo->m_position.y += dt * (30.0f * m_params.speed);

            if (m_charInfo->m_position.y > m_start.y) {
                m_charInfo->m_position.y = m_start.y;
                m_state = STATE_AT_START;
                fn_801D54F4(fn_80053164(), m_endSound, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D53C4(fn_80053164(), m_loopSound, 0, this, 2, 0, lbl_805F4A80);
            }
        } else {
            m_charInfo->m_position.y -= dt * (30.0f * m_params.speed);

            if (m_charInfo->m_position.y < m_start.y) {
                m_charInfo->m_position.y = m_start.y;
                m_state = STATE_AT_START;
                fn_801D54F4(fn_80053164(), m_endSound, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D53C4(fn_80053164(), m_loopSound, 0, this, 2, 0, lbl_805F4A80);
            }
        }
        UpdateModelMatrix();
        break;
    }
}

void Elevator::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 loadedModel = m_params.model;

    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() == 0) {
        return;
    }

    if (m_params.model != loadedModel) {
        m_target.reset();
        ReleaseModel();
        if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
            return;
        }

        m_loadedModelIndex = m_params.model;
        SetupModel();
    }

    Fn8040Vector3 end(m_charInfo->m_position);
    end.y += m_params.distance;
    fn_802E0B58(UnkSegment_80020A20(m_charInfo->m_position, end), lbl_805F06D8);

    Fn8040Vector3 center(m_charInfo->m_position);
    UnkRotation3 rotation = m_charInfo->m_rotation;
    rotation.x += DegreesToRadians(90.0f);
    fn_802E0C60(center, rotation, 0, 10.0f, m_params.range);

    m_model->fn_8016108C();
}

void Elevator::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_state = STATE_START;
    m_start = m_charInfo->m_position;
}

void Elevator::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

bool Elevator::IsNearestAway() {
    boost::weak_ptr<CharInfo> nearest = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position);

    if (boost::shared_ptr<CharInfo> character = nearest.lock()) {
        Fn8040Vector3 other(character->m_position);
        Fn8040Vector3 self(m_charInfo->m_position);

        self.y = 0.0f;
        other.y = 0.0f;

        const Fn8040Vector3 &difference = other - self;
        if (fn_8040CF2C(difference) >= m_params.range * m_params.range) {
            return true;
        }
    }
    return false;
}

void Elevator::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_state == STATE_AT_START || m_state == STATE_TO_START) {
            m_state = STATE_TO_DESTINATION;
        }
        break;
    }
}

void Elevator::SetupModel() {
    m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_params.model);
    m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);

    const char *nodeName = m_crashInfo->fn_802087BC();
    if (nodeName != 0) {
        m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo));
        m_model->m_collisionCallback =
            ColliDetectCallback(boost::bind(&Elevator::OnCollision, this, _1));
    }

    const char *soundName = m_crashInfo->fn_80208768();

    m_endSound = 0xFFFF;
    m_loopSound = 0xFFFF;
    if (soundName != 0) {
        char name[64];

        sprintf(name, "%s_end", soundName);
        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            m_endSound = record->field_0x8;
        }

        sprintf(name, "%s_loop", soundName);
        record = fn_80208888(m_resource, name);
        if (record != 0) {
            m_loopSound = record->field_0x8;
        }
    }
}

void Elevator::ReleaseModel() {
    m_model->fn_80161880();
    m_model->fn_80162804();
    m_crashInfo.reset();
}

void Elevator::OnCollision(ColliDetectArgs &args) {
    if (fn_8015AC84(args)) {
        m_hitFlags.Set(FLAG_TOUCHED);
    }
}

} // namespace Elevator

/* The descriptor's load hook: adds the model file to the list of resources to load (name
 * inferred). It takes the string's address instead of loading s_modelFile. */
extern "C" void fn_80147B34(void) {
    fn_8006B474("ELEVATOR.BIN", 1);
}

extern "C" void fn_80147B30(void) {}

void Elevator::Elevator::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Elevator(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "distance", 0, 0, 0, 10.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "speed", 0, 0, 0, 0.5f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "range", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80529CB0 = {
    "ELEVATOR", fn_80147B34, fn_80147B30, Elevator::Elevator::Create, 0, 0, 0x1132, 1,
    0,          10,          s_paramInfo,
};
