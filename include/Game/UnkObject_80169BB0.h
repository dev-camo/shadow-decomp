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
/* Creates an object of the given kind and returns a weak pointer to it (behaviour inferred). The
 * body ignores target and passes only the kind and 0 on to fn_8034DF48; the hit callbacks of
 * CircusBall, DashPanel, DashRing, JumpBoard, LongSpring and Spring and Catapult's launch discard
 * the result. */
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
 * its caller; GravityChangeZone passes 5 and -2 when a character touches its zone, and
 * SpringPanel's hit callback passes 5 and -2 when it launches a character; both discard the
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
/* As fn_8034DF48, except that it passes 1 instead of 0 in r7 to fn_8034B0A8 (behaviour inferred
 * from its body; purpose unknown). The Tornado gadget's towing task passes 7 and -2 and keeps the
 * result. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DEE4(int kind, int arg1);
/* As fn_8034DF48, except that it passes 2 instead of 0 in r7 to fn_8034B0A8, together with arg2
 * in f1 (behaviour inferred from its body; purpose unknown; the order of the integer and float
 * parameters is inferred). The DEVIL_DOOM boss's ATK_PSYCHO_PREPARE mode unit passes 8, -2 and 3.0
 * and discards the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DE74(int kind, int arg1, float arg2);
/* Creates an object of the given kind at position, as fn_8034DD9C does but passing 0 where that
 * function passes 1 to fn_8034AF24 (behaviour inferred from the two bodies; arg2 is not
 * understood). The ELEC_CORE gadget passes 9 and -2 at its character info's position when it
 * breaks and discards the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_8034DE08(int kind, const Fn8040Vector3 *position, int arg2);
/* Attempts to create an object of the given kind at position: forwards to fn_8034DE08 with -2 as
 * its last argument (behaviour inferred from the 0x30-byte wrapper). The RAIL SWITCH gadget passes
 * 9 at its character info's position when an attack toggles it, and the BA GUNSHIP gadget passes 3
 * at its character info's position when it breaks; both discard the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_80169B80(int kind, const Fn8040Vector3 *position);
/* Forwards kind and 0 to fn_8034DEE4 and returns its result; the body ignores target (behaviour
 * inferred from the 0x34-byte wrapper). The ROCKET gadget's rocket passes its rider and 11 when it
 * launches and keeps the result. */
boost::weak_ptr<UnkObject_80169BB0> fn_80169B4C(const boost::weak_ptr<CharInfo> &target, int kind);
}

#endif
