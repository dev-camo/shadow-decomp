/* Control of motions: fn_80420308 and fn_804203AC (see Game/fn_804157E8_motion_control.h for the
 * file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* The four functions that evaluate three curves (fn_80420308 to fn_804204C8) read the curves
 * through a local pointer.
 * Matching workaround: with track->curves[i] at each use the compiler addresses the curves from
 * track and the function is one instruction shorter than the target, which computes the address
 * of the first curve once and keeps it in a saved register (an inline helper taking the curves
 * gives the shorter code as well). Revisit if the track types turn out to be classes with the
 * curves as one member. */
void fn_80420308(MotionVectorTrack *track) {
    MotionKeyCursor *curves = track->curves;

    if (curves[0].position != 0) {
        track->value.x = 1.0f / lbl_805F13D0(&curves[0]);
    }
    if (curves[1].position != 0) {
        track->value.y = 1.0f / lbl_805F13D0(&curves[1]);
    }
    if (curves[2].position != 0) {
        track->value.z = 1.0f / lbl_805F13D0(&curves[2]);
    }
}

void fn_804203AC(MotionVectorTrack *track) {
    MotionKeyCursor *curves = track->curves;

    if (curves[0].position != 0) {
        track->value.x = lbl_805F13D0(&curves[0]);
    }
    if (curves[1].position != 0) {
        track->value.y = lbl_805F13D0(&curves[1]);
    }
    if (curves[2].position != 0) {
        track->value.z = lbl_805F13D0(&curves[2]);
    }
}
