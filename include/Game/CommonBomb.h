#ifndef GAME_COMMONBOMB_H
#define GAME_COMMONBOMB_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

class CharInfo;

extern "C" {
/* Creates a 0x50-byte CommonBomb task (class name from the string its constructor fn_801DDBBC
 * stores as the task's name) under task layer 11 at position (behaviour inferred). arg1 is stored
 * at 0x48 of the task and is a type index into the table of 0xC-byte entries at 0x8053D570; arg2 is
 * stored at 0x4C and is a bit mask that the task tests against the hit character's word at 0x5C
 * (bit 2 for 4, bit 1 for 3, bit 0 for 1 or 6). arg3 is passed on to fn_80054DD0 (meaning
 * unknown). */
void fn_801DD0BC(const Fn8040Vector3 *position, int arg1, int arg2, float arg3);
/* As fn_801DD0BC with an owner: 0x801DD040 passes owner to the constructor fn_801DDBBC where
 * fn_801DD0BC passes an empty weak pointer (behaviour from the two bodies). The const reference is
 * inferred; the code is that of a pointer parameter. */
void fn_801DD040(const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *position, int arg1,
                 int arg2, float arg3);
}

#endif
