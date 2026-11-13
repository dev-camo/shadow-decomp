/* The BK_TANK gadget (classes described in Game/BKTank.h): BKTank::SetBKTank and the functions
 * that create and destroy it together with its BKTankController::Controller. File name inferred
 * from the namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The attack object's callbacks are boost::function objects built with
 * boost::bind(&SetBKTank::..., this, _1); their manager, invoker and copy functions come from the
 * boost headers.
 *
 * 149.98999f is the original's health threshold constant (0x4315FD70), the float just below
 * 149.99f (0x4315FD71). */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/BKTank.h"
#include "Game/AngleConversion.h"
#include "Game/BKTankController.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/EffectManager.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Target.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The model file of SetBKTank; the descriptor's load hook passes the same string. */
static const char *s_fileName = "BKTANK.BIN";

/* 0.05 degrees in radians, set by the file's static initialiser; nothing in this file reads it
 * (name inferred). */
static float s_angle = DegreesToRadians(0.05f);

namespace BKTank {

SetBKTank::SetBKTank() : m_health(350.0f) {}

SetBKTank::~SetBKTank() {}

void SetBKTank::Initialize() {
    m_charInfo = fn_8015CB88(0x91, 4, "SetBKTank");
    fn_801DCD8C(fn_800DDF20(), m_charInfo);
    m_charInfo->field_0x30 = 1;

    m_model = fn_8015F800(1, s_fileName);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8016082C(0);
    m_model->fn_80160B18(0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliBase> colliRef = m_model->fn_80161614(0);

    if (!colliRef.expired()) {
        boost::shared_ptr<CharaColliBase> colli = colliRef.lock();

        colli->UnkSlot20(0x100000, -1);
    }

    boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161428(0);

    if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
        attack->fn_80054DD0(1, 1, AttackCallback(boost::bind(&SetBKTank::fn_8031F908, this, _1)),
                            AttackCallback(boost::bind(&SetBKTank::fn_8031F904, this, _1)), 1.0f);
    }

    boost::shared_ptr<CharaColliBase> probe = fn_800C7DEC(
        fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x100000, 5, 0, &lbl_805E07A0, 56.0f);

    m_handle = fn_8015B8C4(probe);
    m_handle->ClearFlags(2);

    attackRef = fn_800C4374(fn_80021B08(), m_handle->m_probe);

    if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
        attack->fn_80054DD0(0, 1, AttackCallback(boost::bind(&SetBKTank::fn_8031F908, this, _1)),
                            AttackCallback(boost::bind(&SetBKTank::fn_8031F904, this, _1)), 1.0f);
        attack->fn_80096B64(1, AttackCallback(boost::bind(&SetBKTank::OnAttack, this, _1)));
    }

    UpdateModelMatrix();
    m_previousPosition = m_charInfo->m_position;
    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x22C00);

    m_hitTimer = 0.0f;
    m_timer = 0.0f;
    field_0x28 = 0.0f;
    field_0xA8 = false;
    m_stationary = true;
    m_moving = false;
    field_0xAB = false;
    m_state = STATE_START;
    m_flags.Clear();
}

void SetBKTank::UnkSlot3() {
    fn_801DCCF4(fn_800DDF20(), m_charInfo);

    boost::shared_ptr<EffectElement> effect;

    for (int i = 0; i < 10; i++) {
        effect = m_effects[i].lock();
        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            effect.reset();
        }
    }
}

void SetBKTank::Update(float dt) {
    if (dt != 0.0f) {
        /* The copy of the difference is the argument (spelling inferred): the original calls
         * fn_8040CF50 with the pointer that the copy constructor returns (USA (GUPE8P) call at
         * 0x8031FA34). */
        m_speed = fn_8040CF50(Fn8040Vector3(m_previousPosition - m_charInfo->m_position)) / dt;
    }

    boost::shared_ptr<EffectElement> effect;

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (field_0x14.get() != 0) {
        field_0x14->fn_8016761C(dt);
    }

    m_hitTimer -= dt;
    if (m_hitTimer < 0.0f) {
        m_hitTimer = 0.0f;
    }

    m_timer -= dt;
    if (m_timer < 0.0f) {
        m_timer = 0.0f;
    }

    /* The constant-first comparisons here and in OnAttack give the original's code; written the
     * other way round, they compile to different instructions. */
    if (0.0f < field_0x28) {
        field_0x28 -= dt;
        if (field_0x28 < 0.0f) {
            field_0x28 = 0.0f;

            effect = m_effects[1].lock();
            if (effect.get() != 0) {
                effect->RequestStop();
                effect.reset();
            }
            effect = m_effects[2].lock();
            if (effect.get() != 0) {
                effect->RequestStop();
                effect.reset();
            }
            field_0xA8 = false;
        }
    }

    if (field_0x28 == 0.0f) {
        m_target->SetFlags(1);
        field_0x2C = 0;
    }

    effect = m_effects[0].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }
    effect = m_effects[9].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }
    effect = m_effects[7].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }
    effect = m_effects[1].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }
    effect = m_effects[2].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }
    effect = m_effects[6].lock();
    if (effect.get() != 0) {
        fn_80425E68(effect.get(), GetNodeMatrix());
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_HEALTH_HIGH;
        break;

    case STATE_HEALTH_HIGH:
        if (m_health <= 149.98999f) {
            m_state = STATE_HEALTH_MID;
        }
        break;

    case STATE_HEALTH_MID:
        if (m_health <= 50.0f) {
            SetStage(0);
            m_state = STATE_HEALTH_LOW;
        }
        break;

    case STATE_HEALTH_LOW:
        if (m_health <= 0.0f) {
            SetStage(1);
            m_timer = 1.5f;
            m_state = STATE_COUNTDOWN;
        }
        break;

    case STATE_COUNTDOWN:
        if (m_timer <= 0.0f) {
            m_state = STATE_FINISHING;
        }
        break;

    case STATE_FINISHING:
        m_flags.Set(FLAG_FINISHED);
        m_state = STATE_FINISHED;
        break;

    case STATE_FINISHED:
        break;
    }

    if (m_speed >= 1800.0f) {
        m_speed = 0.0f;
        field_0xAB = true;

        /* Declared, then assigned: an initialised declaration here or in the stationary branch
         * below makes Update two instructions shorter than the original. */
        boost::shared_ptr<EffectElement> stopped;

        stopped = m_effects[0].lock();
        if (stopped.get() != 0) {
            stopped->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            stopped.reset();
        }
        stopped = m_effects[7].lock();
        if (stopped.get() != 0) {
            stopped->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            stopped.reset();
        }
    } else if (field_0xAB) {
        field_0xAB = false;

        if (m_effects[0].expired()) {
            m_effects[0] = fn_80168A50(GetNodeMatrix(), 0x11);
        }
        m_effects[8] = fn_80168A50(GetNodeMatrix(), 0x70);
    }

    const Fn8040Vector3 &position = m_charInfo->m_position;
    bool stationary = m_previousPosition.x == position.x && m_previousPosition.y == position.y &&
                      m_previousPosition.z == position.z;

    if (stationary) {
        if (!m_stationary) {
            if (m_effects[9].expired()) {
                m_effects[9] = fn_80168A50(GetNodeMatrix(), 0x6F);
            }

            boost::shared_ptr<EffectElement> stopped;

            stopped = m_effects[0].lock();
            if (stopped.get() != 0) {
                stopped->ClearFlags(Effect::System::ELEMENT_ACTIVE);
                stopped.reset();
            }
        }
        fn_801D53C4(fn_80053164(), 0x714F, &m_charInfo->m_position, this, 3, 0, 5);
    } else {
        if (m_stationary) {
            if (m_effects[0].expired()) {
                m_effects[0] = fn_80168A50(GetNodeMatrix(), 0x11);
            }

            boost::shared_ptr<EffectElement> stopped = m_effects[9].lock();

            if (stopped.get() != 0) {
                stopped->ClearFlags(Effect::System::ELEMENT_ACTIVE);
                stopped.reset();
            }
            fn_801D54F4(fn_80053164(), 0x7160, &m_charInfo->m_position, 0, 1);
        }
        fn_801D53C4(fn_80053164(), 0x714D, &m_charInfo->m_position, this, 3, 0, 5);
    }
    m_stationary = stationary;

    bool moving = m_speed >= 180.0f;
    if (!moving && m_moving) {
        boost::shared_ptr<EffectElement> stopped = m_effects[7].lock();

        if (stopped.get() != 0) {
            stopped->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            stopped.reset();
        }
    }
    m_moving = moving;

    m_previousPosition = m_charInfo->m_position;
}

void SetBKTank::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

bool SetBKTank::IsFinished() {
    return m_flags.Test(FLAG_FINISHED);
}

void SetBKTank::UnkSlot7() {}

void SetBKTank::UnkSlot8() {}

void SetBKTank::fn_8031F908(AttackCallbackParam &param) {
    param.field_0x34 = Fn8040Vector3(0.0f, 1.0f, 0.0f);
    param.field_0x4C = 1.0f;
    param.field_0x50 = 1.0f;
}

void SetBKTank::fn_8031F904(AttackCallbackParam &param) {}

void SetBKTank::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_hitTimer > 0.0f) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3FA) == 0) {
            return;
        }

        if (attack->field_0x40 & 0xA) {
            if (++field_0x2C >= 3) {
                m_target->ClearFlags(1);
                return;
            }
        }

        float previousHealth = m_health;

        m_health -= param.field_0x50;
        m_hitTimer = 0.05f;
        field_0x28 = 2.0f;

        if (149.98999f < m_health) {
            m_effects[3] = fn_80168C48(&param.field_0x18, &param.field_0x24, 0x71);
            fn_800A31F0(); /* the effect manager is fetched and not used */
            fn_801D54F4(fn_80053164(), 0x7150, &m_charInfo->m_position, 0, 1);
        } else if (50.0f < m_health) {
            m_effects[4] = fn_80168C48(&param.field_0x18, &param.field_0x24, 0x64);
            fn_801D54F4(fn_80053164(), 0x7150, &m_charInfo->m_position, 0, 1);
        } else {
            m_effects[6] = fn_80168C48(&param.field_0x18, &param.field_0x24, 0x7F);
            fn_801D54F4(fn_80053164(), 0x7152, &m_charInfo->m_position, 0, 1);
        }

        if (!field_0xA8) {
            if (149.98999f < m_health) {
                field_0xA8 = true;
                m_effects[1] = fn_80168A50(GetNodeMatrix(), 0x12);
            } else if (50.0f < m_health) {
                field_0xA8 = true;
                m_effects[2] = fn_80168A50(GetNodeMatrix(), 0x63);
            }
        } else if (149.98999f < previousHealth && m_health <= 149.98999f) {
            boost::shared_ptr<EffectElement> effect = m_effects[1].lock();

            if (effect.get() != 0) {
                effect->RequestStop();
                effect.reset();
            }
            m_effects[2] = fn_80168A50(GetNodeMatrix(), 0x63);
        } else if (50.0f < previousHealth && m_health <= 50.0f) {
            boost::shared_ptr<EffectElement> effect = m_effects[2].lock();

            if (effect.get() != 0) {
                effect->RequestStop();
                effect.reset();
            }
            m_effects[5] = fn_80168A50(GetNodeMatrix(), 0x13);
            fn_801D54F4(fn_80053164(), 0x7151, &m_charInfo->m_position, 0, 1);
        }

        if (m_health > 0.0f) {
            param.field_0x54 = 1;
        } else {
            param.field_0x54 = 3;
        }
    }
}

void SetBKTank::StartCrashEffect() {
    if (m_model.get() != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 2.0f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 1;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }
    fn_801D54F4(fn_80053164(), 0x7153, &m_charInfo->m_position, 0, 1);
}

void SetBKTank::SetStage(int stage) {
    switch (stage) {
    case 0: {
        m_handle.reset();

        boost::weak_ptr<CharaColliBase> colliRef = m_model->fn_80161614(0);
        boost::shared_ptr<CharaColliBase> colli = colliRef.lock();

        if (colli.get() != 0) {
            boost::weak_ptr<CharaColliAttack> attackRef = fn_800C42D0(fn_80021B08(), colli);

            if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
                attack->fn_80096B64(1, AttackCallback(boost::bind(&SetBKTank::OnAttack, this, _1)));
            }
        }
        fn_8034DE08(8, &m_charInfo->m_position, -2);
        break;
    }

    case 1: {
        boost::shared_ptr<EffectElement> effect;

        StartCrashEffect();
        m_model->fn_801604FC();
        m_model->fn_80162804();

        effect = m_effects[7].lock();
        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            effect.reset();
        }
        effect = m_effects[9].lock();
        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            effect.reset();
        }
        effect = m_effects[0].lock();
        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
            effect.reset();
        }
        fn_8034DE08(8, &m_charInfo->m_position, -2);
        break;
    }
    }
}

void SetBKTank::OnCommand(CharCommand &command) {}

} // namespace BKTank

/* The controller of the current SetBKTank (name inferred).
 * Matching workaround: defined here rather than at the top of the file. The original's .sbss has
 * it (USA (GUPE8P) 0x805F0B24) after the three one-byte objects that the boost function code
 * emitted for Initialize uses (0x805F0B18-0x805F0B20) and before the one of the shared_ptr
 * constructor that fn_8031EBEC uses (0x805F0B28); defined at the top it comes before them. Revisit
 * if the original order of the file's definitions is found. */
static BKTankController::Controller *s_controller;

extern "C" void fn_8031EBEC(void) {
    boost::shared_ptr<PathObject> object = boost::shared_ptr<PathObject>(new BKTank::SetBKTank());

    Task *parent = fn_801EB304(fn_8001C4F8(), 9);

    s_controller = new BKTankController::Controller(parent, object);
}

extern "C" void fn_8031EBA4(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
}

extern "C" void fn_8031EBA0(void) {}

extern "C" void fn_8031EB54(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
    fn_8031EBEC();
}

namespace BKTank {

Fn8040Matrix34 SetBKTank::GetNodeMatrix() {
    boost::weak_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(0);

    if (!motion.expired()) {
        /* The locked pointer is released before the call, as in the original. */
        _stMtnPlayWork *work = motion.lock().get();
        MotionTransform *transform = fn_80415E70(work, 10);
        Fn8040Matrix34 matrix;

        fn_80016370(&matrix, &transform->matrix);
        return matrix;
    }
    return lbl_805E0720;
}

} // namespace BKTank

/* The descriptor's load hook: adds the model file to the list of resources to load (name
 * inferred). */
extern "C" void fn_8031EA58(void) {
    fn_8006B474("BKTANK.BIN", 1);
}

extern "C" void fn_8031EA54(void) {}

namespace BKTank {

boost::weak_ptr<CharInfo> SetBKTank::GetCharInfo() {
    return m_charInfo;
}

} // namespace BKTank

int fn_8031E9CC(void) {
    fn_8031EBEC();
    return 1;
}

int fn_8031E9A8(void) {
    fn_8031EBA0();
    return 1;
}

int fn_8031E984(void) {
    fn_8031EB54();
    return 1;
}

int fn_8031E960(void) {
    fn_8031EBA4();
    return 1;
}

GadgetTypeInfo lbl_8055C4F8 = {
    "BK_TANK", fn_8031EA58, fn_8031EA54, 0, 0, 0, 0x898, 1, 0x80, 10, 0,
};
