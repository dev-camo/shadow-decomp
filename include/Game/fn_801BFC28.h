#ifndef GAME_FN_801BFC28_H
#define GAME_FN_801BFC28_H

#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"

/* Passes 0, 0 and its arguments to USA (GUPE8P) fn_801BFC68 (0x801BFC68; Japan's fn_801BFC68 is a
 * different function), which clears bit 0 of flags, reads bit 1, passes position and movement on to
 * its own tests and can set bit 0 again (behaviour inferred from the bodies; what the tests do and
 * the original name are unknown). The HEALING_UNIT gadget's unit passes 0 for arg0 and arg4, the
 * address of a copy of its previous position, its movement since then and 10 for radius;
 * fn_802CB9F0 passes its own object as arg0, whose pointee type is not established. The parameter
 * types, names and pointer spellings are inferred.
 *
 * The seven callers that pass 0 for arg0 (the calls at USA 0x800EE8DC, 0x800F7028, 0x800FA110,
 * 0x800FD424, 0x80107FD8, 0x80278448 and 0x80292BE4) all call fn_80021B08 first and discard its
 * result, then load the arguments in the same order, which suggests a shared inline wrapper that is
 * not recovered. */
extern "C" bool fn_801BFC28(void *arg0, Fn8040Vector3 *position, const Fn8040Vector3 *movement,
                            UnkBitFlags_80014E04 *flags, int arg4, float radius);

#endif
