/* True when |value| < 0.0001 (behaviour inferred). Callers use the inline definition in
 * Game/fn_8000A950_near_zero.h; this is the copy the link kept. It follows fn_8000A800 in code
 * that has no source yet, so it stays a separate unit until that code is recovered.
 * This NonMatching unit is an artificial split of code that has no source yet, and normal builds
 * link the original object for it; its ordinary definition does not include the header, so it does
 * not clash with the inline one. Retire it when the containing source is recovered. */
#include "Game/fn_8040_vector_types.h"

extern "C" bool fn_8000A950(float value) {
    return fn_8000A980(value) < 0.0001f;
}
