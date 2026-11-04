#ifndef GAME_FN_800E9CEC_H
#define GAME_FN_800E9CEC_H

#include "rw/rwcore.h"

/* Rotates the frame about the y axis by angle radians, with the combine operation combine
 * (behaviour inferred from the sin and cos it computes and the matrix rotation it calls). */
extern "C" void fn_800E9CEC(RwFrame *frame, int combine, float angle);

#endif
