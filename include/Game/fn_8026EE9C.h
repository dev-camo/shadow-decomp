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
/* The correction for a list of hits, or the zero vector for a null or empty list (behaviour
 * inferred): the sum, over the hits whose fn_80099080 word is 1 and whose float at 0x10 of the
 * part at 0xC is negative, of the hit's vector at 0x20 times minus that float, plus the mean over
 * all hits of that vector times 0.02. */
Fn8040Vector3 fn_8026EF38(SonicteamUSA::System::Collision::CollisionResultList *hits);
/* Starts from motion and, for each hit whose vector at 0x20 has a dot product with it that is not
 * above 0, sets blocked to true and subtracts that vector times the dot product; from velocity it
 * subtracts the same vector times its dot product with velocity. Returns what is left of motion
 * (behaviour inferred). */
Fn8040Vector3 fn_8026F0BC(Fn8040Vector3 *velocity, const Fn8040Vector3 *motion,
                          SonicteamUSA::System::Collision::CollisionResultList *hits,
                          bool *blocked);
/* Moves a sphere of the given radius from position by motion in at most five steps (behaviour
 * inferred). Each step asks the world (fn_80021934) for what the sphere meets on the way, moves
 * by the step and by fn_8026EF38's correction, and continues with what fn_8026F0BC leaves of the
 * motion that the step did not cover; it stops when nothing was met or when the squared length
 * of that rest is below 0.0001. position receives where the sphere ends up and motion how far it
 * came; the result is the flag that fn_8026F0BC sets. */
bool fn_8026F204(Fn8040Vector3 *velocity, Fn8040Vector3 *position, Fn8040Vector3 *motion,
                 UnkProbePredicate predicate, void *context, float radius);
}

#endif
