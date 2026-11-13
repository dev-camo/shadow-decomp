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
 * character they look at to it, and the TornadoCollision gadget reads the bit set at 0xA8. None of
 * its own members are mapped: the view declared here is its 0x290-byte PlayerBase part, while the
 * object is larger (fn_80076D24, in its vtable, returns a copy of the shared pointer at 0x290),
 * and its full size is unknown. */
class Player : public PlayerBase {};
CHECK_SIZEOF(Player, 0x290);

} // namespace Player

extern "C" {
/* True when bit number bit of the bit set starting at bits is set (behaviour inferred). */
bool fn_8006CD78(const u32 *bits, int bit);
/* Returns bit 3 of the player's motion work flags (behaviour and signature inferred). */
bool fn_800A9E1C(const boost::shared_ptr<Player::PlayerBase> &player);
/* Clears bit number bit of the bit set starting at bits (behaviour from its body). */
void fn_80076CC8(u32 *bits, int bit);
/* Sets bit number bit of the bit set starting at bits (behaviour from its body). */
void fn_80076CA8(u32 *bits, int bit);
/* Return fn_8006CD78 for bit 10 (fn_800B78B0), bit 15 (fn_800B7938), bit 2 (fn_800B7AD0) or
 * bit 41 (fn_800B7828) of the bit set at 0xA8 of the Player::PlayerBase that
 * boost::dynamic_pointer_cast (0x80074630) gives for charInfo, or false when there is none
 * (behaviour inferred). */
bool fn_800B78B0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7938(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7AD0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7828(const boost::shared_ptr<CharInfo> &charInfo);
/* Erases from the list of the manager that fn_80074194 returns the first element whose pointee's
 * first word equals the pointer that player holds (behaviour from its body, which reads only the
 * first word at player). The destructor of PlayerWatch.cpp's Watch task passes the address of its
 * shared pointer member; the reference to a shared pointer is inferred from that one caller. */
void fn_800A8E5C(UnkManager_80074194 *manager, const boost::shared_ptr<Player::Player> &player);
}

#endif
