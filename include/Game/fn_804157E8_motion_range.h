#ifndef GAME_FN_804157E8_MOTION_RANGE_H
#define GAME_FN_804157E8_MOTION_RANGE_H

#include "layout_check.h"

/* The frames and the speed with which a player of the motion control plays its motion (names
 * inferred from the way the functions compare them with the current frame). fn_80415854 and
 * fn_80415A18 scale the speed by lbl_805EDD60. In its own header so that the C motion code
 * (Game/fn_804157E8_motion_control.h) and the C++ callers (Game/GadgetModelInfo.h) share one
 * definition: the two headers declare the control with different types and cannot be included
 * together yet. */
typedef struct MotionRange {
    /* 0x00 */ float startFrame;
    /* 0x04 */ float endFrame;
    /* 0x08 */ float speed;
    /* 0x0C */ float targetFrame;
} MotionRange;
CHECK_SIZEOF(MotionRange, 0x10);

#endif
