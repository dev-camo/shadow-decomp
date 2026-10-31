/* Control of motions, part 2 (see Game/fn_804157E8.c). */
#include "Game/fn_804157E8_motion_control.h"

/* The number of players of a control. */
int fn_8041583C(MotionControl *control) {
    if (control != 0) {
        return control->playerCount;
    }
    return 0;
}
