#ifndef GAME_UNKOBJECT_801E72DC_H
#define GAME_UNKOBJECT_801E72DC_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

class UnkObject_801E72DC;

extern "C" {
/* Creates the object that the Chao and Cream gadgets keep, with the argument 0x2000 (placeholder
 * names and behaviour unknown). */
boost::shared_ptr<UnkObject_801E72DC> fn_801E72DC(int arg0);
/* Copies position into the object's position at 0x8 (behaviour inferred from its body). */
void fn_800CCE38(UnkObject_801E72DC *object, const Fn8040Vector3 *position);
}

#endif
