/* ShootingGame::Score (class name from RTTI; the gadget's descriptor is "SHOOTING_PANEL";
 * behaviour in the class comment). File name inferred from the class. Method names marked
 * "(name inferred)" in the header are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first).
 *
 * The attack object's callback is a boost::function object built with
 * boost::bind(&Score::OnAttack, this, _1); its manager, invoker and copy functions come from the
 * boost headers. */
#include "Game/ShootingGameScore.h"
#include "Game/CharaColliAttack.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/ShootingGameManagerTask.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *ShootingGame::Score::s_name = "ShootingGame::Score";

static const char *s_modelFile = "SHOOTINGGAMESCORE.BIN";

namespace ShootingGame {

Score::Score(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner), m_id(-1) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x49, 4, "Score");
    CopyFromOwner(owner);
    m_model = fn_8015F800(1, s_modelFile);

    if (ManagerTask::s_table != 0 && ManagerTask::s_table->IsState(m_id, kShootingGameStarted)) {
        if (!ManagerTask::s_table->TestFlag(m_id)) {
            SetupModel(1);
            m_model->fn_80160BA0("hit");
            m_model->fn_8015FF60();
            m_model->fn_801609F8("hit");

            boost::weak_ptr<_stMtnPlayWork> weakMotion = m_model->fn_801601A0("hit");
            if (boost::shared_ptr<_stMtnPlayWork> motion = weakMotion.lock()) {
                float frame = ManagerTask::s_table->GetCurrentFrame(m_id);

                stMtnRequestCurrentFrame(motion.get(), 0, frame);
            }
            m_state = 2;
        } else {
            SetupModel(2);
            m_model->fn_80160BA0("act");
            m_model->fn_8015FF60();
            m_model->fn_801609F8("act");

            boost::weak_ptr<_stMtnPlayWork> weakMotion = m_model->fn_801601A0("act");
            if (boost::shared_ptr<_stMtnPlayWork> motion = weakMotion.lock()) {
                float frame = ManagerTask::s_table->GetCurrentFrame(m_id);

                stMtnRequestCurrentFrame(motion.get(), 0, frame);
            }
            m_state = 3;
        }
    } else {
        SetupModel(0);
    }

    UpdateModelMatrix();
}

Score::~Score() {}

void Score::UpdateBody(float dt) {
    m_model->fn_80162FB0(dt);

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        if (ManagerTask::s_table != 0) {
            if (ManagerTask::s_table->IsState(m_id, kShootingGameStarted)) {
                ClearModel();
                SetupModel(1);
                m_model->fn_80160BA0("hit");
                m_model->fn_8015FF60();
                m_model->fn_801609F8("hit");
                UpdateModelMatrix();
                m_state = 2;
            } else if (ManagerTask::s_table->IsState(m_id, kShootingGameReset)) {
                m_state = 5;
            }
        }
        break;

    case 2: {
        if (ManagerTask::s_table != 0 && ManagerTask::s_table->IsState(m_id, kShootingGameReset)) {
            m_model->fn_8016268C(-1.0f);
            m_timer = 2.0f;
            m_state = 5;
            break;
        }

        boost::weak_ptr<_stMtnPlayWork> weakMotion = m_model->fn_801601A0("hit");
        if (!weakMotion.expired()) {
            boost::shared_ptr<_stMtnPlayWork> motion = weakMotion.lock();

            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                m_state = 3;
                ClearModel();
                SetupModel(2);
                m_model->fn_80160BA0("act");
                m_model->fn_8015FF60();
                m_model->fn_801609F8("act");
                ManagerTask::s_table->SetFlag(m_id);
            }
            if (m_hitFlags.Test(0)) {
                MotionRange range;

                fn_80415D40(motion.get(), 0, &range);
                ManagerTask::s_table->SetRange(m_id, &range);
            }
        }
        break;
    }

    case 3: {
        if (ManagerTask::s_table != 0 && ManagerTask::s_table->IsState(m_id, kShootingGameReset)) {
            m_model->fn_8016268C(-1.0f);
            m_timer = 2.0f;
            m_state = 5;
            break;
        }

        boost::weak_ptr<_stMtnPlayWork> weakMotion = m_model->fn_801601A0("act");
        if (!weakMotion.expired()) {
            boost::shared_ptr<_stMtnPlayWork> motion = weakMotion.lock();

            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                m_state = 5;
                m_timer = 2.0f;
                m_model->fn_8016268C(-1.0f);
            }
            if (m_hitFlags.Test(0)) {
                MotionRange range;

                fn_80415D40(motion.get(), 0, &range);
                ManagerTask::s_table->SetRange(m_id, &range);
            }
        }

        int count = ManagerTask::s_table->GetCount(m_id);
        if (count != m_count) {
            m_count = count;

            if (m_count < 0) {
                for (int i = 0; i < kScorePartCount; i++) {
                    m_scoreParts[i]->color = lbl_805F137C;
                }
            } else {
                int lit = m_count;
                if (lit > kScorePartCount) {
                    lit = kScorePartCount;
                }

                for (int i = 0; i < lit; i++) {
                    m_scoreParts[i]->color = lbl_805F1370;
                }
                for (int i = lit; i < kScorePartCount; i++) {
                    m_scoreParts[i]->color = lbl_805F1360;
                }
            }
        }
        break;
    }

    case 5:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = 6;
        }
        break;

    case 6:
        if (m_hitFlags.Test(0) && ManagerTask::s_table != 0) {
            ManagerTask::s_table->Finish(m_id, m_attacker);
        }
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }

    UpdateModelMatrix();
}

void Score::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void Score::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_id, sizeof(m_id));
    m_state = 0;
    m_timer = 0.0f;
    m_count = -1;
    for (int i = 0; i < kScorePartCount; i++) {
        m_scoreParts[i] = 0;
    }
}

void Score::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8015CEA0(m_charInfo.get(), &matrix);
    m_model->fn_801633DC(&matrix);
}

void Score::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 0x10) {
                param.field_0x54 = 3;
                if (ManagerTask::s_table != 0) {
                    ManagerTask::s_table->Reset(m_id);
                }
                return;
            }
            if (attack->field_0x40 & 0x2) {
                param.field_0x54 = 5;
                return;
            }
        }

        param.field_0x54 = 1;
        /* The copy of the attack's pointer is released before the table is checked. */
        {
            boost::weak_ptr<CharaColliAttack> weakAttack = param.field_0x0;

            if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
                m_attacker = attack->m_owner;
            }
        }

        if (ManagerTask::s_table != 0 &&
            !ManagerTask::s_table->IsState(m_id, kShootingGameStarted)) {
            ManagerTask::s_table->Start(m_id);
            m_hitFlags.Set(0);
        }
    } else if (m_state == 2 || m_state == 3) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 0x10) {
                param.field_0x54 = 3;
                if (ManagerTask::s_table != 0) {
                    ManagerTask::s_table->Reset(m_id);
                }
                /* The original leaves this branch through a separate exit after releasing
                 * attack (a plain fall-through shares the release code). */
                return;
            }
        }
    }
}

void Score::SetupModel(int index) {
    m_model->fn_80162A68(index, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Score::OnAttack, this, _1));
    }

    if (index == 2) {
        for (int i = 0; i < kScorePartCount; i++) {
            m_model->fn_80162424(i);
            m_scoreParts[i] = m_model->fn_80162298(i);
        }
    }
}

void Score::ClearModel() {
    m_model->fn_8016232C();
    m_model->fn_80161880();
    m_model->fn_801604FC();
    m_model->fn_80162804();
}

bool Score::IsFinished() {
    if (m_owner != 0) {
        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
            return true;
        }

        /* A panel whose attack put the entry into state 1 skips the second owner query while the
         * entry stays in that state. */
        if (m_hitFlags.Test(0) && ManagerTask::s_table != 0 &&
            ManagerTask::s_table->IsState(m_id, kShootingGameStarted)) {
            return false;
        }

        if (fn_800C996C(m_owner)) {
            m_flags |= 1;
            return true;
        }
    }
    return false;
}

} // namespace ShootingGame

extern "C" void fn_8013D91C(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_8013D918(void) {}

void ShootingGame::Score::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Score(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Id", 1, 1, 0xFF, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805292FC = {
    "SHOOTING_PANEL", fn_8013D91C, fn_8013D918, ShootingGame::Score::Create, 0, 0, 0xC83, 1, 0, 10,
    s_paramInfo,
};
