/* The PARK_BALLOON gadget: ShootingGame::BalloonGenerator and the ShootingGame::Balloon objects
 * it creates (class names from RTTI), with the RandomSampling instances that choose their kinds
 * and angles. File name inferred from the classes and the gadget's resource file. Method names
 * marked "(name inferred)" in the header are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first).
 *
 * The generator's command handler and the balloon's attack callback are boost::function objects
 * built with boost::bind(&BalloonGenerator::OnCommand, this, _1) and
 * boost::bind(&Balloon::OnAttack, this, _1); their manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/ShootingGameBalloon.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/ShootingGameManagerTask.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include <cstdio>

using ShootingGame::Balloon;
using ShootingGame::BalloonGenerator;
using ShootingGame::ManagerTask;
using ShootingGame::RandomSampling;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
namespace Kind = ShootingGame::Kind;

/* The effect that a burst balloon starts (the id passed to fn_80168C48), by kind. */
static int s_effects[4] = {25, 31, 19, 14};

const char *Balloon::s_name = "ShootingGame::Balloon";
const char *BalloonGenerator::s_name = "ShootingGame::BalloonGenerator";

Balloon::Balloon(Task *parent, const UnkBalloonParam_8013B2C0 &param) : Task(parent) {
    m_name = s_name;
    m_param = param;
    m_charInfo = fn_8015CB88(0x47, 4, "ParkBalloon");
    m_charInfo->m_position = m_param.m_position;
    m_model = fn_8015F800(1, "SHOOTINGGAMEBALLOON.BIN");
    m_model->fn_80162A68(m_param.m_kind, 0);
    m_model->fn_80161FC8(m_param.m_kind, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(m_param.m_kind);

    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Balloon::OnAttack, this, _1));
    }

    m_target = fn_801E72DC(0xC00);
    m_state = 0;
    UpdateModelMatrix();
}

Balloon::~Balloon() {
    fn_801E7108(m_target);
}

void Balloon::Update(float dt) {
    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        m_charInfo->m_position.y += 30.0f * m_param.m_speed * dt;

        if (m_charInfo->m_position.y - m_param.m_position.y > m_param.m_height) {
            m_state = 3;
        }
        break;

    case 2: {
        int id = m_param.m_id;

        switch (m_param.m_kind) {
        case Kind::B:
            ManagerTask::s_table->AddCount(id, 1);
            break;
        case Kind::E:
            ManagerTask::s_table->AddCount(id, -2);
            break;
        case Kind::N:
            ManagerTask::s_table->AddCount(id, 2);
            break;
        case Kind::G:
            break;
        }

        if (!m_param.m_generator.expired()) {
            boost::shared_ptr<GadgetCharInfo> generator = m_param.m_generator.lock();

            generator->fn_801DC980(new DeadCommand(4));
        }

        Fn8040Vector3 position = m_charInfo->m_position;

        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
        fn_80168C48(&position, &up, s_effects[m_param.m_kind]);

        if (m_param.m_kind == Kind::B || m_param.m_kind == Kind::N) {
            fn_801D54F4(fn_80053164(), 0x7433, &m_charInfo->m_position, 0, 1);
        } else if (m_param.m_kind == Kind::E) {
            fn_801D54F4(fn_80053164(), 0x7433, &m_charInfo->m_position, 0, 1);
            fn_801D54F4(fn_80053164(), 0xE005, &m_charInfo->m_position, 0, 1);
        } else {
            fn_801D54F4(fn_80053164(), 0x7434, &m_charInfo->m_position, 0, 1);
            fn_801D54F4(fn_80053164(), 0x7435, &m_charInfo->m_position, 0, 1);
        }

        m_state = 3;
        break;
    }

    case 3:
        m_flags |= 1;
        break;
    }

    UpdateModelMatrix();
}

void Balloon::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8015CEA0(m_charInfo.get(), &matrix);
    m_model->fn_801633DC(&matrix);

    if (m_target.get() != 0) {
        Fn8040Vector3 position = m_charInfo->m_position;
        position.y += 10.0f;
        fn_800CCE38(m_target.get(), position);
    }
}

void Balloon::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 2) {
                param.field_0x54 = 5;
                return;
            }
        }

        param.field_0x54 = 3;
        m_state = 2;
    } else {
        param.field_0x54 = 5;
    }
}

void Balloon::Create(const UnkBalloonParam_8013B2C0 &param) {
    new Balloon(fn_801EB304(fn_8001C4F8(), 11), param);
}

BalloonGenerator::BalloonGenerator(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x48, 4, "ParkBalloonGen");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&BalloonGenerator::OnCommand, this, _1));
    ReadOwner(owner);
}

BalloonGenerator::~BalloonGenerator() {}

void BalloonGenerator::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (m_params.id > 0) {
            m_state = 1;
        } else {
            m_state = 2;
        }
        break;

    case 1:
        if (ManagerTask::s_table->IsState(m_params.id, kShootingGameStarted)) {
            m_state = 2;
        }
        break;

    case 2:
        m_timer -= dt;

        if (m_timer < 0.0f) {
            m_timer = m_params.sec;
            Generate(m_params.num);
        }

        if (m_params.id > 0) {
            if (ManagerTask::s_table->IsState(m_params.id, kShootingGameFinished)) {
                m_state = 3;
            }
        } else if (m_burstCount >= m_params.maxNum) {
            m_state = 3;
        }
        break;

    case 3:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void BalloonGenerator::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);

    UnkRotation3 rotation(0.0f, 0.0f, 0.0f);
    rotation.x = 90.0f * (lbl_805FAD38 / 180.0f);

    float halfHeight = 0.5f * m_params.height;

    Fn8040Vector3 center = m_charInfo->m_position;
    center.y += halfHeight;
    fn_802E0C60(&center, &rotation, 0, halfHeight, m_params.radius);
}

void BalloonGenerator::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
    m_timer = m_params.sec;
    m_burstCount = 0;
    m_roundCount = 0;
}

void BalloonGenerator::Generate(int count) {
    boost::shared_ptr<RandomSampling<Kind::__Kind> > kinds =
        boost::shared_ptr<RandomSampling<Kind::__Kind> >(new RandomSampling<Kind::__Kind>(count));
    boost::shared_ptr<RandomSampling<float> > angles =
        boost::shared_ptr<RandomSampling<float> >(new RandomSampling<float>(count));

    int countB = m_params.rate >> 24;
    int countN = (m_params.rate >> 16) & 0xFF;
    int countE = (m_params.rate >> 8) & 0xFF;
    int countG = m_params.rate & 0xFF;

    for (int i = 0; i < countB; i++) {
        kinds->Add(Kind::B);
    }
    for (int i = 0; i < countN; i++) {
        kinds->Add(Kind::N);
    }
    for (int i = 0; i < countE; i++) {
        kinds->Add(Kind::E);
    }
    for (int i = 0; i < countG; i++) {
        kinds->Add(Kind::G);
    }

    int angleCount = count + 6;
    int roundIndex = m_roundCount++;
    float step = 360.0f / angleCount * (lbl_805FAD38 / 180.0f);
    float offset = 13.0f * roundIndex * (lbl_805FAD38 / 180.0f);

    for (int i = 0; i < angleCount; i++) {
        angles->Add(step * i + offset);
    }
    angles->Shuffle();

    for (int i = 0; i < count; i++) {
        Fn8040Matrix34 matrix;
        Fn8040Vector3 position(0.0f, 0.0f, m_params.radius);

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, angles->Pop());
        position = matrix * position;

        UnkBalloonParam_8013B2C0 param;

        param.m_position = position;
        param.m_height = m_params.height;

        float speedRange = m_params.maxSpeed - m_params.minSpeed;
        param.m_speed = speedRange * fn_8040ECF8() + m_params.minSpeed;

        if (m_params.id > 0) {
            param.m_id = m_params.id;
        }

        param.m_kind = kinds->Pop();
        param.m_generator = m_charInfo;
        Balloon::Create(param);
    }
}

void BalloonGenerator::OnCommand(CharCommand &command) {
    if (command.m_type == 8) {
        m_burstCount++;
    }
}

extern "C" void fn_8013AEE4(void) {
    fn_8006B474("SHOOTINGGAMEBALLOON.BIN", 1);
    ManagerTask::Init();
}

extern "C" void fn_8013AEC4(void) {
    ManagerTask::Release();
}

void BalloonGenerator::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BalloonGenerator(parent, owner);
}

/* The labels of the values of the "Id" parameter. */
static const char *s_idLabels[2] = {"Id Off", "Id On"};

/* The label callback of the "Rate" and "Id" parameters, entries 6 and 8 of the table below
 * (behaviour inferred): the four counts of Rate, or whether an Id is set. */
static const char *fn_8013ADC4(UnkParamCursor_800CBC20 *cursor) {
    static char s_buffer[64];
    /* A function-local static pointer, initialised on the first call: the object has its .sbss
     * pointer and guard flag. */
    static const char *s_format = "B:N:E:G = %2d:%2d:%2d:%2d";

    if (cursor->m_index == 6) {
        u32 rate = cursor->m_value;

        /* Each count is one byte of rate (0 to 255), so %2d prints these unsigned values
         * exactly. */
        sprintf(s_buffer, s_format, rate >> 24, (rate >> 16) & 0xFF, (rate >> 8) & 0xFF,
                rate & 0xFF);
        return s_buffer;
    }

    if (cursor->m_index == 8) {
        if (cursor->m_value == 0) {
            return s_idLabels[0];
        }

        return s_idLabels[1];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "Height", 0, 0, 0, 10.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "Radius", 0, 0, 0, 10.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "MinSpeed", 0, 0, 0, 0.1f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {4, "MaxSpeed", 0, 0, 0, 1.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {1, "Num", 1, 1, 100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "MaxNum", 1, 1, 100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    /* The default counts B:N:E:G = 4:4:1:1. */
    {3, "Rate", 0x04040101, 0, 0x7FFFFFFF, 0.0f, 0.0f, 0.0f, fn_8013ADC4, 0},
    {4, "Sec", 0, 0, 0, 5.0f, 0.0f, 3600.0f, fn_800CBC20, 0},
    {1, "Id", 0, 0, 255, 0.0f, 0.0f, 0.0f, fn_8013ADC4, 0},
    {0},
};

GadgetTypeInfo lbl_805291B8 = {
    "PARK_BALLOON", fn_8013AEE4, fn_8013AEC4, BalloonGenerator::Create, 0, 0, 0xC82, 1, 0, 10,
    s_paramInfo,
};

/* RandomSampling's layout, checked at the end of the file: a check before its uses instantiates the
 * class earlier and changes where the compiler emits its weak members. */
CHECK_SIZEOF(ShootingGame::RandomSampling<float>, 0xC);
CHECK_SIZEOF(ShootingGame::RandomSampling<ShootingGame::Kind::__Kind>, 0xC);
