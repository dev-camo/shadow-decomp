#ifndef GAME_PLAYERSHADOWELECTRIC_H
#define GAME_PLAYERSHADOWELECTRIC_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Player {

class PlayerBase;

namespace Shadow {

/* Player::Shadow::Electric (class name and base from RTTI): a 0x30-byte task that Player::Effect's
 * implementation creates (Game/PlayerEffect.cpp). Its constructor (USA (GUPE8P) 0x8028D9D8)
 * creates a 0x1C-byte object, which keeps the slot at its 0x8, at 0x28 and stores the task in the
 * slot. This is a view: the object at 0x28 is not mapped. */
class Electric : public Task {
  public:
    Electric(Task *parent, const boost::shared_ptr<PlayerBase> &player, Electric **slot);
    virtual ~Electric();

    /* Clears the slot, resets bit 0x26 of the player's bit set at 0xA8 and sets the object's
     * word at 0x18 to 2 before calling fn_8028DAE8 on it (behaviour inferred from its body). */
    void fn_8028D834();

    /* 0x28 */ u8 field_0x28[0x30 - 0x28];

  private:
    Electric(const Electric &);
    Electric &operator=(const Electric &);
};
CHECK_SIZEOF(Electric, 0x30);

} // namespace Shadow
} // namespace Player

#endif
