/* Subtracts the scaled second vector from a point and returns the scale (behaviour inferred). */
#include "Game/fn_8040C340.h"

float fn_8040C340(const UnkBss_80570B50 *record, const Fn8040Vector3 *point,
                  Fn8040Vector3 *result) {
    float scale = fn_8040C3AC(record, point);
    *result = *point - record->field_0xC * scale;
    return scale;
}
