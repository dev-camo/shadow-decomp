/* Control of motions: fn_8041FD70 and fn_8041FED0 (see Game/fn_804157E8_motion_control.h for the
 * file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* A full turn in the 16-bit angle units that fn_8041FD70 truncates its angles to. */
#define ANGLE_UNITS_PER_TURN 65536.0f
#define RADIANS_PER_TURN 6.2831855f

/* Rotates the matrix on top of the matrix stack by the three angles of a vector (radians), each
 * truncated to whole angle units of which only the low 16 bits are used; an angle that truncates
 * to zero is skipped. */
void fn_8041FD70(MotionRotationStep *step) {
    int unitsX = (int)(ANGLE_UNITS_PER_TURN * step->rotation->x / RADIANS_PER_TURN);
    int unitsY = (int)(ANGLE_UNITS_PER_TURN * step->rotation->y / RADIANS_PER_TURN);
    int unitsZ = (int)(ANGLE_UNITS_PER_TURN * step->rotation->z / RADIANS_PER_TURN);

    if (unitsZ != 0) {
        fn_8040B80C(TopMatrix(), RADIANS_PER_TURN * (unitsZ & 0xFFFF) / ANGLE_UNITS_PER_TURN);
    }
    if (unitsY != 0) {
        fn_8040B848(TopMatrix(), RADIANS_PER_TURN * (unitsY & 0xFFFF) / ANGLE_UNITS_PER_TURN);
    }
    if (unitsX != 0) {
        fn_8040B884(TopMatrix(), RADIANS_PER_TURN * (unitsX & 0xFFFF) / ANGLE_UNITS_PER_TURN);
    }
}

void fn_8041FED0(MotionOffsetStep *step) {
    fn_8040B59C(TopMatrix(), step->offset);
}
