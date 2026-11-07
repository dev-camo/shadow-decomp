/* The Monster gadget (class and behaviour described in Game/Monster.h) and the methods of its
 * MonsterCharInfo. File name inferred from the class. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). The lengths in metres below assume that
 * fn_8005AE20 converts metres to the game's units (inferred).
 *
 * NonMatching: the constructor differs in 2 words, the order of the two argument moves of the
 * assignment call that copies the path's start position. */
#include "Game/Monster.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MonsterControl.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8005AE20.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::PathCtrl;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The half extents of the box probe (fn_800C7C34's halfExtents), 6 by 6 by 0.5 metres. */
static Fn8040Vector3 s_probeHalfExtents(fn_8005AE20(6.0f), fn_8005AE20(6.0f), fn_8005AE20(0.5f));
/* The vector the constructor passes as fn_800C7C34's arg6, where ElecCircuitColli passes the zero
 * vector lbl_805E07A0 (not understood). */
static Fn8040Vector3 s_probeVector(0.0f, fn_8005AE20(-5.0f), fn_8005AE20(-6.0f));
/* The absolute z of s_probeVector (60), which UpdateBody subtracts from the character's
 * distance. */
static const float s_probeVectorDepth = fn_8000A980(s_probeVector.z);
/* The initial m_leadDistance (15 metres) and the amount that a MISS takes off it (7.5 metres). */
static const float s_leadDistance = fn_8005AE20(15.0f);
static const float s_missDistance = fn_8005AE20(7.5f);
/* For a movement speed v, UpdateBody requests the motion frame speed v / (60 * (this / 80)); this
 * is 50 metres a second. */
static const float s_motionSpeedBase = fn_8005AE20(50.0f);

/* The sound that START, MISS, END and a finishing attack play. */
static const u16 s_actionSound = 0x744F;

const char *Monster::s_name = "Monster";

MonsterCharInfo::~MonsterCharInfo() {}

void MonsterCharInfo::fn_80139F98(int kind, MonsterControl *control) {
    m_monster->OnControl(kind);
    control->DisableCollision();
}

Monster::Monster(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0), m_leadDistance(s_leadDistance), m_started(0) {
    m_name = s_name;

    m_path = boost::shared_ptr<PathCtrl>(new PathCtrl);
    fn_8001C7B4(fn_8001B494(), m_path.get(), 0x22);
    fn_80018304(m_path.get());

    m_charInfo = boost::shared_ptr<MonsterCharInfo>(new MonsterCharInfo(this));
    m_charInfo->SetField0x5C(4);
    m_charInfo->field_0x30 = 1;

    /* The constructor's 2 differing words are in this copy (see the top of the file). */
    PathCtrl *path = m_path.get();
    MonsterCharInfo *info = m_charInfo.get();
    const Fn8040Vector3 &start = fn_80018634(path);

    info->m_position = start;
    m_charInfo->m_rotation = fn_80018434(m_path.get(), 1, 0);
    fn_8040CE60(&m_charInfo->field_0x18);
    fn_801DCD8C(fn_800DDF20(), m_charInfo);

    m_model = fn_8015F800(1, "MONSTER.BIN");
    m_model->fn_80162A68(0, 0);

    m_probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), s_probeHalfExtents,
                          0x10120, 6, 0, &s_probeVector, &lbl_805E07B8, 0);

    {
        boost::shared_ptr<CharaColliAttack> attack = fn_800C4374(fn_80021B08(), m_probe).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&Monster::OnAttack, this, _1)));
            attack->field_0x55 = 0;
        }

        m_probe->UnkSlot2(-1);
    }

    ReadOwner(owner);

    m_target = fn_80166AF4();

    UnkWeakHolder_800CC6C8 weakTarget;
    weakTarget.ptr = m_target;
    m_model->fn_8015FA78(fn_80169D00, &weakTarget);
}

Monster::~Monster() {
    fn_801DCCF4(fn_800DDF20(), m_charInfo);
}

extern "C" void fn_801392A0(void) {
    fn_8006B474("MONSTER.BIN", 1);
}

extern "C" void fn_8013929C(void) {}

void Monster::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Monster(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void Monster::UpdateBody(float dt) {
    if (m_path.get() == 0) {
        return;
    }

    if (!fn_800ABDCC(m_path.get())) {
        return;
    }

    switch (m_state) {
    case 1:
        if (m_started) {
            /* Flag 3 of the motion's player 0 (MOTION_PLAYER_FLAG_400000, read only); taken
             * here as the end of the start motion (inferred). */
            if (stMtnControlWorkFlag(m_model->fn_801602B4(0).lock().get(), 0, 3, 1)) {
                m_probe->UnkSlot1(-1);

                boost::shared_ptr<CharaColliAttack> attack =
                    fn_800C42D0(fn_80021B08(), m_probe).lock();

                if (attack.get() != 0) {
                    attack->field_0x54 = 1;
                    attack->field_0x55 = 1;
                }

                m_model->fn_80160A84(0, 2);
                m_state = 2;
            }
        } else {
            m_started = 1;
        }
        break;

    case 2: {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040BCD4(&matrix);

        boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

        if (character.get() != 0) {
            Fn8040Vector3 local = matrix * character->m_position;
            Fn8040Vector3 position;

            /* The character's distance along the monster's negative z axis, beyond the depth
             * of s_probeVector (the position is taken into the model's space by the inverted
             * matrix). */
            float distance = -local.z - s_probeVectorDepth;
            /* The step of the last update (the length of the movement in field_0x18) moves
             * towards the step of the minimum or maximum speed by at most the deceleration or
             * acceleration over dt. */
            float lastStep = fn_8040CF50(&m_charInfo->field_0x18);
            float targetStep;

            if (distance < m_leadDistance) {
                targetStep = m_params.minSpeed * dt;
            } else {
                targetStep = m_params.maxSpeed * dt;
            }

            float step;

            if (lastStep > targetStep) {
                float maxChange = dt * (m_params.decel * dt);

                if (lastStep - targetStep > maxChange) {
                    step = lastStep - maxChange;
                } else {
                    step = targetStep;
                }
            } else {
                float maxChange = dt * (m_params.accel * dt);

                if (targetStep - lastStep > maxChange) {
                    step = lastStep + maxChange;
                } else {
                    step = targetStep;
                }
            }

            fn_8001C91C(fn_8001B494(), m_path.get(), 0, &position, step, &m_charInfo->m_rotation,
                        1);
            m_charInfo->field_0x18 = position - m_charInfo->m_position;
            m_charInfo->m_position = position;

            boost::shared_ptr<_stMtnPlayWork> control = m_model->fn_801602B4(0).lock();

            if (control.get() != 0) {
                float speed;

                if (fn_8000A950(dt)) {
                    speed = 0.0f;
                } else {
                    speed = step / dt;
                }

                stMtnRequestFrameSpeed(control.get(), 0,
                                       speed / (60.0f * (s_motionSpeedBase / 80.0f)));
            }
        }

        UpdateModelMatrix();
        break;
    }
    }

    m_model->fn_80162FB0(dt);

    if (m_state == 2) {
        boost::shared_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(0).lock();

        if (motion.get() != 0) {
            _stMtnPlayWork *control = motion.get();
            unsigned short eventId;
            void *payload;

            while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                if (eventId == 0x5000) {
                    m_target->fn_80167448(*static_cast<int *>(payload));
                } else if (eventId == 0x5001) {
                    u32 *sound = static_cast<u32 *>(payload);
                    fn_801D54F4(fn_80053164(), static_cast<u16>(*sound), &m_charInfo->m_position, 0,
                                1);
                }
                /* A null control continues with the same player's next event. */
                control = 0;
            }
        }
    }

    m_target->fn_8016761C(dt);
}

void Monster::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    m_model->fn_80162EF4();
    m_probe->UnkSlot26();
    ReadOwner(owner);
}

bool Monster::IsFinished() {
    if (m_owner != 0) {
        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
            return true;
        }

        if (m_state == 0) {
            if (fn_800C996C(m_owner)) {
                m_flags |= 1;
                return true;
            }
        } else {
            boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

            if (character.get() != 0) {
                Fn8040Vector3 distance = character->m_position - m_charInfo->m_position;
                if (fn_8040CF50(&distance) > fn_800C9938(m_owner)) {
                    m_flags |= 1;
                    return true;
                }
            }
        }
    }

    return false;
}

void Monster::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.minSpeed = fn_8005AE20(m_params.minSpeed);
    m_params.maxSpeed = fn_8005AE20(m_params.maxSpeed);
    m_params.accel = fn_8005AE20(m_params.accel);
    m_params.decel = fn_8005AE20(m_params.decel);
}

void Monster::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
}

void Monster::OnControl(int kind) {
    switch (kind) {
    case 0: /* START */
        if (m_state == 0) {
            m_model->fn_80160C28(0);
            m_model->fn_80160A84(0, 1);
            m_model->fn_8016271C();
            m_target->fn_80167010(1);
            UpdateModelMatrix();
            fn_801D54F4(fn_80053164(), s_actionSound, &m_charInfo->m_position, 0, 1);
            m_started = 0;
            m_state = 1;
        }
        break;

    case 1: /* MISS */
        if (m_state == 2) {
            m_leadDistance -= s_missDistance;
            fn_801D54F4(fn_80053164(), s_actionSound, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 2: /* END */
        m_target->fn_80166F88();
        fn_801D54F4(fn_80053164(), s_actionSound, &m_charInfo->m_position, 0, 1);
        m_flags |= 1;
        break;
    }
}

void Monster::OnAttack(AttackCallbackParam &param) {
    if (fn_8015D4E4(&param)) {
        boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock();

        if (attack.get() != 0 && (attack->field_0x40 & 0x10)) {
            param.field_0x54 = 3;

            /* The attacker's pointer is released before the sound plays. */
            {
                boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock();
                fn_8020642C(attacker, 0x30);
            }

            fn_801D54F4(fn_80053164(), s_actionSound, &m_charInfo->m_position, 0, 1);
            m_flags |= 1;
            return;
        }
    }

    param.field_0x54 = 5;
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "MIN_SPPED m/sec", 0, 0, 0, 30.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MAX_SPPED m/sec", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ACCEL m/sec^2", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "DECEL m/sec^2", 0, 0, 0, 150.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528DEC = {
    "Monster", fn_801392A0, fn_8013929C, Monster::Create, 0, 0, 0xBC7, 1, 0, 10, s_paramInfo,
};
