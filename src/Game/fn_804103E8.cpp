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
 * The code computes acceleration * inverseDrag (a / k) again at each of its three uses. */
void fn_804103E8(Fn8040Vector3 *position, Fn8040Vector3 *velocity,
                 const Fn8040Vector3 *acceleration, float drag, float duration) {
    if (drag <= 0.0f) {
        *position = *position + *velocity * duration + *acceleration * duration * duration * 0.5f;
        *velocity = *velocity + *acceleration * duration;
    } else {
        float growth = std::pow(2.7182817f, drag * duration);
        float decay = 1.0f / growth;
        float inverseDrag = 1.0f / drag;

        *position = *position + *acceleration * inverseDrag * duration +
                    (*velocity * drag - *acceleration) * inverseDrag * inverseDrag * (1.0f - decay);

        *velocity = *acceleration * inverseDrag + (*velocity - *acceleration * inverseDrag) * decay;
    }
}
}
