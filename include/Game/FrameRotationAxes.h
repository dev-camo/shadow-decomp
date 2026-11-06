#ifndef GAME_FRAMEROTATIONAXES_H
#define GAME_FRAMEROTATIONAXES_H

#include "rw/rwcore.h"

/* The unit axes that the frame rotation helpers pass to the matrix rotation: fn_800E9C64 the x
 * axis, fn_800E9CEC the y axis and fn_80109AF8 the z axis (behaviour inferred; file and names
 * inferred). Several gadget files have these three vectors next to each other in their .rodata,
 * whether they use them or not; each file that includes this header gets its own copies, as those
 * files do. */
static const RwV3d s_xAxis = {1.0f, 0.0f, 0.0f};
static const RwV3d s_yAxis = {0.0f, 1.0f, 0.0f};
static const RwV3d s_zAxis = {0.0f, 0.0f, 1.0f};

#endif
