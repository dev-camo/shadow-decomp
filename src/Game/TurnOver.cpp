/* The TANK and TURNOVER gadgets (class described in Game/TurnOver.h). File name inferred from the
 * namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The character info's command handler and the probe's two callbacks are boost::function objects
 * built with boost::bind(&TurnOverTask::method, this, _1); their manager, invoker and copy
 * functions come from the boost headers. */
#include <cstdio>
#include "Game/TurnOver.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/Target.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The value that UpdateLineTarget stores in the line target's float at 0x20 (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original initialiser
 * expression. The original sets the value with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function; as in Fire.cpp, the const qualifier lets UpdateLineTarget load the
 * value after the out-of-line operator-> call, as the original does (USA (GUPE8P) lfs at
 * 0x80126224 after the call at 0x80126220). Revisit when the original
 * initialiser is found. */
static const float s_lineTargetField20 = UnkStartupFloat_800E2570(20.0f).value;

namespace TurnOver {

TurnOverTask::TurnOverTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner, int modelArg,
                           const char *fileName)
    : Gadget(parent, owner) {
    m_charInfo = fn_8015CB88(0x25, 4, "TurnOver");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&TurnOverTask::OnCommand, this, _1));
    ReadOwner(owner);

    m_resource =
        static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), modelArg, fileName));
    m_modelIndex = m_params.model;
    m_model = fn_8015F800(modelArg, fileName);
    m_model->fn_80162A68(m_modelIndex, 0);
    m_model->fn_80160C28(m_modelIndex);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo), m_modelIndex);
    CreateProbe();

    if (m_handle.get() != 0) {
        m_handle->m_probe->UnkSlot2(-1);
    }

    m_lineTarget = fn_801E29D8(boost::weak_ptr<CharInfo>(m_charInfo), 0x40);

    /* ResetFromOwner repeats the model calls above and this search after it changes the model
     * entry; the original has the code in place in both functions, and an inline helper for the
     * search stays out of line here (its body calls inline code: the search's constructor and the
     * smart pointer's operator->). */
    UnkFrameSearch_80169E8C search;

    /* The original clears the frame before each search, also before the first one, after the
     * search's constructor has already cleared it (USA (GUPE8P) stores at 0x80126FAC and
     * 0x80126FC4). */
    search.id = 1000;
    search.frame = 0;
    m_model->fn_8015FA78(fn_80169E8C, &search);
    m_frame1000 = search.frame;
    search.id = 1001;
    search.frame = 0;
    m_model->fn_8015FA78(fn_80169E8C, &search);
    m_frame1001 = search.frame;

    UpdateLineTarget();
    UpdatePositions();
}

TurnOverTask::~TurnOverTask() {
    ReleaseProbe();
}

void TurnOverTask::UpdateBody(float dt) {
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }

    switch (m_state) {
    case STATE_RESET:
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        if (m_target.get() != 0) {
            m_target->fn_80167010(4);
        }

        m_state = STATE_IDLE;
        break;
    case STATE_LIFTING:
        if (!m_soundFlags.Test(1)) {
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                PlaySound(0);
                m_soundFlags.Set(1);
            }
        }
        break;
    case STATE_CANCELLING:
        if (m_model->fn_80160040(m_modelIndex)) {
            m_state = STATE_IDLE;

            if (m_target.get() != 0) {
                m_target->fn_80167448(1);
            }

            fn_80169B80(5, &m_charInfo->m_position);
            PlaySound(1);
        }
        break;
    case STATE_TURNING:
        if (m_motion.expired()) {
            m_state = STATE_WAITING;

            if (m_handle.get() != 0) {
                m_handle->m_probe->UnkSlot2(-1);
            }

            fn_80169AEC(8, 2.0f, &m_charInfo->m_position);
        } else {
            boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock();

            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1) != 0) {
                m_state = STATE_WAITING;

                if (m_handle.get() != 0) {
                    m_handle->m_probe->UnkSlot2(-1);
                }
            }

            /* fn_80415A18 copies the data of the motion's events 0x5001 and 0x5000 into these
             * buffers; their first words are a sound id and the selector passed to the target's
             * fn_80167448. The two-word size is inferred from the 8-byte stack slots that the
             * original gives each buffer. */
            u32 sound[2];

            if (fn_80415A18(motion.get(), 0, 0x5001, sound, 0) != 0) {
                fn_801D54F4(fn_80053164(), sound[0], &m_charInfo->m_position, 0, 1);
            }

            s32 selector[2];

            if (fn_80415A18(motion.get(), 0, 0x5000, selector, 0) != 0) {
                if (m_target.get() != 0) {
                    m_target->fn_80167448(selector[0]);
                }
            }
        }
        break;
    case STATE_WAITING:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_timer = 1.5f;
            m_model->fn_8016268C(-1.0f);
            m_state = STATE_FINISHING;
        }
        break;
    case STATE_FINISHING:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            fn_8015A794(this);
            m_flags |= 1;
        }
        break;
    }

    UpdatePositions();
}

void TurnOverTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousModel = m_params.model;

    ReadOwner(owner);

    if (previousModel != m_params.model) {
        m_target.reset();
        ReleaseProbe();
        m_model->fn_80161880();
        m_model->fn_801604FC();
        m_model->fn_80162804();

        if (!m_model->fn_80162EA8(m_params.model)) {
            return;
        }

        m_modelIndex = m_params.model;
        m_model->fn_80162A68(m_modelIndex, 0);
        m_model->fn_80160C28(m_modelIndex);
        m_model->fn_8015FF60();
        m_model->fn_80161BD8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo), m_modelIndex);

        UnkFrameSearch_80169E8C search;

        search.id = 1000;
        search.frame = 0;
        m_model->fn_8015FA78(fn_80169E8C, &search);
        m_frame1000 = search.frame;
        search.id = 1001;
        search.frame = 0;
        m_model->fn_8015FA78(fn_80169E8C, &search);
        m_frame1001 = search.frame;
    }

    /* Queues a debug cross at each end of the line target's segment and a debug line between
     * them. The ends are computed as in UpdatePositions; the original has that code in place in
     * both functions, and an inline helper for it stays out of line here (it uses the vector's
     * inline constructor). */
    RwMatrix *startMatrix = RwFrameGetLTM(m_frame1000);
    Fn8040Vector3 start(startMatrix->pos.x, startMatrix->pos.y, startMatrix->pos.z);
    RwMatrix *endMatrix = RwFrameGetLTM(m_frame1001);
    Fn8040Vector3 end(endMatrix->pos.x, endMatrix->pos.y, endMatrix->pos.z);

    start.y += 1.0f;
    end.y += 1.0f;
    fn_8015F5D8(&start, 10.0f);
    fn_8015F5D8(&end, 10.0f);
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);

    UpdatePositions();
    UpdateLineTarget();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }
}

void TurnOverTask::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_RESET;
}

void TurnOverTask::UpdatePositions() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);

    RwMatrix *startMatrix = RwFrameGetLTM(m_frame1000);
    Fn8040Vector3 start(startMatrix->pos.x, startMatrix->pos.y, startMatrix->pos.z);
    RwMatrix *endMatrix = RwFrameGetLTM(m_frame1001);
    Fn8040Vector3 end(endMatrix->pos.x, endMatrix->pos.y, endMatrix->pos.z);

    start.y += 1.0f;
    end.y += 1.0f;

    /* Matching workaround: the segment is a named local, so its construction (USA (GUPE8P)
     * 0x8012634C) comes before the load of m_lineTarget (0x80126350), as in the original; passed
     * as a temporary, the line target is read first and 10 words differ. Revisit if another form
     * gives the original's order. */
    UnkSegment_80020A20 segment(start, end);

    fn_801E2C9C(m_lineTarget.get(), segment);
}

void TurnOverTask::UpdateLineTarget() {
    if (m_lineTarget.get() != 0) {
        m_lineTarget->SetField20(s_lineTargetField20);

        Fn8040Vector3 axis(0.0f, 0.0f, 1.0f);
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);

        axis = matrix * axis;
        fn_801E73B0(m_lineTarget.get(), &axis);
    }
}

void TurnOverTask::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 14:
        if (m_state == STATE_IDLE) {
            m_state = STATE_LIFTING;
            m_timer = 0.2f;
            m_soundFlags.Reset(1);
            m_model->fn_80160A84(m_modelIndex, 1);
            UpdatePositions();

            if (m_target.get() != 0) {
                m_target->fn_80167448(1);
            }
        }
        break;
    case 13:
        if (m_state == STATE_LIFTING) {
            m_user = static_cast<UseCommand &>(command).m_user;
            m_state = STATE_TURNING;
            m_timer = 2.0f;
            m_model->fn_80160A84(m_modelIndex, 2);
            UpdatePositions();
            m_motion = m_model->fn_801602B4(m_modelIndex);

            if (m_target.get() != 0) {
                m_target->fn_80167448(2);
            }

            if (m_handle.get() != 0) {
                m_handle->m_probe->UnkSlot1(-1);
            }
        }
        break;
    case 15:
        if (m_state == STATE_LIFTING) {
            m_user.reset();
            m_state = STATE_CANCELLING;
            m_model->fn_80160A84(m_modelIndex, 3);
            UpdatePositions();
        }
        break;
    case 28:
        if (boost::shared_ptr<CharInfo> user = m_user.lock()) {
            ScoreCommand &score = static_cast<ScoreCommand &>(command);

            user->fn_801DC980(new ScoreCommand(score.field_0x14, score.field_0x18, score.field_0x1C,
                                               score.field_0x20, score.field_0x24));
        }
        break;
    }
}

void TurnOverTask::fn_80125F1C(AttackCallbackParam &param) {
    boost::shared_ptr<CharaColliAttack> attack = param.field_0x8.lock();

    if (attack.get() != 0) {
        boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock();

        if (attacker.get() != 0 && attacker->field_0x5C == 1) {
            param.field_0x30 = 1;
            /* Matching workaround: the explicit return gives this path its own copy of the two
             * smart pointers' destructor calls and a branch to the epilogue, as in the original
             * (USA (GUPE8P) 0x80125FAC, 0x80125FB8 and 0x80125FBC); without it the path falls
             * through to the shared destructor calls (0x80125FC8, 0x80125FD4) and the function is
             * 45 words long instead of 52. Revisit if a form without the early return matches. */
            return;
        }
    }
}

void TurnOverTask::fn_80125F18(AttackCallbackParam &param) {}

void TurnOverTask::CreateProbe() {
    void *collision = fn_801616CC(m_model.get(), m_modelIndex);
    boost::weak_ptr<_stMtnPlayWork> motionWeak = m_model->fn_801602B4(m_modelIndex);

    if (collision != 0) {
        if (boost::shared_ptr<_stMtnPlayWork> motion = motionWeak.lock()) {
            boost::shared_ptr<CharaColliBase> probe =
                fn_800C7F7C(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), collision,
                            motion.get(), 3, 0);
            boost::weak_ptr<CharaColliAttack> attackWeak = fn_800C4374(fn_80021B08(), probe);

            if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
                attack->fn_80054DD0(
                    3, 0xBC0, AttackCallback(boost::bind(&TurnOverTask::fn_80125F1C, this, _1)),
                    AttackCallback(boost::bind(&TurnOverTask::fn_80125F18, this, _1)), 999.0f);
            }

            m_handle = fn_8015B8C4(probe);
        }
    }
}

void TurnOverTask::ReleaseProbe() {
    m_handle.reset();
}

void TurnOverTask::PlaySound(int sound) {
    switch (sound) {
    case 0: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        sprintf(name, "%s_liftup", modelName);

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            fn_801D560C(fn_80053164(), record->field_0x8, 0, 0);
        }
        break;
    }
    case 1: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        sprintf(name, "%s_cancel", modelName);

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            fn_801D560C(fn_80053164(), record->field_0x8, 0, 0);
        }
        break;
    }
    }
}

} // namespace TurnOver

extern "C" void fn_80125620(void) {
    fn_8006B474("TURNOVER.BIN", 1);
}

extern "C" void fn_8012561C(void) {}

void TurnOver::TurnOverTask::CreateTurnOver(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new TurnOverTask(parent, owner, 1, "TURNOVER.BIN");
}

extern "C" void fn_8012558C(void) {
    fn_8006B474("TANK.BIN", 1);
}

extern "C" void fn_80125588(void) {}

void TurnOver::TurnOverTask::CreateTank(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new TurnOverTask(parent, owner, 0, "TANK.BIN");
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "blast", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80527E0C = {
    "TURNOVER", fn_80125620, fn_8012561C, TurnOver::TurnOverTask::CreateTurnOver, 0, 0, 0x23, 1,
    0,          10,          s_paramInfo,
};

GadgetTypeInfo lbl_80527E30 = {
    "TANK", fn_8012558C, fn_80125588, TurnOver::TurnOverTask::CreateTank, 0, 0, 0x21, 1,
    0,      10,          s_paramInfo,
};
