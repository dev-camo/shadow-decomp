/* The BAMine gadget (class described in Game/BAMine.h; the gadget's descriptor is "BA Mine"). File
 * name inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The command handler and the two attack callbacks are boost::function objects built with
 * boost::bind(&BAMine::Method, this, _1); their managers, invokers and copy functions come from the
 * boost headers. */
/* Matching workaround: TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names
 * appear in the unit's .rodata; the headers' stand-in inline functions emit that RTTI data (see
 * TNumEdit.h). Revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/BAMine.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/CommonBomb.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8033F0F8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

static const char *s_modelFile = "BAMINE.BIN";

namespace BAMine {

const char *BAMine::s_name = "BAMine";

BAMine::BAMine(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8E, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&BAMine::OnCommand, this, _1));
    ReadOwner(owner);

    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, m_charInfo);

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 1;
        attack->field_0x38 = 1;
        attack->field_0x34 = 1.0f;
        attack->field_0x10.field_0x0 = AttackCallback(boost::bind(&BAMine::OnHit, this, _1));
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&BAMine::OnAttack, this, _1));
    }

    m_target = fn_801E7128(m_charInfo, 0x880);
    UpdateModelMatrix();
}

BAMine::~BAMine() {
    fn_801E7108(m_target);
}

void BAMine::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_RUN;
        break;

    case STATE_RUN:
        UpdateMove(dt);
        UpdateModelMatrix();
        break;

    case STATE_FINISH:
        fn_8015A794(this);
        m_flags |= 1;
        break;

    case STATE_EXPLODE:
        fn_801DD0BC(&m_charInfo->m_position, 1, 7, 1.0f);
        fn_801D54F4(fn_80053164(), 0x7934, &m_charInfo->m_position, 0, 1);
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void BAMine::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    Fn8040Vector3 start = m_charInfo->m_position;

    Fn8040Vector3 end(m_params.x, m_params.y, m_params.z);
    fn_8040CFF8(&end, &start);
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
}

void BAMine::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_START;
    m_startPosition = m_charInfo->m_position;

    m_angleSpeed = m_params.second != 0.0f ? 360.0f / m_params.second : 0.0f;
    m_timer = 0.0f;

    if (m_params.second <= 0.0f) {
        m_moveFlags.Set(MOVE_STILL);
    } else {
        m_moveFlags.Reset(MOVE_STILL);
    }
}

void BAMine::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_charInfo->m_position);
    }
}

void BAMine::UpdateMove(float dt) {
    if (!m_moveFlags.Test(MOVE_STILL)) {
        m_timer += dt;

        if (m_timer > m_params.second) {
            m_timer = 0.0f;
            m_moveFlags.Clear();
        } else if (m_timer > 0.5f * m_params.second + 0.5f) {
            if (!m_moveFlags.Test(MOVE_SOUND_HALF)) {
                m_moveFlags.Set(MOVE_SOUND_HALF);
                m_moveFlags.Set(MOVE_PLAY_SOUND);
            }
        } else if (m_timer > 0.5f) {
            if (!m_moveFlags.Test(MOVE_SOUND_START)) {
                m_moveFlags.Set(MOVE_SOUND_START);
                m_moveFlags.Set(MOVE_PLAY_SOUND);
            }
        }

        if (m_moveFlags.Test(MOVE_PLAY_SOUND)) {
            fn_801D54F4(fn_80053164(), 0x7933, &m_charInfo->m_position, 0, 1);
            m_moveFlags.Reset(MOVE_PLAY_SOUND);
        }
    }

    float angle = m_angleSpeed * m_timer;
    Fn8040Vector3 offset(m_params.x, m_params.y, m_params.z);
    float length = fn_8040CF50(&offset);

    offset = fn_8040CEE8(offset);

    fn_8040CE38(&offset, 0.5f * length - 0.5f * length * std::cos(fn_8000AB90(angle)));

    m_charInfo->m_position = m_startPosition + offset;
}

void BAMine::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_state == STATE_RUN) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x3D1) == 0) {
                return;
            }

            param.field_0x54 = 3;
            m_state = STATE_EXPLODE;
        }
    }
}

void BAMine::OnHit(AttackCallbackParam &param) {
    if (m_state == STATE_RUN) {
        m_state = STATE_EXPLODE;
    }
}

void BAMine::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14:
        if (m_state == STATE_RUN) {
            m_model->fn_80161880();
            fn_8033F0F8(&command, m_model, 0, &m_charInfo->m_position, &m_charInfo->m_rotation);
            m_model.reset();
            m_state = STATE_FINISH;
        }
        break;
    }
    command.m_flags |= 2;
}

} // namespace BAMine

extern "C" void fn_80300038(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_80300034(void) {}

void BAMine::BAMine::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BAMine(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "second", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055ABFC = {
    "BA Mine", fn_80300038, fn_80300034, BAMine::BAMine::Create, 0, 0, 0x17D5, 1,
    0,         10,          s_paramInfo,
};
