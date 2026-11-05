/* Axis-angle quaternion helper (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040C874.h"

extern "C" void fn_8040C874(Fn8040Vector4 *result, const Fn8040Vector3 *axis, float angle) {
    float sine;
    float cosine;
    Fn8040Vector3 unitAxis = fn_8040CEE8(*axis);
    fn_80031568(&sine, &cosine, 0.5f * angle);
    Fn8040Vector4 quaternion(sine * unitAxis.x, sine * unitAxis.y, sine * unitAxis.z, cosine);
    *result = quaternion;
}
