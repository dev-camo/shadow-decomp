/* Control of motions: the start of the file (see Game/fn_804157E8_motion_control.h for the file
 * and its pieces). The variables of the file are defined here. */
#include "Game/fn_804157E8_motion_control.h"

/* The factor that is applied to the speed of a range (see fn_804157E8). */
float lbl_805EDD60 = 1.0f;

/* The curve evaluators that fn_804188A0 selects for the direction of the playback: two that
 * return one float for a cursor and one that returns two. */
MotionCurveEvaluator lbl_805F13D0;
MotionCurveEvaluator lbl_805F13D4;
MotionCurveBothEvaluator lbl_805F13D8;

/* The player that fn_80415A18 (lbl_805F13DC) and fn_80415854 (lbl_805F13E0) acted on last. A null
 * control makes them act on this player again. */
MotionPlayer *lbl_805F13DC;
MotionPlayer *lbl_805F13E0;

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
int fn_80415808(_stMtnPlayWork *control, unsigned int nodeId, int arg2, int arg3) {
    if (control != 0) {
        return fn_8041D1A0(control->rootNode, nodeId, arg2, arg3);
    }
    return 0;
}
