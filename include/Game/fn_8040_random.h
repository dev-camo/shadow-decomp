#ifndef GAME_FN_8040_RANDOM_H
#define GAME_FN_8040_RANDOM_H

/* Random numbers from the global generator lbl_805E07D4, in the segment of the
 * SonicUSA::System::Random class (class name from RTTI). The original names of these functions are
 * unknown. */

#ifdef __cplusplus
extern "C" {
#endif

/* Returns a random float between min and max (behaviour inferred). */
float fn_8040EC80(float min, float max);
/* Returns a random integer from 0 to count - 1 (behaviour inferred). */
int fn_8040EA90(int count);

#ifdef __cplusplus
}
#endif

#endif
