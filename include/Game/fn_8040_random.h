#ifndef GAME_FN_8040_RANDOM_H
#define GAME_FN_8040_RANDOM_H

/* Random numbers from the SonicUSA::System::Random generator (class name from RTTI) that
 * fn_8040EA90.cpp constructs at startup. The original names of these functions are unknown. */

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns a random float between min and max (behaviour inferred). */
float fn_8040EC80(float min, float max);
/* Returns a random float from 0 up to but excluding 1 (behaviour inferred). */
float fn_8040EE90(void);
/* Returns a random float from 0 up to and including 1 (behaviour inferred). */
float fn_8040ECF8(void);
/* Returns a random integer from 0 to count - 1 (behaviour inferred); passes count straight to
 * the generator's integer method (Random::NextInt, name inferred). */
u32 fn_8040EA90(u32 count);

#ifdef __cplusplus
}
#endif

#endif
