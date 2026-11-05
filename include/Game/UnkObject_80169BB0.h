#ifndef GAME_UNKOBJECT_80169BB0_H
#define GAME_UNKOBJECT_80169BB0_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* The object that fn_80169BB0 and fn_8034DD24 create and return a weak pointer to (placeholder
 * name; the original class is unknown). Most callers discard the result; SuctionPillar keeps the
 * one from fn_80169B1C. */
class UnkObject_80169BB0;

extern "C" {
/* Creates an object of the given kind for target and returns a weak pointer to it (behaviour
 * inferred; the target is not used by the body, and the hit callbacks of CircusBall, DashPanel
 * and DashRing discard the result). */
boost::weak_ptr<UnkObject_80169BB0> fn_80169BB0(const boost::weak_ptr<CharInfo> &target, int kind);
/* Creates an object of the given kind at position and returns a weak pointer to it (behaviour
 * inferred: the kind also selects the value that fn_8034DAF0 returns for it, and both are passed
 * on to fn_8034AF24 with arg1, position and arg3; arg1 and arg3 are not understood). FallRoad
 * passes the position of its character info. The kind comes first, as in fn_8034DF48, which
 * fn_80169BB0 calls with its kind and 0; FallRoad's call evaluates the float before the position,
 * so the float precedes the position here, but its exact place before it is not known. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DD24(int kind, float arg1, const Fn8040Vector3 *position,
                                                int arg3);
/* Creates an object of the given kind and returns a weak pointer to it (purpose unknown; the
 * second argument is not understood). fn_80169BB0 passes its kind and 0 and returns the result to
 * its caller; GravityChangeZone passes 5 and -2 when a character touches its zone and discards the
 * result. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DF48(int kind, int arg1);
/* Creates an object of the given kind at position: forwards to fn_8034DD9C with -2 as its last
 * argument (behaviour inferred from the 0x30-byte wrapper). SuctionPillar keeps the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_80169B1C(int kind, const Fn8040Vector3 *position);
/* Unless the object has expired, passes a value derived from it (through fn_8034DC44 and
 * fn_8034AA14) to fn_8034A418 together with fn_80177F38's result; then empties the weak pointer
 * (fn_8034DBD8) (behaviour inferred; purpose unknown). SuctionPillar calls it when it stops rising
 * and from its destructor. */
void fn_8034DB60(boost::weak_ptr<UnkObject_80169BB0> &object);
/* Creates an object of the given kind at position: forwards to fn_8034DD24 with -2 as its last
 * argument (behaviour inferred from the 0x30-byte wrapper). MagmaGuard passes 7 and 2.0 at its
 * character info's position when its health is used up and discards the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_80169AEC(int kind, float arg1,
                                                const Fn8040Vector3 *position);
}

#endif
