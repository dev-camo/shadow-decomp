#ifndef GAME_UNKCHARINFO_800999C0_H
#define GAME_UNKCHARINFO_800999C0_H

#include "Game/CharInfo.h"
#include "Game/PlayerBase.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>

/* A Player::PlayerBase subclass with a placeholder name, taken from the address of the copy
 * constructor of its shared pointer. The type record that the cast helper at 0x8009DC3C (USA,
 * GUPE8P) passes to __dynamic_cast for it is named Player::Player and lists CharInfo,
 * Player::PlayerObject and Player::PlayerBase as its bases. Gadgets and player commands cast the
 * shared CharInfo of the character they look at to it, and the TornadoCollision gadget reads the
 * bit set at 0xA8. None of its own members are mapped, and its size is unknown. */
class UnkCharInfo_800999C0 : public Player::PlayerBase {};

extern "C" {
/* True when bit number bit of the bit set starting at bits is set (behaviour inferred). */
bool fn_8006CD78(const u32 *bits, int bit);
/* Return fn_8006CD78 for bit 10 (fn_800B78B0), bit 15 (fn_800B7938), bit 2 (fn_800B7AD0) or
 * bit 41 (fn_800B7828) of the bit set at 0xA8 of the object that fn_80074630 and fn_8006D714
 * derive from charInfo, or false when there is none (behaviour inferred). */
bool fn_800B78B0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7938(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7AD0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7828(const boost::shared_ptr<CharInfo> &charInfo);
}

#endif
