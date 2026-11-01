/* Path-control wrapper; original class and filename unknown.
 * NonMatching: float literal order and readonly-span layout differ. */
#include "Game/Unk801A6844.h"
#include "Game/UnkResource_80012700.h"

Unk801A6844::Unk801A6844() : field_0x58(0.0f) {}

Unk801A6844::~Unk801A6844() {
    fn_801A6648();
}

UnkPathData_8001DB08 *Unk801A6844::fn_801A6790(int index) const {
    signed char resourceIndices[9] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
    return static_cast<UnkPathData_8001DB08 *>(
        fn_8001277C(fn_800122D4(), 6, resourceIndices[index]));
}

void Unk801A6844::fn_801A6688(const Fn8040Vector3 *position, const Fn8040Vector3 &rotation,
                              int resourceIndex, bool selectLocalRecord, int reversed) {
    fn_801A6648();
    UnkCollision_8001B494 *collision = fn_8001B494();
    UnkPathData_8001DB08 *resource = fn_801A6790(resourceIndex);
    fn_8001DB08(collision, resource, position, rotation, 2, &m_path,
                selectLocalRecord ? UnkBss_80570B50(lbl_805E07A0, Fn8040Vector3(1.0f, 0.0f, 0.0f))
                                  : lbl_80570B50);
    fn_80018304(&m_path);
    field_0x58 = fn_800183D8(&m_path);
    if (reversed == 0) {
        field_0x5C = 0;
    } else {
        field_0x5C = 1;
    }
    fn_8040CE60(&field_0x60);
}

void Unk801A6844::fn_801A6648() {
    if (fn_800ABDCC(&m_path)) {
        fn_8001E2DC(fn_8001B494(), &m_path);
    }
}

bool Unk801A6844::fn_801A6588(float distance, Fn8040Vector3 *position, Fn8040Vector3 *rotation) {
    bool result = false;
    if (fn_800ABDCC(&m_path)) {
        if (field_0x5C == 0) {
            fn_80018304(&m_path);
        } else {
            fn_800181BC(&m_path);
        }
        result = fn_8001C91C(fn_8001B494(), &m_path, field_0x5C, position, distance, rotation, 1);
        if (rotation != 0) {
            Fn8040Matrix34 matrix;
            fn_8040B6F8(&matrix);
            fn_8001E070(&matrix, rotation, 1);
            fn_800494F8(rotation, &matrix);
        }
    }
    return result;
}
