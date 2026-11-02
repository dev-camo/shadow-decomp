#ifndef GAME_FN_8040CBF8_H
#define GAME_FN_8040CBF8_H

#include "Game/fn_8040_vector_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Two-component arithmetic (behaviour inferred; original C++ identities unknown). */
void fn_8040CBF8(Fn8040Vector2 *result, const Fn8040Vector2 *left, const Fn8040Vector2 *right);
void fn_8040CC1C(Fn8040Vector2 *result, const Fn8040Vector2 *vector, float factor);
void fn_8040CC38(Fn8040Vector2 *result, const Fn8040Vector2 *left, const Fn8040Vector2 *right);

#ifdef __cplusplus
}
#endif

#endif
