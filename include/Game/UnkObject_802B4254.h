#ifndef GAME_UNKOBJECT_802B4254_H
#define GAME_UNKOBJECT_802B4254_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>

class UnkObject_802B4254;

extern "C" {
/* Creates the second object that the Chao and Cream gadgets keep (placeholder names; arguments 2
 * and 0 unknown). */
boost::shared_ptr<UnkObject_802B4254> fn_802B4254(int arg0, int arg1);
/* Methods of that object: the float is stored at 0x20 of its target, the position goes on to
 * fn_802B44EC, and the byte at 0x14 of its target is replaced by the argument and its old value
 * returned (behaviour inferred from the bodies). */
void fn_802B4498(UnkObject_802B4254 *object, float value);
void fn_802B44C8(UnkObject_802B4254 *object, const Fn8040Vector3 *position);
u8 fn_802B4510(UnkObject_802B4254 *object, int value);
}

#endif
