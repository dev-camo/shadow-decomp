#ifndef GAME_UNKOBJECT_80169BB0_H
#define GAME_UNKOBJECT_80169BB0_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* The object that fn_80169BB0 and fn_8034DD24 create and return a weak pointer to (placeholder
 * name; the original class is unknown). Its callers discard the result. */
class UnkObject_80169BB0;

extern "C" {
/* Creates an object of the given kind for target and returns a weak pointer to it (behaviour
 * inferred; the target is not used by the body, and CircusBall's hit callback discards the
 * result). */
boost::weak_ptr<UnkObject_80169BB0> fn_80169BB0(const boost::weak_ptr<CharInfo> &target, int kind);
/* Creates an object of the given kind at position and returns a weak pointer to it (behaviour
 * inferred: the kind also selects the value that fn_8034DAF0 returns for it, and both are passed
 * on to fn_8034AF24 with arg1, position and arg3; arg1 and arg3 are not understood). FallRoad
 * passes the position of its character info. The kind comes first, as in fn_8034DF48, which
 * fn_80169BB0 calls with its kind and 0; FallRoad's call evaluates the float before the position,
 * so the float precedes the position here, but its exact place before it is not known. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DD24(int kind, float arg1, const Fn8040Vector3 *position,
                                                int arg3);
}

#endif
