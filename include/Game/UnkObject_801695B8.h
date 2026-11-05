#ifndef GAME_UNKOBJECT_801695B8_H
#define GAME_UNKOBJECT_801695B8_H

#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

/* The object that fn_801695B8 creates and returns in a shared pointer (placeholder name; the
 * original class is unknown). The CircusBall, DashPanel, DashRing, JumpBoard and LongSpring gadgets
 * keep one, advance it each frame, ask it whether a character info has already been dealt with and
 * release it in their destructors (behaviour inferred from the calls). */
class UnkObject_801695B8;

extern "C" {
/* Creates the object (placeholder names; behaviour inferred from the calls): fn_801697F0 advances
 * it each frame, fn_801697CC tells whether it has already dealt with the character info, and
 * fn_8016952C releases it. */
boost::shared_ptr<UnkObject_801695B8> fn_801695B8(void);
void fn_801697F0(UnkObject_801695B8 *object);
bool fn_801697CC(UnkObject_801695B8 *object, const boost::weak_ptr<CharInfo> &target);
void fn_8016952C(boost::shared_ptr<UnkObject_801695B8> &object);
}

#endif
