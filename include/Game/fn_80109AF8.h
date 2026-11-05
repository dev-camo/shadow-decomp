#ifndef GAME_FN_80109AF8_H
#define GAME_FN_80109AF8_H

#include "rw/rwcore.h"

/* Rotates the frame about the z axis by angle radians, with the combine operation combine
 * (behaviour inferred: it computes the sine and cosine of angle and passes the axis (0, 0, 1) to
 * the matrix rotation, as fn_800E9CEC does for the y axis). */
extern "C" void fn_80109AF8(RwFrame *frame, int combine, float angle);

#endif
