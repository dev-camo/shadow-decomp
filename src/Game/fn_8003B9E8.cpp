/* Direction-to-angle helper (behaviour inferred; original name and filename unknown). */
#include <cmath>

#include "Game/SharedConstants.h"
#include "Game/fn_8040_vector_types.h"

extern "C" void fn_8003B9E8(const Fn8040Vector3 *direction, float *pitch, float *yaw) {
    float yawAngle = fn_80016260(direction->x, direction->z);
    float z = direction->z;
    *yaw = yawAngle;

    float trig;
    float angle;
    if (fn_8000A980(direction->x) > fn_8000A980(z)) {
        trig = std::sin(yawAngle);
        angle = fn_80016260(-direction->y * trig, direction->x);
    } else {
        trig = std::cos(yawAngle);
        angle = fn_80016260(-direction->y * trig, direction->z);
    }
    if (trig < 0.0f) {
        angle += (angle < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
    }
    *pitch = angle;
}
