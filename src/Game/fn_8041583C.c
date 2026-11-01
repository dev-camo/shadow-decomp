/* Control of motions: fn_8041583C (see Game/fn_804157E8_motion_control.h for the file and its
 * pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* The number of players of a control. */
int fn_8041583C(_stMtnPlayWork *control) {
    if (control != 0) {
        return control->playerCount;
    }
    return 0;
}
