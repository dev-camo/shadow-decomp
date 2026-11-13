#ifndef GAME_FN_8027B338_H
#define GAME_FN_8027B338_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

class CharInfo;
class GadgetModelInfo;

/* Allocates 0x80 bytes and, only when the allocation succeeds, passes all of its arguments
 * unchanged to the constructor at USA (GUPE8P) 0x8027B7EC; the result is not kept (behaviour from
 * its body; what the object does and the original name are unknown). The CONTAINER gadget's
 * container calls it when it is thrown, with the thrower, its model, its "model", "select" and
 * "value" parameters, its character info's position, the angles of its grab matrix, its character
 * info's velocity, a scaled vector, an id and 0, 1 and 600. The SECRET CONTAINER gadget's
 * container (Game/ContainerSecret.h) calls it in the same way, with 0, 6 and its "weapon"
 * parameter in place of the three parameters.
 *
 * The signature is inferred: the bytes give the eight integer register arguments, the two stack
 * words and the three floats, but not the parameter types, the pointer or reference spellings or
 * the const qualifiers. */
extern "C" void fn_8027B338(const boost::weak_ptr<CharInfo> &thrower,
                            const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex,
                            int select, int value, const Fn8040Vector3 *position,
                            const UnkRotation3 *angles, const Fn8040Vector3 *velocity,
                            const Fn8040Vector3 &acceleration, int eventId, float arg10,
                            float arg11, float arg12);

#endif
