#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "Game/CharInfo.h"
#include "Game/PlayerBase.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"
#include <dolphin/types.h>

namespace Player {

/* Player::Player (class name from RTTI, with CharInfo, Player::PlayerObject and Player::PlayerBase
 * as its bases). The type record that the cast helper at 0x8009DC3C (USA, GUPE8P) passes to
 * __dynamic_cast for it (0x805E47E0) names it, and so does the record at 0x805E4060 that its
 * vtable (0x805200AC) points to. Gadgets and player commands cast the shared CharInfo of the
 * character they look at to it, and the TornadoCollision gadget reads the bit set at 0xA8. Only
 * the shared pointer at 0x290 and the signed id at 0x298 are mapped here. The full native class
 * extent is not established. Construction, copying and destruction are not recovered
 * through this view. */
class Player : public PlayerBase {
  public:
    /* 0x290 */ boost::shared_ptr<UnkPlayerShared_80076D24> field_0x290;
    /* 0x298 */ s32 field_0x298;

  private:
    Player();
    Player(const Player &);
    Player &operator=(const Player &);
    virtual ~Player();
};
/* Checks only the recovered borrowed view. */
CHECK_SIZEOF(Player, 0x29C);

} // namespace Player

extern "C" {
/* Stores id at 0x298 (behaviour and signature inferred). */
void fn_80077294(Player::Player *player, s32 id);
/* True when bit number bit of the bit set starting at bits is set (behaviour inferred). */
bool fn_8006CD78(const u32 *bits, int bit);
/* Returns bit 3 of the player's motion work flags (behaviour and signature inferred). */
bool fn_800A9E1C(const boost::shared_ptr<Player::PlayerBase> &player);
/* Clears bit number bit of the bit set starting at bits (behaviour from its body). */
void fn_80076CC8(u32 *bits, int bit);
/* Return fn_8006CD78 for bit 10 (fn_800B78B0), bit 15 (fn_800B7938), bit 2 (fn_800B7AD0) or
 * bit 41 (fn_800B7828) of the bit set at 0xA8 of the Player::PlayerBase that
 * boost::dynamic_pointer_cast (0x80074630) gives for charInfo, or false when there is none
 * (behaviour inferred). */
bool fn_800B78B0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7938(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7AD0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7828(const boost::shared_ptr<CharInfo> &charInfo);
}

#endif
