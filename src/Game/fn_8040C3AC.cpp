/* Dot product of point minus the first vector with the second vector (behaviour inferred). */
#include "Game/fn_8040C340.h"

float fn_8040C3AC(const UnkBss_80570B50 *record, const Fn8040Vector3 *point) {
    Fn8040Vector3 difference;
    fn_8040CD74(&difference, point, &record->field_0x0);
    return fn_8040CF90(&difference, &record->field_0xC);
}
