/* Tests a horizontal position against the boundary's squared-distance limit (inferred). */
#include "Game/Unk8018A374.h"
#include "Game/fn_8040_vector_types.h"

bool Unk8018A374::fn_801A6944(const Fn8040Vector3 &position) const {
    Fn8040Vector3 horizontal(position.x, 0.0f, position.z);
    bool inside = fn_8040D180(&horizontal, &field_0x0.field_0x0) < field_0x0.field_0x1C;
    return inside;
}
