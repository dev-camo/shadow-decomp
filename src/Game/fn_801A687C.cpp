/* Updates the boundary's full position and its stored horizontal coordinates (inferred). */
#include "Game/Unk8018A374.h"

void Unk8018A374::fn_801A687C(const Fn8040Vector3 &position) {
    fn_800091D0(&field_0x0.field_0xC, &position);
    field_0x0.field_0x0.x = position.x;
    field_0x0.field_0x0.z = position.z;
}
