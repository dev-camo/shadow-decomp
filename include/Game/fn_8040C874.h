#ifndef GAME_FN_8040C874_H
#define GAME_FN_8040C874_H

#include "Game/fn_8040_vector_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Builds a quaternion from an axis and angle; signature inferred from the original calls. */
void fn_8040C874(Fn8040Vector4 *result, const Fn8040Vector3 *axis, float angle);

#ifdef __cplusplus
}
#endif

#endif
