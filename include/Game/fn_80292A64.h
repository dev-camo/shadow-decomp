#ifndef GAME_FN_80292A64_H
#define GAME_FN_80292A64_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* Allocates a Parachute::Parachute task (class name from the RTTI of its vtable at 0x805525E8)
 * under task layer 11 and, when the allocation succeeds, passes the three arguments to its
 * constructor (0x802933B4; behaviour inferred). A failed allocation skips the constructor
 * (USA (GUPE8P) 0x80292A98-0x80292AA0). The ROCKET gadget's rocket passes its rider and its hang
 * position and angles when its flight ends. The parameter spellings are inferred, and so is the
 * void return type: the function returns with the allocation or constructor result in r3, and
 * its only caller (0x801161BC) does not use r3 after the call. */
extern "C" void fn_80292A64(const boost::weak_ptr<CharInfo> &rider, const Fn8040Vector3 *position,
                            const UnkRotation3 *angles);

#endif
