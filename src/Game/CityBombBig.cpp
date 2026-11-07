/* The CITY_BOMB_BIG gadget (class described in Game/CityBombBig.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The command handler and the attack callback are boost::function objects built with
 * boost::bind(&CityBombBig::Method, this, _1); their managers, invokers and copy functions come
 * from the boost headers. */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included for the RTTI data that
 * their never-called stand-in functions emit (see TNumEdit.h); this file uses neither class.
 * TNumEdit.h comes first, in the order their names appear in the unit's .rodata. Revisit when the
 * original inline functions behind that data are found. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CityBombBig.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/CommonBomb.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Mission.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The height of m_object2 above the character info's position and the value passed to its
 * fn_802B4498 (names inferred from their uses).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the unknown way the original
 * initialises these floats at startup. The original sets them with a start-up function in .ctors
 * (0x802F46D0; a constant float initialiser would be emitted as initialised .sdata data instead),
 * and the shared stand-in reproduces those two float stores. The bytes do not establish the
 * variables' original type. Revisit when the original initialiser is found. */
static UnkStartupFloat_800E2570 s_objectHeight(34.0f);
static UnkStartupFloat_800E2570 s_objectValue(1200.0f);

const char *CityBombBig::s_name = "CityBombBig";

CityBombBig::CityBombBig(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x54, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&CityBombBig::OnCommand, this, _1));
    ReadOwner(owner);

    m_model = fn_8015F800(1, "CITYBOMBBIG.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, m_charInfo);

    {
        boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(0).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&CityBombBig::OnAttack, this, _1)));
        }
    }

    m_object = fn_801E7128(m_charInfo, 0xAC00);
    m_object2 = fn_802B4254(0, 0);
    fn_802B4498(m_object2.get(), s_objectValue.value);
    fn_802B4510(m_object2.get(), 1);
    UpdateModelMatrix();

    m_target = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 weakTarget;

        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
    m_target->fn_80167180(1);

    m_sound = fn_801D3E58(0x714A, &m_charInfo->m_position, this);
}

CityBombBig::~CityBombBig() {
    m_sound.fn_801D455C();
}

extern "C" void fn_802F37C0(void) {
    fn_8006B474("CITYBOMBBIG.BIN", 1);
}

extern "C" void fn_802F37BC(void) {}

void CityBombBig::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CityBombBig(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8055A070 = {
    "CITY_BOMB_BIG", fn_802F37C0, fn_802F37BC, CityBombBig::Create, 0, 0, 0xFA0, 1, 0, 10, 0,
};

void CityBombBig::UpdateBody(float dt) {
    switch (m_state) {
    case STATE_ARMED:
        if (m_stateFlags.Test(0)) {
            m_target->fn_80166E84(1);
            m_sound.fn_801D455C();
            m_target->fn_80167180(2);
            fn_801D54F4(fn_80053164(), 0x7131, &m_charInfo->m_position, 0, 1);
            fn_801D54F4(fn_80053164(), 0x7132, &m_charInfo->m_position, 0, 1);
            m_state = STATE_COUNTDOWN;
            m_stateFlags.Reset(0);
        }
        break;

    case STATE_COUNTDOWN:
        m_countdown -= dt;
        if (m_countdown < 0.0f || fn_8000A950(m_countdown)) {
            m_model->fn_80162790();
            m_model->fn_80161880();
            m_target->fn_80166F88();

            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
            UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1000);
            parameters.field_0x44 =
                static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->name));
            parameters.field_0x40 = m_charInfo->m_position.y;
            parameters.field_0x34 = 1.0f;
            parameters.field_0x4A = 0;
            parameters.field_0x54 = 1;
            fn_8015D93C(&parameters);

            fn_801DD0BC(&m_charInfo->m_position, 2, 7, 25.0f);
            fn_801D54F4(fn_80053164(), 0x714B, &m_charInfo->m_position, 0, 1);

            {
                boost::shared_ptr<Mission::Mission> mission =
                    fn_800740E4(fn_8006D6A8(), lbl_8057D6A0.field_0x125 ? 1 : 0).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            }

            fn_8016907C(fn_800CB69C(), m_comId, 1);
            fn_8000D664(fn_80009548(), 0, 2.2f, 3.0f, 15.0f);
            m_object2.reset();
            fn_800C9B04(m_owner);
            m_flags |= 1;
        }
        break;
    }

    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void CityBombBig::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void CityBombBig::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
    m_health = 10.0f;
    m_countdown = 1.0f;
    m_state = STATE_ARMED;
    m_stateFlags.Clear();
}

void CityBombBig::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), m_charInfo->m_position);

    if (m_object2.get() != 0) {
        Fn8040Vector3 above(0.0f, s_objectHeight.value, 0.0f);

        const Fn8040Vector3 &position = above + m_charInfo->m_position;
        fn_802B44C8(m_object2.get(), &position);
    }
}

void CityBombBig::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x1A:
        if (m_state != STATE_COUNTDOWN) {
            m_state = STATE_IDLE;
        }
        break;
    case 0x1B:
        if (m_state == STATE_IDLE) {
            m_state = STATE_ARMED;
        }
        break;
    }
    command.m_flags |= 2;
}

void CityBombBig::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_state == STATE_ARMED) {
        param.field_0x54 = 1;
        m_health -= param.field_0x50;
        param.field_0x40 = param.field_0x24;
        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;
            if (!m_stateFlags.Test(0)) {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);
                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x36);
                }
                m_stateFlags.Set(0);
            }
        }
    }
}
