/* The CITY_BOMB_SMALL gadget and VacuumCityBombSmall, its model while a character pulls it in
 * (classes described in Game/CityBombSmall.h). File name inferred from the class. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The command handler and the attack callback are boost::function objects built with
 * boost::bind(&CityBombSmall::Method, this, _1); their managers, invokers and copy functions come
 * from the boost headers. */
#include "Game/CityBombSmall.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/CommonBomb.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MissionClearCollision.h"
#include "Game/Mission.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* Set at startup to lbl_805F72B0 / 10 (the expression VacuumContainer.cpp uses for its model
 * scale); nothing in this file reads it. */
static const float s_unusedValue = lbl_805F72B0 / 10.0f;
/* The height of the second object above the position it follows and the value passed to its
 * fn_802B4498 (names inferred from their uses).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the unknown way the original
 * initialises these floats at startup. The original sets them with a start-up function in .ctors
 * (0x802F6320; a constant float initialiser would be emitted as initialised .sdata data instead),
 * and the shared stand-in reproduces those two float stores. The bytes do not establish the
 * variables' original type. Revisit when the original initialiser is found. */
static UnkStartupFloat_800E2570 s_object2Height(16.0f);
static UnkStartupFloat_800E2570 s_object2Value(400.0f);

const char *CityBombSmall::s_name = "CityBombSmall";

/* Creates a VacuumCityBombSmall from command, model, object2 and target, keeps it in a
 * shared_ptr<VacuumObject> (when the allocation fails, the pointer is empty and the calls below
 * are still made through it), passes position and rotation to its virtual fn_802382E0 and
 * fn_80238268 and creates a VacuumObjectTask for it, which is constructed only when the task's own
 * allocation succeeds (behaviour inferred; the original name is unknown). The signature is
 * inferred: the bytes give six integer register arguments, but not the pointer or reference
 * spellings or the const qualifiers. */
static void fn_802F46E4(VacuumCommand *command, const Fn8040Vector3 *position,
                        const UnkRotation3 *rotation,
                        const boost::shared_ptr<GadgetModelInfo> &model,
                        const boost::shared_ptr<UnkObject_802B4254> &object2,
                        const boost::shared_ptr<UnkTarget_80166AF4> &target);

CityBombSmall::CityBombSmall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_range(0.0f), m_state(STATE_WAITING) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x55, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&CityBombSmall::OnCommand, this, _1));
    ReadOwner(owner);

    m_model = fn_8015F800(1, "CITYBOMBSMALL.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, m_charInfo);

    {
        boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(0).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&CityBombSmall::OnAttack, this, _1)));
        }
    }

    m_object = fn_801E7128(m_charInfo, 0xA480);
    if (lbl_8057D6A0.field_0xA8.field_0x4 == 12) {
        m_object2 = fn_802B4254(2, 0);
        fn_802B4498(m_object2.get(), s_object2Value.value);
        fn_802B4510(m_object2.get(), 1);
    }
    UpdateModelMatrix();

    m_target = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 weakTarget;

        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
    m_target->fn_80167010(1);
}

CityBombSmall::~CityBombSmall() {}

extern "C" void fn_802F5340(void) {
    fn_8006B474("CITYBOMBSMALL.BIN", 1);
}

extern "C" void fn_802F533C(void) {}

void CityBombSmall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CityBombSmall(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "Range (point)", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055A180 = {
    "CITY_BOMB_SMALL", fn_802F5340, fn_802F533C, CityBombSmall::Create, 0, 0, 0xFA1, 1, 0, 10,
    s_paramInfo,
};

void CityBombSmall::UpdateBody(float dt) {
    switch (m_state) {
    case STATE_WAITING:
        if (lbl_8057D6A0.field_0xA8.field_0x4 == 8) {
            UnkCharInfoNode_801DCB14 *node = fn_801DCB14(fn_800DDF20(), 0x91);

            if (node != 0) {
                boost::shared_ptr<CharInfo> charInfo = node->charInfo.lock();

                if (charInfo.get() != 0) {
                    Fn8040Vector3 offset = m_charInfo->m_position - charInfo->m_position;

                    if (fn_8040CF50(&offset) < m_range) {
                        m_stateFlags.Set(FLAG_TRIGGERED);
                    }
                }
            }
        }

        {
            boost::shared_ptr<CharInfo> nearest =
                fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

            if (nearest.get() != 0) {
                Fn8040Vector3 offset = m_charInfo->m_position - nearest->m_position;

                if (fn_8040CF50(&offset) < m_range) {
                    m_stateFlags.Set(FLAG_TRIGGERED);
                }
            }
        }

        if (m_stateFlags.Test(FLAG_TRIGGERED)) {
            m_target->fn_80167180(2);
            fn_801D54F4(fn_80053164(), 0x7131, &m_charInfo->m_position, 0, 1);
            m_state = STATE_COUNTDOWN;
            m_stateFlags.Reset(FLAG_TRIGGERED);
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

            fn_801DD0BC(&m_charInfo->m_position, 1, 7, 8.0f);
            fn_801D54F4(fn_80053164(), 0x714C, &m_charInfo->m_position, 0, 1);
            fn_8016907C(fn_800CB69C(), m_comId, 1);
            m_object2.reset();
            fn_800C9B04(m_owner);
            m_flags |= 1;
        }
        break;
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void CityBombSmall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void CityBombSmall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
    fn_800C9DBC(owner, &m_range, sizeof(m_range));
    m_health = 5.0f;
    m_countdown = 1.0f;
    m_state = STATE_WAITING;
    m_stateFlags.Clear();
}

void CityBombSmall::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    if (m_model.get() != 0) {
        m_model->fn_801633DC(&matrix);
    }
    if (m_object.get() != 0) {
        fn_800CCE38(m_object.get(), m_charInfo->m_position);
    }

    if (m_object2.get() != 0) {
        Fn8040Vector3 above(0.0f, s_object2Height.value, 0.0f);

        const Fn8040Vector3 &position = above + m_charInfo->m_position;
        fn_802B44C8(m_object2.get(), &position);
    }
}

void CityBombSmall::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14: /* VacuumCommand */
        if (m_state == STATE_WAITING && !m_stateFlags.Test(FLAG_TRIGGERED)) {
            fn_802F46E4(static_cast<VacuumCommand *>(&command), &m_charInfo->m_position,
                        &m_charInfo->m_rotation, m_model, m_object2, m_target);
            m_model.reset();
            m_target.reset();
            m_object2.reset();
            m_state = STATE_VACUUMED;

            {
                boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            }

            fn_8016907C(fn_800CB69C(), m_comId, 1);

            {
                boost::shared_ptr<CharInfo> charInfo = fn_800A8C54(fn_80074194(), 0).lock();

                if (charInfo.get() != 0) {
                    fn_8020642C(charInfo, 0x38);
                }
            }

            fn_800C9B04(m_owner);
            m_flags |= 1;
        }
        break;
    }
    command.m_flags |= 2;
}

void CityBombSmall::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_state == STATE_WAITING) {
        param.field_0x54 = 1;
        m_health -= param.field_0x50;
        param.field_0x40 = param.field_0x24;
        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;
            if (!m_stateFlags.Test(FLAG_TRIGGERED)) {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);
                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x37);
                }
                m_stateFlags.Set(FLAG_TRIGGERED);
            }
        }
    }
}

VacuumCityBombSmall::VacuumCityBombSmall(VacuumCommand *command,
                                         const boost::shared_ptr<GadgetModelInfo> &model,
                                         const boost::shared_ptr<UnkObject_802B4254> &object2,
                                         const boost::shared_ptr<UnkTarget_80166AF4> &target)
    : VacuumObject(fn_80237EDC(command), command), m_model(model), m_object2(object2),
      m_target(target) {}

VacuumCityBombSmall::~VacuumCityBombSmall() {}

void VacuumCityBombSmall::Update(float dt) {
    VacuumObject::Update(dt);

    Fn8040Vector3 position = fn_802382B0();
    UnkRotation3 rotation = fn_80238248();

    if (m_model.get() != 0) {
        /* Matching workaround: matrix is declared before translation. The original keeps
         * translation at the lower stack offset (0x44, matrix at 0x74), which this compiler gives
         * the later of two locals; declared the other way round, the offsets swap (7 words).
         * Revisit if translation turns out to be the result of a function of its own. */
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        fn_8040B034(&translation, &position);
        fn_8040C148(&matrix, &translation);
        fn_8040B848(&matrix, rotation.y);
        fn_8040B884(&matrix, rotation.x);
        fn_8040B80C(&matrix, rotation.z);
        m_model->fn_801633DC(&matrix);
        m_model->fn_80162FB0(dt);
    }

    if (m_object2.get() != 0) {
        Fn8040Vector3 above(0.0f, s_object2Height.value, 0.0f);

        const Fn8040Vector3 &abovePosition = position + above;
        fn_802B44C8(m_object2.get(), &abovePosition);
    }

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void VacuumCityBombSmall::fn_80238180() {}

static void fn_802F46E4(VacuumCommand *command, const Fn8040Vector3 *position,
                        const UnkRotation3 *rotation,
                        const boost::shared_ptr<GadgetModelInfo> &model,
                        const boost::shared_ptr<UnkObject_802B4254> &object2,
                        const boost::shared_ptr<UnkTarget_80166AF4> &target) {
    boost::shared_ptr<VacuumObject> vacuumObject =
        boost::shared_ptr<VacuumObject>(new VacuumCityBombSmall(command, model, object2, target));

    vacuumObject->fn_802382E0(position);
    vacuumObject->fn_80238268(rotation);
    new VacuumObjectTask(vacuumObject);
}
