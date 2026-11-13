#ifndef GAME_PLAYERMANAGE_H
#define GAME_PLAYERMANAGE_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>

class CharInfo;
class UnkManager_80074194;
namespace Player {
class Player;
}

extern int lbl_805E4968;
/* The id that callers pass to fn_800A8B74 (meaning unknown). */
extern s32 lbl_805EE588;

/* Signatures inferred from the original callers and bodies. */
extern "C" {
/* Returns the manager (behaviour inferred): constructs a function-local static manager on first
 * use, registers its destructor and caches its address in a static pointer. */
UnkManager_80074194 *fn_80074194(void);
/* Returns the listed character info nearest to position, or an empty pointer when the list is
 * empty (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800A8A7C(UnkManager_80074194 *manager, const Fn8040Vector3 *position);
/* Returns the listed character info nearest to the segment (by fn_8003C660's distance), or an
 * empty pointer when the list is empty (behaviour inferred). The segment is taken by const
 * reference (inferred): the ELEC_LIGHT gadget passes a temporary whose constructor's result it
 * hands on. */
boost::weak_ptr<CharInfo> fn_800A8810(UnkManager_80074194 *manager,
                                      const UnkSegment_80020A20 &segment);
/* Returns the listed character info whose object has the word id at 0x298, or an empty pointer
 * when there is none (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800A8B74(UnkManager_80074194 *manager, s32 id);
/* Returns the listed character info whose word at 0x54 equals id, or an empty pointer when
 * there is none. The mission clear gadgets pass 0. */
boost::weak_ptr<CharInfo> fn_800A8C54(UnkManager_80074194 *manager, s32 id);
/* Erases from the list of the manager that fn_80074194 returns the first element whose pointee's
 * first word equals the pointer that player holds (behaviour from its body, which reads only the
 * first word at player). The destructor of PlayerWatch.cpp's Watch task passes the address of its
 * shared pointer member; the reference to a shared pointer is inferred from that one caller. */
void fn_800A8E5C(UnkManager_80074194 *manager, const boost::shared_ptr<Player::Player> &player);
void fn_800A8F08(UnkManager_80074194 *manager, const boost::shared_ptr<Player::Player> &player);
void fn_800A8DFC(UnkManager_80074194 *manager);
void fn_800A8D9C(UnkManager_80074194 *manager);
void fn_800A8D48(UnkManager_80074194 *manager);
void fn_800A8CF4(UnkManager_80074194 *manager);
}

#endif
