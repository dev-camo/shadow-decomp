/* FootingRoll (class name from RTTI; the gadget's descriptor has the same name): a footing that,
 * once the state of its com id becomes 1, moves back and forth by "MoveLength" with a half-cosine
 * ease and turns about one of its axes (behaviour inferred). File name inferred from the class.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include <cmath>
#include "Game/FootingRoll.h"
#include "Game/CharCommandUnit.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

const char *FootingRoll::s_name = "FootingRoll";

FootingRoll::FootingRoll(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x6D, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&FootingRoll::OnCommand, this, _1));

    ReadOwner(owner);
    m_model = fn_8015F800(1, "FOOTINGROLL.BIN");
    m_model->fn_80162A68(m_params.model, 1);
    m_model->fn_80161FC8(m_params.model * 2, m_charInfo);
    m_model->fn_80161FC8(m_params.model * 2 + 1, m_charInfo);
}

FootingRoll::~FootingRoll() {}

extern "C" void fn_80261658(void) {
    fn_8006B474("FOOTINGROLL.BIN", 1);
}

extern "C" void fn_80261654(void) {}

void FootingRoll::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new FootingRoll(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void FootingRoll::UpdateBody(float dt) {
    if (m_state == 0) {
        if (m_comId == 0 || fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = 1;
        }
    } else if (m_state == 1) {
        m_moveTimer += dt;
        switch (m_movePhase) {
        case 0:
            if (m_moveTimer < m_params.moveSec && !fn_8000A950(m_params.moveSec)) {
                float phase = m_moveTimer / m_params.moveSec;
                float wave = std::cos(phase * lbl_805FAD38);
                float fraction = 0.5f * (1.0f - wave);
                Fn8040Vector3 move(m_params.moveLengthX, m_params.moveLengthY,
                                   m_params.moveLengthZ);

                fn_800C9F14(m_owner, &m_charInfo->m_position);
                /* The forward leg calls the out-of-line copy of operator* (an argument of the
                 * inline AddVector) and the return leg the scale itself, as the original does. */
                if (m_moveBack == 0) {
                    AddVector(&m_charInfo->m_position, move * fraction);
                } else {
                    m_charInfo->m_position = m_charInfo->m_position + move * (1.0f - fraction);
                }
            } else {
                m_moveTimer = 0.0f;
                m_movePhase = 1;
            }
            break;

        case 1:
            if (m_moveTimer > m_params.movePauseSec) {
                m_moveTimer = 0.0f;
                if (m_moveBack == 0) {
                    m_moveBack = 1;
                } else {
                    m_moveBack = 0;
                }
                m_movePhase = 0;
            }
            break;
        }

        switch (m_params.rotType) {
        case 0:
            m_angle += m_params.rotSpd * dt;
            m_angle = fn_8040E99C(m_angle);
            break;

        case 1: {
            float direction = m_turnBack == 0 ? 1.0f : -1.0f;

            m_angle += direction * (m_params.rotSpd * dt);
            if (fn_8000A980(m_angle) > m_params.rotMax) {
                if (m_angle > 0.0f) {
                    m_angle = m_params.rotMax;
                } else {
                    m_angle = -m_params.rotMax;
                }
                if (m_turnBack == 0) {
                    m_turnBack = 1;
                } else {
                    m_turnBack = 0;
                }
            }
            break;
        }
        }

        UpdateModelMatrix();
    }

    m_model->fn_80162FB0(dt);
}

void FootingRoll::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    FootingRollParams previous = m_params;

    ReadOwner(owner);
    if (previous.model != m_params.model) {
        m_model->fn_80162804();
        m_model->fn_80161880();
        m_model->fn_80162A68(m_params.model, 0);
        m_model->fn_80161FC8(m_params.model * 2, m_charInfo);
        m_model->fn_80161FC8(m_params.model * 2 + 1, m_charInfo);
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws debug lines along the move and along the turning axis: the move line in the shared
     * debug colour and the axis line in a colour chosen by the axis (behaviour inferred). */
    Fn8040Vector3 axis;

    UnkSegment_80020A20 moveLine(m_charInfo->m_position,
                                 m_charInfo->m_position + Fn8040Vector3(m_params.moveLengthX,
                                                                        m_params.moveLengthY,
                                                                        m_params.moveLengthZ));

    fn_802E0AA0(moveLine, lbl_805F06D8, 5.0f);

    UnkDebugColor_802E0E84 color(0xFF, 0x00, 0x00, 0x00);

    axis.Set(0.0f, 0.0f, 0.0f);

    switch (m_params.rotAxis) {
    case 0:
        color.field_0x1 = 0xFF;
        axis.x = 1.0f;
        break;

    case 1:
        color.field_0x2 = 0xFF;
        axis.y = 1.0f;
        break;

    case 2:
        color.field_0x3 = 0xFF;
        axis.z = 1.0f;
        break;
    }

    axis = fn_8040D044(axis, m_charInfo->m_rotation.z);
    axis = fn_8040D118(axis, m_charInfo->m_rotation.x);
    axis = fn_8040D0AC(axis, m_charInfo->m_rotation.y);
    fn_8040CE38(&axis, 1000.0f);
    UnkSegment_80020A20 axisLine(m_charInfo->m_position, m_charInfo->m_position + axis);

    fn_802E0AA0(axisLine, color, 2.0f);
}

void FootingRoll::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
    m_params.rotSpd *= lbl_805FAD38 / 180.0f;
    m_params.rotMax *= lbl_805FAD38 / 180.0f;

    m_state = 0;
    m_movePhase = 0;
    m_moveTimer = 0.0f;
    m_moveBack = 0;
    m_turnBack = 0;
    m_angle = 0.0f;
}

void FootingRoll::UpdateModelMatrix() {
    Fn8040Matrix34 rotation;

    fn_8040C148(&rotation, &lbl_805E0720);
    fn_800C9ED4(m_owner, &m_charInfo->m_rotation);
    fn_8040B848(&rotation, m_charInfo->m_rotation.y);
    fn_8040B884(&rotation, m_charInfo->m_rotation.x);
    fn_8040B80C(&rotation, m_charInfo->m_rotation.z);
    switch (m_params.rotAxis) {
    case 0:
        fn_8040B884(&rotation, m_angle);
        break;

    case 1:
        fn_8040B848(&rotation, m_angle);
        break;

    case 2:
        fn_8040B80C(&rotation, m_angle);
        break;
    }
    fn_800494F8(&m_charInfo->m_rotation, &rotation);

    Fn8040Matrix34 matrix;
    Fn8040Matrix34 product;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8001E044(&product, &matrix, &rotation);
    fn_8040C148(&matrix, &product);
    m_model->fn_801633DC(&matrix);
}

void FootingRoll::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x17: {
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        if (transparent.m_alpha < 1.0f) {
            fn_8015FDD8(m_model.get(), transparent.m_alpha);
        } else {
            fn_8015FD10(m_model.get());
        }
        break;
    }
    }
    command.m_flags |= 2;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Model 0:Small 1:Large", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "MoveLengthX point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveLengthY point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveLengthZ point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveSec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MovePauseSec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {2, "RotType 0:OneWay 1:RT", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "RotAxis 0:X 1:Y 2:Z", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RotSpd deg/sec", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RotMax(RT Only) deg", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054BC24 = {
    "FootingRoll", fn_80261658, fn_80261654, FootingRoll::Create, 0, 0, 0x189F, 1, 0, 10,
    s_paramInfo,
};
