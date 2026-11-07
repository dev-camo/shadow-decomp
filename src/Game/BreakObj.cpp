/* The BreakObj gadget (class described in Game/BreakObj.h), descriptor "BRK_OBJ". File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The attack callback is a boost::function object built with
 * boost::bind(&BreakObj::OnAttack, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/BreakObj.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cstring>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *BreakObj::s_name = "BreakObj";

BreakObj::BreakObj(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_kind, sizeof(m_kind));
    m_charInfo = fn_8015CD90(0x11, 4);
    m_model = new GadgetModelInfo(0, "BREAKOBJ.BIN");
    m_loadedKind = m_kind;
    m_model->fn_80162A68(m_loadedKind, 0);

    if (m_model->fn_80161FC8(m_loadedKind, m_charInfo)) {
        boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(m_loadedKind);

        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&BreakObj::OnAttack, this, _1));
        }
    }

    m_state = STATE_RESET;
    ReadOwner(m_owner);
    UpdateModelMatrix();
}

BreakObj::~BreakObj() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void BreakObj::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_RESET:
        m_state = STATE_WAITING;
        break;
    case STATE_HIT:
        StartBreakEffect();
        fn_800C9B34(m_owner);
        m_flags |= 1;
        break;
    }
}

void BreakObj::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model != 0) {
        if (m_kind != m_loadedKind) {
            if (!m_model->fn_80162EA8(m_kind)) {
                return;
            }

            m_loadedKind = m_kind;
            m_model->fn_80161880();
            m_model->fn_80162804();
            m_model->fn_80162A68(m_loadedKind, 0);

            if (m_model->fn_80161FC8(m_loadedKind, m_charInfo)) {
                boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(m_loadedKind);

                if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
                    attack->field_0x54 = 0;
                    attack->field_0x55 = 1;
                    attack->field_0x1C.field_0x0 =
                        AttackCallback(boost::bind(&BreakObj::OnAttack, this, _1));
                }
            }
        }

        m_model->fn_8016108C();
    }
}

void BreakObj::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_kind, sizeof(m_kind));
}

void BreakObj::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void BreakObj::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 2) != 0) {
                param.field_0x54 = 5;
                return;
            }

            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                switch (m_kind) {
                case 0:
                    fn_8020642C(attacker, 0x14);
                    break;
                case 1:
                    fn_8020642C(attacker, 0x15);
                    break;
                }
            }

            param.field_0x54 = 3;
            m_state = STATE_HIT;
            return;
        }
    }

    param.field_0x54 = 5;
}

void BreakObj::StartBreakEffect() {
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(m_kind + 1000);

        if (entry != 0) {
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.3f;
            parameters.field_0x44 =
                static_cast<RpClump *>(fn_80012700(fn_800122D4(), 0, entry->name));
            fn_8015D93C(&parameters);

            if (strcmp(entry->name, "BRKOBJGUNCRSH.DFF") == 0) {
                fn_801D54F4(fn_80053164(), 0x101C, &m_charInfo->m_position, 0, 1);
            } else {
                fn_801D54F4(fn_80053164(), 0x101D, &m_charInfo->m_position, 0, 1);

                Fn8040Vector3 position = m_charInfo->m_position;

                position.y += 10.0f;
                fn_8015D794(&position, 0xB9);
            }
        }
    }
}

extern "C" void fn_800F13A4(void) {
    fn_8006B474("BREAKOBJ.BIN", 1);
}

extern "C" void fn_800F13A0(void) {}

void BreakObj::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BreakObj(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "kind", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80524E90 = {
    "BRK_OBJ", fn_800F13A4, fn_800F13A0, BreakObj::Create, 0, 0, 0x1B, 1, 0, 10, s_paramInfo,
};
