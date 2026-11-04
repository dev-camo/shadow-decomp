#ifndef GAME_FN_801072C4_H
#define GAME_FN_801072C4_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* Creates a HealingUnit::HealingUnit (class name from the RTTI of the vtable its constructor
 * stores), passes position and rotation to it, and creates a task under task layer 11 for it that
 * receives arg3 (behaviour inferred from the body and the helper at 0x8010735C; owner is passed to
 * the HealingUnit's constructor and arg3 is not understood). The BombServer gadget is the only
 * caller. */
extern "C" void fn_801072C4(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation,
                            const boost::weak_ptr<CharInfo> &owner, float arg3);

#endif
