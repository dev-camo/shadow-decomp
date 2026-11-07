/* Control of motions: fn_8041FD70 and fn_8041FED0 (see Game/fn_804157E8_motion_control.h for the
 * file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* Rotates the matrix on top of the matrix stack by the three angles of a vector (radians), each
 * truncated to whole angle units of which only the low 16 bits are used; an angle that truncates
 * to zero is skipped. */
void fn_8041FD70(MotionRotationStep *step) {
    int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->x / MOTION_RADIANS_PER_TURN);
    int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->y / MOTION_RADIANS_PER_TURN);
    int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->z / MOTION_RADIANS_PER_TURN);

    if (unitsZ != 0) {
        fn_8040B80C(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsY != 0) {
        fn_8040B848(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsX != 0) {
        fn_8040B884(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
}

void fn_8041FED0(MotionOffsetStep *step) {
    fn_8040B59C(TopMatrix(), *step->offset);
}
