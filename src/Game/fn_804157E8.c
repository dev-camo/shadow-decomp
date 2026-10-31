/* Control of motions, part 1 (the original file name is unknown; every name here is inferred from
 * the code). Game/fn_804157E8.c, Game/fn_8041583C.c and Game/fn_80415854.c are consecutive
 * pieces of one original file that are kept as separate units until the rest of it is sourced. */
#include "Game/fn_804157E8_motion_control.h"

/* Sets the factor that is applied to the speed of a range (a negative factor becomes 0) and
 * returns the previous one. */
float fn_804157E8(float scale) {
    float previous = lbl_805EDD60;

    if (scale < 0.0f) {
        scale = 0.0f;
    }
    lbl_805EDD60 = scale;
    return previous;
}

/* Calls fn_8041D1A0 for the root node of a control. */
int fn_80415808(MotionControl *control, unsigned int key, int arg2, int arg3) {
    if (control != 0) {
        return fn_8041D1A0(control->rootNode, key, arg2, arg3);
    }
    return 0;
}
