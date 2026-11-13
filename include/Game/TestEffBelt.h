#ifndef GAME_TESTEFFBELT_H
#define GAME_TESTEFFBELT_H

#include "Game/DebugDisp.h"
#include "Game/Task.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Player {
class PlayerBase;
}

namespace Test {

/* Test::EffBelt (class name and base from RTTI; its task name is "Eff::Belt"): a 0x30-byte task
 * whose constructor (USA (GUPE8P) 0x802DDEBC) creates a 0x24-byte object at 0x28 from its
 * arguments and sets bit 0x100 of the task flags. The parent is the task that fn_80046178's
 * manager holds at 0x20 (0x802DDEE4-0x802DDEF0). This is a view: the object at 0x28 is not mapped, and the meanings of
 * the arguments are not known (the belt of Game/PlayerEffect.cpp passes 0x3C, a colour from the
 * player's character table and its own float). */
class EffBelt : public Task {
  public:
    EffBelt(const boost::weak_ptr<Player::PlayerBase> &player, int arg1,
            const UnkDebugColor_802E0E84 &color, float arg3);
    virtual ~EffBelt();

    /* Sets the byte at 0x20 of the object at 0x28 (behaviour inferred from its body). */
    void fn_802DDC94();
    /* Passes the player to fn_802DE4B4 for the object at 0x28 (the parameter type is inferred). */
    void fn_802DDCC8(Player::PlayerBase *player);

    /* 0x28 */ u8 field_0x28[0x30 - 0x28];

  private:
    EffBelt(const EffBelt &);
    EffBelt &operator=(const EffBelt &);
};
CHECK_SIZEOF(EffBelt, 0x30);

} // namespace Test

#endif
