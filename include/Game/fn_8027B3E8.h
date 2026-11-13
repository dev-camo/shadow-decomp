#ifndef GAME_FN_8027B3E8_H
#define GAME_FN_8027B3E8_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

class CharInfo;
class GadgetModelInfo;

/* Allocates 0x80 bytes and, only when the allocation succeeds, passes its arguments to the
 * constructor at USA (GUPE8P) 0x8027B7EC, adding a float and a pointer of its own; then it passes
 * the result (null when the allocation failed) and 0x254 to ThrowObject::StartEffect (behaviour
 * from its body; what the object does and the original name are unknown). The constructor passes
 * target on to ThrowObject's constructor, which copies the vector it points to. fn_8027B338 is the
 * same kind of function for a thrown container. VacuumContainer::Container's fn_80238184 calls it
 * with the character info that pulls the container in, the model, the container's parameters, the
 * object's position and angles, its argument, an id and two floats.
 *
 * The signature is inferred: the bytes give the eight integer register arguments, one stack word
 * and two floats, but not the parameter types, the const qualifiers or the order of the stack
 * word relative to the floats. The caller passes the character info and the position as
 * temporaries, so those two are references; the other pointer or reference spellings are
 * inferred. */
extern "C" void fn_8027B3E8(const boost::weak_ptr<CharInfo> &charInfo,
                            const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex,
                            int select, int value, const Fn8040Vector3 &position,
                            const UnkRotation3 *angles, const Fn8040Vector3 *target, int eventId,
                            float arg9, float arg10);

#endif
