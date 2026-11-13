#ifndef GAME_PLAYERSHADOWSUPER_H
#define GAME_PLAYERSHADOWSUPER_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Player {

class PlayerBase;

namespace Shadow {

/* The 0x78-byte object that Super's constructor creates with fn_8027EFC4 (placeholder name; its
 * class is unknown and its members are not mapped). */
class UnkSuperObject_8027EFC4;

/* Player::Shadow::Super (class name and base from RTTI): a 0x30-byte task that Player::Effect's
 * implementation creates (Game/PlayerEffect.cpp). Its constructor (USA (GUPE8P) 0x8027E2D0) creates
 * a 0x78-byte object, which keeps the slot, at 0x28 and stores the task in the slot. */
class Super : public Task {
  public:
    Super(Task *parent, const boost::shared_ptr<PlayerBase> &player, Super **slot);
    virtual ~Super();

    /* Clears the slot, resets bits 0x24, 0x25 and 0x33 of the player's bit set at 0xA8 and calls
     * fn_80079334 for the player with 0, among other work (behaviour inferred from its body). */
    void fn_8027E0D8();

    /* 0x28 */ UnkSuperObject_8027EFC4 *field_0x28;

  private:
    Super(const Super &);
    Super &operator=(const Super &);
};
CHECK_SIZEOF(Super, 0x30);

} // namespace Shadow
} // namespace Player

#endif
