/* Float absolute-value wrapper (behaviour inferred). Units built with the MSL headers use the
 * inline definition in Game/fn_8000A950_near_zero.h; this is the copy the link kept, with the weak
 * MSL fabsf after it.
 * Matching workaround: this unit is an artificial split of code that has no source yet, and it
 * defines the function as an ordinary one so that the 44 retained bytes (fn_8000A980 and the weak
 * fabsf) come from source. Including only the header emits neither: an unreferenced inline
 * function is not compiled. Retire this unit when the containing source is recovered and its
 * caller emits the copy. */
#include "Game/fn_8040_vector_types.h"

#include <cmath>

extern "C" float fn_8000A980(float value) {
    return fabsf(value);
}
