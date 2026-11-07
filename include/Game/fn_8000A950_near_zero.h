#ifndef GAME_FN_8000A950_NEAR_ZERO_H
#define GAME_FN_8000A950_NEAR_ZERO_H

#include <cmath>

/* Header inline helpers: the absolute value of a float, and the test of it against 0.0001 (names
 * unknown; behaviour from their bodies). The DOL keeps one copy of each: fn_8000A950, fn_8000A980
 * and the weak MSL fabsf follow fn_8000A800, and fn_8000B044 follows fn_8000AF74, each test with
 * its own 0.0001 literal. The game C++ flags (-inline noauto,level=1) do not inline a function
 * whose body calls another inline function, so callers branch to the kept copies (bl), and each
 * unit that uses one also compiles a weak copy, which the link discards. In PoleHolder, the 0.0001
 * literal of that copy puts the unit's .sdata2 literals in the original order (measured); other
 * callers differ (fn_8040F364 places its copy's literal in .rodata, and fn_8004924C uses only the
 * absolute value and emits no 0.0001).
 *
 * Units built against include/libc, where fabsf is the compiler intrinsic, would inline
 * fn_8000A980; they call it through the declaration in fn_8040_vector_types.h instead. */

/* Absolute value (behaviour inferred). */
extern "C" inline float fn_8000A980(float value) {
    return fabsf(value);
}

/* True when |value| < 0.0001: the value is close enough to 0 to count as 0 (behaviour inferred). */
extern "C" inline bool fn_8000A950(float value) {
    return fn_8000A980(value) < 0.0001f;
}

/* The same test as fn_8000A950 (behaviour inferred). The DOL has two separate copies and the game
 * calls both (ChaoHolder calls each), so they are two functions. */
extern "C" inline bool fn_8000B044(float value) {
    return fn_8000A980(value) < 0.0001f;
}

#endif
