#ifndef GAME_FN_8033E644_H
#define GAME_FN_8033E644_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

class GadgetModelInfo;

/* This header declares fn_8033E644 and fn_8033E828, the function that follows it. */
extern "C" {
/* Starts from three default words (copied from lbl_80510B58). When the model is not null it looks
 * up the model's entries modelIndex + 1000, + 2000 and + 3000 (fn_8015F7BC), and for each entry
 * that is not null it replaces the matching word with what fn_80012700 returns for the file name
 * at the entry's 0x4; when the model is null it skips all three lookups. Then it works with the
 * words that are not zero, the position and the rotation (behaviour only partly read; the
 * original name is unknown). The CONTAINER gadget's container calls it when it breaks, with a
 * copy of its model pointer that it destroys after the call, its "model" parameter and its
 * character info's position and rotation; the SECRET CONTAINER gadget's container
 * (Game/ContainerSecret.h) calls it in the same way with 0 for modelIndex. The signature is
 * inferred, as are the by-value model pointer and the pointer spellings. */
void fn_8033E644(boost::shared_ptr<GadgetModelInfo> model, int modelIndex,
                 const Fn8040Vector3 *position, const UnkRotation3 *rotation);
/* Chooses by select, through a jump table, what to create (behaviour from its body; the callees'
 * purposes are only partly read; the original name is unknown):
 * - 0, or any value outside 0 to 6 (an unsigned compare): returns without a call.
 * - 1: fn_8010B824 with value, copies of position and rotation, and 0.5.
 * - 2: fn_80221BA8 with position, rotation, the vector (0, 1, 0), value and -1.
 * - 3: ScatteringRing::ScatteringRing::Create with a local record (constructed by the empty
 *   fn_800B7E40) holding position and, at 0xC, value as the ring count, and 1.
 * - 4: fn_801070CC with a copy of position whose Y is lowered by 10, rotation and 500 (value is
 *   not used).
 * - 5: EnergyCore::EnergyCoreTask::Create with value, copies of position and rotation, and 500.
 * - 6: fn_8033E9A0 with position, rotation, the vector (0, 1, 0) and value.
 * The CONTAINER gadget's container calls it when it breaks, with its "select" and "value"
 * parameters and its character info's position and rotation; the SECRET CONTAINER gadget's
 * container calls it with 6 and its "weapon" parameter. The signature is inferred. */
void fn_8033E828(int select, int value, const Fn8040Vector3 *position,
                 const UnkRotation3 *rotation);
}

#endif
