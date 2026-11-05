#ifndef GAME_FN_8003B8F4_H
#define GAME_FN_8003B8F4_H

#include "Game/fn_8040_vector_types.h"

extern "C" {
/* The counterpart of the direction-to-angle helper fn_8003B9E8 for the x and z rotations (behaviour
 * inferred from its body; the original name is unknown): stores fn_80016260 of -direction->x and
 * direction->y in roll, and the angle that follows from direction->z in pitch. */
void fn_8003B8F4(const Fn8040Vector3 *direction, float *pitch, float *roll);
}

#endif
