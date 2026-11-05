/* Boundary volume of an enemy: a horizontal circle around a position and a cylinder placed at an
 * offset from it (behaviour and names inferred; original type and file names unknown). Three of
 * the five functions use the file's .sdata2 literals (0.0f, 180.0f). They are defined in reverse
 * address order: the compiler emits the last definition first. */
#include "Game/Unk8018A374.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8040_vector_types.h"

/* Sets up both volumes from a position, a rotation and the set data (inferred): the circle of
 * radius moveRange around the position, and the cylinder of radius searchWidth placed searchRange
 * from it at the angle rotation.y plus searchAngle + 180 degrees (converted to radians), raised
 * by searchHeightOffset. */
void Unk8018A374::fn_801A69FC(const Fn8040Vector3 &position, const Fn8040Vector3 &rotation,
                              const UnkEnemySetDataCommon &setData) {
    Fn8040Vector3 center;
    Fn8040Vector3 moved;
    Fn8040Vector3 rotated;
    Fn8040Vector3 offset;

    fn_800091D0(&field_0x0.field_0xC, &position);
    field_0x0.field_0x0.x = position.x;
    field_0x0.field_0x0.z = position.z;
    field_0x0.field_0x18 = setData.moveRange;
    field_0x0.field_0x1C = setData.moveRange * setData.moveRange;

    float turn = (setData.searchAngle + 180.0f) * (lbl_805FAD38 / 180.0f);
    Fn8040Vector3 height(0.0f, setData.searchHeightOffset, 0.0f);
    offset.Set(0.0f, 0.0f, setData.searchRange);
    fn_8040D0AC(&rotated, &offset, rotation.y + turn);
    fn_8040CDD0(&moved, &position, &rotated);
    fn_8040CDD0(&center, &moved, &height);

    fn_800091D0(&field_0x20.field_0xC, &center);
    field_0x20.field_0x0.x = field_0x20.field_0xC.x;
    field_0x20.field_0x0.z = field_0x20.field_0xC.z;
    field_0x20.field_0x18 = setData.searchWidth;
    field_0x20.field_0x1C = setData.searchWidth * setData.searchWidth;
    field_0x20.field_0x20 = setData.searchHeight;
}

/* Tests a full position against the circle's squared-distance limit (inferred). */
bool Unk8018A374::fn_801A69A0(const Fn8040Vector3 &position) const {
    Fn8040Vector3 point(position.x, position.y, position.z);
    bool inside = fn_8040D180(&point, &field_0x0.field_0xC) < field_0x0.field_0x1C;
    return inside;
}

/* Tests a horizontal position against the boundary's squared-distance limit (inferred). */
bool Unk8018A374::fn_801A6944(const Fn8040Vector3 &position) const {
    Fn8040Vector3 horizontal(position.x, 0.0f, position.z);
    bool inside = fn_8040D180(&horizontal, &field_0x0.field_0x0) < field_0x0.field_0x1C;
    return inside;
}

/* Tests a position against the cylinder: its height difference, then its horizontal squared
 * distance (inferred). Declared non-const because that form matches: the cylinder's y is loaded
 * after the register saves (0x801A68DC), while a const method loads it before them. */
bool Unk8018A374::fn_801A68C0(const Fn8040Vector3 &position) {
    if (fn_8000A980(position.y - field_0x20.field_0xC.y) < field_0x20.field_0x20) {
        Fn8040Vector3 horizontal(position.x, 0.0f, position.z);
        if (fn_8040D180(&horizontal, &field_0x20.field_0x0) < field_0x20.field_0x1C) {
            return true;
        }
    }
    return false;
}

/* Updates the boundary's full position and its stored horizontal coordinates (inferred). */
void Unk8018A374::fn_801A687C(const Fn8040Vector3 &position) {
    fn_800091D0(&field_0x0.field_0xC, &position);
    field_0x0.field_0x0.x = position.x;
    field_0x0.field_0x0.z = position.z;
}
