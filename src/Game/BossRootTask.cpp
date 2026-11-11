/* BossRootTask (class described in Game/BossRootTask.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/BossRootTask.h"
#include "Game/BossVarParam.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/Gadget.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkResource_80012700.h"
#include "Game/fn_8040_vector_types.h"

const char *BossRootTask::s_name = "BossRootTask";

/* The effect launcher that the boss files share (described in Game/BossRootTask.h). */
boost::shared_ptr<SonicteamUSA::System::Effect::EffectLaunchInfoUSA> lbl_805F0240;

BossRootTask::BossRootTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag,
                           BossVarParam *varParam)
    : Task(parent), m_param(varParam) {
    m_name = s_name;
    if (owner != 0) {
        m_owner = owner;
        fn_800C9CD0(m_owner, this);
    }
    m_initialized = false;
    field_0x31 = flag;
}

BossRootTask::~BossRootTask() {
    if (m_owner != 0) {
        fn_800C9BE8(m_owner);
        m_owner = 0;
    }
}

void BossRootTask::Update(float dt) {
    if (!m_initialized) {
        if (fn_80012408(fn_800122D4(), 0x16) == 3 && fn_80074470()->field_0x8 == 3) {
            Init();
            m_initialized = true;
        } else {
            /* This return is an unreachable branch to the epilogue in the original (0x8027DF14
             * in USA); the failed tests jump to the epilogue directly. An early return before
             * the Init call compiles to different code (0x100 bytes instead of 0x104). */
            return;
        }
    }

    if (m_owner != 0) {
        if (fn_800C9D0C(m_owner)) {
            if (m_param->field_0xC != 1) {
                m_param->field_0x14 = 1;
            }
            if (fn_800C9D14(m_owner)) {
                ReadOwner();
                fn_800C9CF8(m_owner);
            }
        } else if (m_param->field_0xC == 1) {
            m_param->field_0x14 = 2;
        }

        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
        }
    }
}

void BossRootTask::ReadOwner() {
    fn_800C9F14(m_owner, &m_param->m_charInfo->m_position);
    fn_800C9ED4(m_owner, &m_param->m_charInfo->m_rotation);
    m_param->m_charInfo->m_rotation.x = fn_8040E99C(m_param->m_charInfo->m_rotation.x);
    m_param->m_charInfo->m_rotation.y = fn_8040E99C(m_param->m_charInfo->m_rotation.y);
    m_param->m_charInfo->m_rotation.z = fn_8040E99C(m_param->m_charInfo->m_rotation.z);
}

/* Sets up the record at 0x60 of lbl_8057D6A0. That it belongs to this file is inferred from its
 * address: it has no data of its own and lies between the static initializer that ends the
 * previous file and ReadOwner. */
extern "C" void fn_8027DE0C(UnkRecord_8027DE0C *record) {
    record->field_0x0 = 0;
    record->field_0x4 = 0;
    record->field_0x8 = 0;
    record->field_0x14 = lbl_805F1C08;
}
