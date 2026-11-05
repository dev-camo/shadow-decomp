#ifndef GAME_FN_8026EE9C_H
#define GAME_FN_8026EE9C_H

#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8040_vector_types.h"

extern "C" {
/* Moves a sphere of the given radius from start to position through the world's collision
 * objects and stores where it ends up in position (behaviour inferred; the original name is
 * unknown). It passes velocity, a copy of start, position - start, predicate, context and radius
 * to fn_8026F204 and returns that function's byte result. predicate selects the collision objects
 * and receives context in its records; ElecSecurityCode passes its character info's velocity. */
bool fn_8026EE9C(Fn8040Vector3 *position, const Fn8040Vector3 *start, Fn8040Vector3 *velocity,
                 UnkProbePredicate predicate, void *context, float radius);
}

#endif
