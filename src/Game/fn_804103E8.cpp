/* Motion integration under constant acceleration and linear drag (the unit's original file name
 * is unknown). The function name stays a fn_ placeholder; the description of behaviour is inferred
 * from the code. */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

extern "C" {

/* Advances a position and a velocity by duration under a constant acceleration and a drag
 * coefficient. When drag is not positive (drag <= 0) the motion is the plain constant-acceleration
 * step; with a positive drag k the velocity approaches the terminal velocity a / k exponentially:
 *     x' = x + (a / k) t + (v - a / k) (1 - e^(-k t)) / k
 *     v' = a / k + (v - a / k) e^(-k t)
 * The code computes acceleration * inverseDrag (a / k) again at each of its three uses. Each
 * vector operation keeps its result in a local of its own, declared in the order of the original's
 * stack slots. */
void fn_804103E8(Fn8040Vector3 *position, Fn8040Vector3 *velocity,
                 const Fn8040Vector3 *acceleration, float drag, float duration) {
    if (drag <= 0.0f) {
        Fn8040Vector3 newPosition;
        Fn8040Vector3 movedPosition;
        Fn8040Vector3 velocityStep;
        Fn8040Vector3 halfAccelerationStep;
        Fn8040Vector3 accelerationStepSquared;
        Fn8040Vector3 accelerationStep;
        Fn8040Vector3 newVelocity;
        Fn8040Vector3 velocityGain;

        fn_8040CDA8(&accelerationStep, acceleration, duration);
        fn_8040CDA8(&accelerationStepSquared, &accelerationStep, duration);
        fn_8040CDA8(&halfAccelerationStep, &accelerationStepSquared, 0.5f);
        fn_8040CDA8(&velocityStep, velocity, duration);
        fn_8040CDD0(&movedPosition, position, &velocityStep);
        fn_8040CDD0(&newPosition, &movedPosition, &halfAccelerationStep);
        fn_800091D0(position, &newPosition);

        fn_8040CDA8(&velocityGain, acceleration, duration);
        fn_8040CDD0(&newVelocity, velocity, &velocityGain);
        fn_800091D0(velocity, &newVelocity);
    } else {
        Fn8040Vector3 newPosition;
        Fn8040Vector3 driftedPosition;
        Fn8040Vector3 terminalDisplacement;
        Fn8040Vector3 terminalVelocity;
        Fn8040Vector3 decayedTerm;
        Fn8040Vector3 excessOverDragSquared;
        Fn8040Vector3 excessOverDrag;
        Fn8040Vector3 excess;
        Fn8040Vector3 velocityTimesDrag;
        Fn8040Vector3 newVelocity;
        Fn8040Vector3 terminalVelocityBase;
        Fn8040Vector3 decayedOffset;
        Fn8040Vector3 velocityOffset;
        Fn8040Vector3 terminalVelocityForOffset;

        float growth = std::pow(2.7182817f, drag * duration);
        float decay = 1.0f / growth;
        float inverseDrag = 1.0f / drag;

        fn_8040CDA8(&velocityTimesDrag, velocity, drag);
        fn_8040CD74(&excess, &velocityTimesDrag, acceleration);
        fn_8040CDA8(&excessOverDrag, &excess, inverseDrag);
        fn_8040CDA8(&excessOverDragSquared, &excessOverDrag, inverseDrag);
        fn_8040CDA8(&decayedTerm, &excessOverDragSquared, 1.0f - decay);
        fn_8040CDA8(&terminalVelocity, acceleration, inverseDrag);
        fn_8040CDA8(&terminalDisplacement, &terminalVelocity, duration);
        fn_8040CDD0(&driftedPosition, position, &terminalDisplacement);
        fn_8040CDD0(&newPosition, &driftedPosition, &decayedTerm);
        fn_800091D0(position, &newPosition);

        fn_8040CDA8(&terminalVelocityForOffset, acceleration, inverseDrag);
        fn_8040CD74(&velocityOffset, velocity, &terminalVelocityForOffset);
        fn_8040CDA8(&decayedOffset, &velocityOffset, decay);
        fn_8040CDA8(&terminalVelocityBase, acceleration, inverseDrag);
        fn_8040CDD0(&newVelocity, &terminalVelocityBase, &decayedOffset);
        fn_800091D0(velocity, &newVelocity);
    }
}
}
