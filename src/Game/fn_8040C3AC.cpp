/* Dot product of point minus the first vector with the second vector (behaviour inferred). */
#include "Game/fn_8040C340.h"

float fn_8040C3AC(const UnkBss_80570B50 *record, const Fn8040Vector3 *point) {
    return fn_8040CF90(*point - record->field_0x0, record->field_0xC);
}
