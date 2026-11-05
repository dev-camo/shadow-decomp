#ifndef GAME_SHOOTINGGAMEMANAGERTASK_H
#define GAME_SHOOTINGGAMEMANAGERTASK_H

#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_804157E8_motion_range.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;

/* The number of entries of the shooting game table: the entry index is the gadgets' "Id"
 * parameter, which the table methods accept from 0 to 255. */
static const int kShootingGameEntryCount = 256;

/* The states that the table's methods store in an entry (names inferred from the methods that set
 * them). The balloon generator starts its balloons once its entry is started and stops at
 * finished. */
enum UnkShootingGameState {
    kShootingGameIdle = 0,
    kShootingGameStarted = 1,
    kShootingGameReset = 2,
    kShootingGameFinished = 3
};

/* One entry of the shooting game table (placeholder name; the binary does not name it): the state
 * of the gadgets that share one "Id". The constructor is emitted out of line after the table's
 * constructor, as an inline function is. */
struct UnkShootingGameEntry_8013D86C {
    UnkShootingGameEntry_8013D86C()
        : m_startFrame(0.0f), m_endFrame(0.0f), m_speed(0.0f), m_currentFrame(0.0f), m_count(0),
          m_state(kShootingGameIdle) {}

    /* The score gadget (SHOOTING_PANEL) stores a motion range here with SetRange; the names follow
     * MotionRange's members. */
    /* 0x00 */ float m_startFrame;
    /* 0x04 */ float m_endFrame;
    /* 0x08 */ float m_speed;
    /* 0x0C */ float m_currentFrame;
    /* The balance of the popped balloons: AddCount adds 1, 2 or -2 per balloon depending on its
     * kind, and Finish turns it into the amount that it passes to fn_8015F6EC (name inferred). */
    /* 0x10 */ s32 m_count;
    /* 0x14 */ s32 m_state; /* an UnkShootingGameState */
    /* Bit 0 is set once Finish has run (it then awards nothing); bit 1 is set by SetFlag and read
     * by TestFlag (meaning unknown). Start and Reset clear both. */
    /* 0x18 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(UnkShootingGameEntry_8013D86C, 0x1C);

/* The table of the shooting game entries (placeholder name; the binary does not name it). The
 * ShootingGame manager task allocates it when the PARK_BALLOON gadget type loads. Every method
 * ignores an id outside 0 to 255 (names inferred). */
class UnkShootingGameTable_8013D818 {
  public:
    UnkShootingGameTable_8013D818();
    /* Declared, and empty: ManagerTask::Release's delete of the table keeps a second null test,
     * which this compiler emits for a class with a destructor and folds away without one. */
    ~UnkShootingGameTable_8013D818() {}

    /* True when the entry of id is in state (name inferred). */
    bool IsState(int id, int state) const {
        if (id >= 0 && id < kShootingGameEntryCount) {
            return m_entries[id].m_state == state;
        }

        return false;
    }

    /* Adds delta to the entry's count and plays sound 0x743C when delta is not positive (name
     * inferred). */
    void AddCount(int id, int delta);
    /* Returns the entry's count, or 0 (name inferred). */
    s32 GetCount(int id) const;
    /* Clears the count and the flags, sets the state to started and plays sound 0x7436 (name
     * inferred). */
    void Start(int id);
    /* Sets the state to finished. Unless flag bit 0 is set, sets it and, when player still exists,
     * passes player and an amount for the count to fn_8015F6EC (meaning unknown; it can be
     * negative) and a rank from 0 to 5 to fn_80136B5C (name inferred). */
    void Finish(int id, boost::weak_ptr<CharInfo> player);
    /* Clears the count and the flags and sets the state to reset (name inferred). */
    void Reset(int id);
    /* Sets flag bit 1 (name inferred). */
    void SetFlag(int id);
    /* Tests flag bit 1 (name inferred). */
    bool TestFlag(int id) const;
    /* Copies range into the entry (name inferred). */
    void SetRange(int id, MotionRange *range);
    /* Returns the entry's current frame, or 0 (name inferred). */
    float GetCurrentFrame(int id) const;

    /* 0x0000 */ UnkShootingGameEntry_8013D86C m_entries[kShootingGameEntryCount];
};
CHECK_SIZEOF(UnkShootingGameTable_8013D818, 0x1C00);

namespace ShootingGame {

/* ShootingGame::ManagerTask (class name from RTTI): a task under task layer 0 that clears the
 * shooting game table each frame under the condition described at Update. The PARK_BALLOON
 * gadget type's load and unload hooks call Init and Release; the table and the task live in the
 * two static members. Method names marked "(name inferred)" are not original. */
class ManagerTask : public Task {
  public:
    ManagerTask(Task *parent) : Task(parent) {
        m_name = s_name;
    }
    virtual ~ManagerTask();
    /* Clears the table unless the word at 0x8 of the manager that fn_80074470 returns is 3 or its
     * current stage action's second virtual function returns false (behaviour inferred). */
    virtual void Update(float dt);

    /* Creates the task under task layer 0 (name inferred). */
    static ManagerTask *Create();
    /* Allocates the table and creates the task (name inferred). */
    static void Init();
    /* Marks the task finished and deletes the table (name inferred). */
    static void Release();

    static const char *s_name;
    static UnkShootingGameTable_8013D818 *s_table;
    static ManagerTask *s_instance;
};
CHECK_SIZEOF(ShootingGame::ManagerTask, 0x28);

} // namespace ShootingGame

#endif
