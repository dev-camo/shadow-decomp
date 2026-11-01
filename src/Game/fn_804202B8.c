/* Control of motions: fn_804202B8 (see Game/fn_804157E8_motion_control.h for the file and its
 * pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* Negates the components of a vector as its options say. */
void fn_804202B8(MotionScaledVectorTrack *track) {
    PackNode *options = track->options;

    if (options->values[3] & MOTION_AXIS_NEGATE_X) {
        track->value.x = -track->value.x;
    }
    if (options->values[3] & MOTION_AXIS_NEGATE_Y) {
        track->value.y = -track->value.y;
    }
    if (options->values[3] & MOTION_AXIS_NEGATE_Z) {
        track->value.z = -track->value.z;
    }
}
