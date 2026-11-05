#ifndef GAME_FN_8005AE20_H
#define GAME_FN_8005AE20_H

/* Returns 10 times value (the original name is unknown). The gadget parameters labelled in metres
 * are scaled by the same factor, so this is likely the conversion from metres to the game's units
 * (inferred). An inline function: where a static initialiser passes its result to a constructor
 * (the Monster gadget's vectors), the call stays out of line and goes to the copy at 0x8005AE20,
 * while a static float initialised with it gets the folded constant at startup. */
extern "C" inline float fn_8005AE20(float value) {
    return 10.0f * value;
}

#endif
