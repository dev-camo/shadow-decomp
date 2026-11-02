/* Subtracts the scaled second vector from a point and returns the scale (behaviour inferred). */
#include "Game/fn_8040C340.h"

float fn_8040C340(const UnkBss_80570B50 *record, const Fn8040Vector3 *point,
                  Fn8040Vector3 *result) {
    Fn8040Vector3 difference;
    Fn8040Vector3 scaled;
    float scale = fn_8040C3AC(record, point);
    fn_8040CDA8(&scaled, &record->field_0xC, scale);
    fn_8040CD74(&difference, point, &scaled);
    fn_800091D0(result, &difference);
    return scale;
}
