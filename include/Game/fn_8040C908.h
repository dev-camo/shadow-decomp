#ifndef GAME_FN_8040C908_H
#define GAME_FN_8040C908_H

#include "Game/fn_8040_vector_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Quaternion operations (behaviours inferred); original names and member scope are unknown. */
/* Sets the four components to 0, 0, 0, 1. */
void fn_8040C908(Fn8040Vector4 *result);
/* Scales all four input components. */
Fn8040Vector4 fn_8040C924(const Fn8040Vector4 *input, float scale);
/* If squared length >= 1e-5, scales each component by 1.0f / sqrt(squared length);
 * otherwise sets all components to zero (behaviour inferred). */
void fn_8040C958(Fn8040Vector4 *value);
/* Hamilton product of two four-component quaternions. */
Fn8040Vector4 fn_8040CA10(const Fn8040Vector4 *left, const Fn8040Vector4 *right);

#ifdef __cplusplus
}
#endif

#endif
