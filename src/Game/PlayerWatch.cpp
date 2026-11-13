/* Game/PlayerWatch.cpp (file name from the type names of its two classes,
 * "@unnamed@PlayerWatch_cpp@::WaitTask" and "@unnamed@PlayerWatch_cpp@::Watch"). Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/PlayerWatch.h"
#include "Game/CharInfo.h"
#include "Game/UnkManager_80014B90.h"
#include "layout_check.h"

namespace {

/* Its constructor asks the table of fn_80014B90 for a record, passing "Player" as the name
 * argument, and ignores the result (the request fails when the table is full); its destructor
 * requests the record's removal, which does nothing when no record was taken. It marks itself
 * finished once the player no longer exists or has both bits 0x46 and 0x47 of its bit set at
 * 0xA8 set (meaning of the bits unknown). */
class WaitTask : public Task {
  public:
    WaitTask(Task *parent, const boost::weak_ptr<Player::PlayerBase> &player);
    virtual ~WaitTask();
    virtual void Update(float dt);

    /* 0x28 */ boost::weak_ptr<Player::PlayerBase> m_player; /* name inferred */
};
CHECK_SIZEOF(WaitTask, 0x30);

/* Holds the player and, when destroyed, takes it out of the list of the manager that
 * fn_80074194 returns. It does not override Task's Update. */
class Watch : public Task {
  public:
    Watch(Task *parent, const boost::shared_ptr<Player::Player> &player);
    virtual ~Watch();

    /* 0x28 */ boost::shared_ptr<Player::Player> m_player; /* name inferred */
};
CHECK_SIZEOF(Watch, 0x30);

Watch::Watch(Task *parent, const boost::shared_ptr<Player::Player> &player)
    : Task(parent), m_player(player) {
    m_name = "Player::Watch";
}

Watch::~Watch() {
    if (m_player.get() != 0) {
        fn_800A8E5C(fn_80074194(), m_player);
    }
}

WaitTask::WaitTask(Task *parent, const boost::weak_ptr<Player::PlayerBase> &player)
    : Task(parent), m_player(player) {
    m_name = "Wait";
    fn_8025D1C4(fn_80014B90(), this, 0, "Player");
}

WaitTask::~WaitTask() {
    fn_8025D144(fn_80014B90(), this);
}

void WaitTask::Update(float) {
    boost::shared_ptr<Player::PlayerBase> player = m_player.lock();
    if (player.get() == 0) {
        m_flags |= 1;
        return;
    }
    if (fn_8006CD78(player->field_0xA8, 0x46) && fn_8006CD78(player->field_0xA8, 0x47)) {
        m_flags |= 1;
    }
}

} // namespace

Task *fn_800B8360(Task *parent, const boost::shared_ptr<Player::Player> &player) {
    return new Watch(parent, player);
}

Task *fn_800B8310(Task *parent, const boost::weak_ptr<Player::PlayerBase> &player) {
    return new WaitTask(parent, player);
}
