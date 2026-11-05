#ifndef GAME_FN_800EDA3C_H
#define GAME_FN_800EDA3C_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* Creates a Bomb::Bomb (class name from the RTTI of the vtable its constructor stores), passes
 * position and rotation to it, and creates a task under task layer 11 for it that receives arg3
 * (behaviour inferred from the body; owner is passed to the Bomb's constructor and arg3 is not
 * understood). The BombServer gadget is the only caller. */
extern "C" void fn_800EDA3C(const Fn8040Vector3 *position, const UnkRotation3 *rotation,
                            const boost::weak_ptr<CharInfo> &owner, float arg3);

#endif
