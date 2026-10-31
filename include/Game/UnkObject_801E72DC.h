#ifndef GAME_UNKOBJECT_801E72DC_H
#define GAME_UNKOBJECT_801E72DC_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

/* The object that fn_801E72DC creates (placeholder name; not mapped). Gadgets keep it in a
 * shared_ptr and move it with their position. */
class UnkObject_801E72DC;

extern "C" {
/* Creates the object (the meaning of the argument is unknown; the Chao and Cream gadgets pass
 * 0x2000 and CircusBall 0x400). */
boost::shared_ptr<UnkObject_801E72DC> fn_801E72DC(int arg0);
/* Copies position into the object's position at 0x8 (behaviour inferred from its body). */
void fn_800CCE38(UnkObject_801E72DC *object, const Fn8040Vector3 *position);
/* Hands the shared pointer on to fn_8008A1B0 (purpose unknown; CircusBall calls it from its
 * destructor). */
void fn_801E7108(boost::shared_ptr<UnkObject_801E72DC> &object);
}

#endif
