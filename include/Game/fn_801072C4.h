#ifndef GAME_FN_801072C4_H
#define GAME_FN_801072C4_H

#include "Game/CharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* Builds a HealingUnit::HealingUnit (Game/HealingUnit.h) with owner in a shared pointer, calls its
 * Initialize with null and its ResetTransform with position and rotation, creates a
 * GadgetPickup::PickupBaseTask for it under task layer 11 (only when the allocation succeeds) and
 * calls SetRadius(radius) on the result without testing it for null. The task keeps radius squared
 * and, only when that is > 0 (a NaN skips it), also finishes when fn_80169BE4 reports true for the
 * unit; what fn_80169BE4 tests is not recovered (behaviour inferred). The BombServer gadget is the
 * only caller. */
extern "C" void fn_801072C4(const Fn8040Vector3 *position, const UnkRotation3 *rotation,
                            const boost::weak_ptr<CharInfo> &owner, float radius);

#endif
