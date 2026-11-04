/* Control of motions: fn_804202B8 (see Game/fn_804157E8_motion_control.h for the file and its
 * pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* Negates the components of a vector as its options say. */
void fn_804202B8(MotionScaledVectorTrack *track) {
    NegateVector(&track->value, track->options);
}
