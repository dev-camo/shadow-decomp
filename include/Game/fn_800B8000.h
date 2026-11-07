#ifndef GAME_FN_800B8000_H
#define GAME_FN_800B8000_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

class CharInfo;

/* Sends the character that target refers to commands that place it, among them
 * ClrVelocityCommand, SetPositionCommand with position and SetAngleCommand with rotation
 * (behaviour inferred; the original name is unknown). It sends nothing when the character no
 * longer exists or when boost::dynamic_pointer_cast of it to Player::PlayerBase (0x80074630)
 * gives null. The pointer parameters could also be references: the bytes pass addresses. target
 * is taken by const reference (inferred): the SET_CHARACTOR gadget passes the address of its own
 * reference parameter without the copy that a by-value parameter would need (0x80119B3C). */
extern "C" void fn_800B8000(const boost::weak_ptr<CharInfo> &target, const Fn8040Vector3 *position,
                            const UnkRotation3 *rotation);

#endif
