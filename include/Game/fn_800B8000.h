#ifndef GAME_FN_800B8000_H
#define GAME_FN_800B8000_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

class CharInfo;

/* Sends the character that target refers to commands that place it, among them
 * ClrVelocityCommand, SetPositionCommand with position and SetAngleCommand with rotation
 * (behaviour inferred; the original name is unknown). It sends nothing when the character no
 * longer exists or when fn_80074630's dynamic cast of it to Player::PlayerBase gives null. The pointer parameters could also be
 * references: the bytes pass addresses. */
extern "C" void fn_800B8000(boost::weak_ptr<CharInfo> target, const Fn8040Vector3 *position,
                            const UnkRotation3 *rotation);

#endif
