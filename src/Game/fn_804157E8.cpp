/* Control of motions: the start of the file (see Game/fn_804157E8_motion_control.h for the file
 * and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* The factor that is applied to the speed of a range (see fn_804157E8). */
float lbl_805EDD60 = 1.0f;

/* The curve evaluators that fn_804188A0 selects for the direction of the playback: two that
 * return one float for a cursor and one that returns two. */
MotionCurveEvaluator motionControlCurveEvaluator0;
MotionCurveEvaluator motionControlCurveEvaluator1;
MotionCurveBothEvaluator motionControlCurveBothEvaluator;

/* The player that fn_80415A18 (motionControlLastPlayer0) and fn_80415854 (motionControlLastPlayer1) acted on last. A null
 * control makes them act on this player again. */
MotionPlayer *motionControlLastPlayer0;
MotionPlayer *motionControlLastPlayer1;

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
int fn_80415808(_stMtnPlayWork *control, unsigned int nodeId, MotionNodeCallback callback,
                void *argument) {
    if (control != 0) {
        return fn_8041D1A0(control->rootNode, nodeId, callback, argument);
    }
    return 0;
}
