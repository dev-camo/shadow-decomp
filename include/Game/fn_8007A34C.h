#ifndef GAME_FN_8007A34C_H
#define GAME_FN_8007A34C_H

#include "Game/fn_8040_vector_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Copies four quaternion components (signature inferred from its callers). */
void fn_8007A34C(Fn8040Vector4 *destination, const Fn8040Vector4 *source);

#ifdef __cplusplus
}
#endif

#endif
