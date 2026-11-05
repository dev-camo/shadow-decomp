/* ShootingGame::ManagerTask (class name from RTTI) and the shooting game table that it owns: one
 * entry per gadget "Id", shared by the balloon generators (PARK_BALLOON) and the score gadget
 * (SHOOTING_PANEL). File name inferred from the class. Method names marked "(name inferred)" in
 * the header are not original. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/ShootingGameManagerTask.h"
#include "Game/CharInfo.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkManager_80074470.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80136B5C.h"
#include "boost/shared_ptr.hpp"
#include <cstring>

using ShootingGame::ManagerTask;

UnkShootingGameTable_8013D818::UnkShootingGameTable_8013D818() {
    memset(m_entries, 0, sizeof(m_entries));
}

void UnkShootingGameTable_8013D818::AddCount(int id, int delta) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_count += delta;

        if (delta <= 0) {
            fn_801D560C(fn_80053164(), 0x743C, 0, 0);
        }
    }
}

s32 UnkShootingGameTable_8013D818::GetCount(int id) const {
    if (id >= 0 && id < kShootingGameEntryCount) {
        return m_entries[id].m_count;
    }

    return 0;
}

void UnkShootingGameTable_8013D818::Start(int id) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_count = 0;
        m_entries[id].m_state = kShootingGameStarted;
        m_entries[id].m_flags.Clear();
        fn_801D560C(fn_80053164(), 0x7436, 0, 0);
    }
}

void UnkShootingGameTable_8013D818::Finish(int id, boost::weak_ptr<CharInfo> player) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_state = kShootingGameFinished;

        if (!m_entries[id].m_flags.Test(0)) {
            if (!player.expired()) {
                if (boost::shared_ptr<CharInfo> character = player.lock()) {
                    int count = m_entries[id].m_count;
                    int rank = 5;
                    int amount;

                    if (count < 0) {
                        amount = -50;
                        rank = 0;
                    } else if (count <= 3) {
                        amount = 0;
                    } else if (count <= 7) {
                        amount = 5;
                        rank = 1;
                    } else if (count <= 11) {
                        amount = 10;
                        rank = 2;
                    } else if (count <= 15) {
                        amount = 15;
                        rank = 3;
                    } else {
                        amount = 50;
                        rank = 4;
                    }

                    fn_8015F6EC(character, 0, amount, 0);
                    fn_80136B5C(rank);
                }
            }

            m_entries[id].m_flags.Set(0);
        }
    }
}

void UnkShootingGameTable_8013D818::Reset(int id) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_count = 0;
        m_entries[id].m_state = kShootingGameReset;
        m_entries[id].m_flags.Clear();
    }
}

void UnkShootingGameTable_8013D818::SetFlag(int id) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_flags.Set(1);
    }
}

bool UnkShootingGameTable_8013D818::TestFlag(int id) const {
    if (id >= 0 && id < kShootingGameEntryCount) {
        return m_entries[id].m_flags.Test(1);
    }

    return false;
}

void UnkShootingGameTable_8013D818::SetRange(int id, MotionRange *range) {
    if (id >= 0 && id < kShootingGameEntryCount) {
        m_entries[id].m_startFrame = range->startFrame;
        m_entries[id].m_endFrame = range->endFrame;
        m_entries[id].m_currentFrame = range->currentFrame;
        m_entries[id].m_speed = range->speed;
    }
}

float UnkShootingGameTable_8013D818::GetCurrentFrame(int id) const {
    if (id >= 0 && id < kShootingGameEntryCount) {
        return m_entries[id].m_currentFrame;
    }

    return 0.0f;
}

UnkShootingGameTable_8013D818 *ManagerTask::s_table;
ManagerTask *ManagerTask::s_instance;

void ManagerTask::Init() {
    s_table = new UnkShootingGameTable_8013D818;
    s_instance = Create();
}

void ManagerTask::Release() {
    if (s_instance != 0) {
        s_instance->m_flags |= 1;
        s_instance = 0;
    }

    if (s_table != 0) {
        delete s_table;
        s_table = 0;
    }
}

ManagerTask::~ManagerTask() {}

/* Returns the result of the current stage action's second virtual function, or true when the
 * manager has no action. */
static bool fn_8013D33C(UnkManager_80074470 *manager) {
    if (manager->m_action != 0) {
        return manager->m_action->fn_8013D37C();
    }

    return true;
}

void ManagerTask::Update(float dt) {
    if (fn_80074470()->field_0x8 != 3 && fn_8013D33C(fn_80074470())) {
        if (s_table != 0) {
            memset(s_table, 0, sizeof(*s_table));
        }
    }
}

const char *ManagerTask::s_name = "ShootingGame::ManagerTask";

ManagerTask *ManagerTask::Create() {
    Task *parent = fn_801EB304(fn_8001C4F8(), 0);

    return new ManagerTask(parent);
}
