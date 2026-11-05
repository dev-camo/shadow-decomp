#ifndef GAME_ANGLECONVERSION_H
#define GAME_ANGLECONVERSION_H

#include "Game/SharedConstants.h"

/* Angle helpers shared by game units. The header name is not original. */

/* Converts degrees to radians (name inferred). */
static inline float DegreesToRadians(float degrees) {
    return degrees * (lbl_805FAD38 / 180.0f);
}

/* The same conversion as a function (0x8000AB90), which some callers call instead of converting in
 * place. */
extern "C" float fn_8000AB90(float degrees);

#endif
