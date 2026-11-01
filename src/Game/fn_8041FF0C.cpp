/* Control of motions: fn_8041FF0C to fn_804201C0 (see Game/fn_804157E8_motion_control.h for the
 * file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* Reflects the components of a vector about the values that its options hold for them. */
void fn_80420158(MotionReflectedVectorTrack *track) {
    PackNode *options = track->options;

    if (options->values[3] & MOTION_AXIS_REFLECT_X) {
        track->value.x = 2.0f * options->rows[1][0] - track->value.x;
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Y) {
        track->value.y = 2.0f * options->rows[1][1] - track->value.y;
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Z) {
        track->value.z = 2.0f * options->rows[1][2] - track->value.z;
    }
}
