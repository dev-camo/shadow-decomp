/* Two virtual functions of SonicteamUSA::System::Camera::CameraModeMtnViewer (class name from
 * RTTI), the 3-float function between them and the out-of-line copies of the vector functions
 * that the first of them calls: USA (GUPE8P) 0x80008DA8-0x80009208. The rest of the class is not
 * recovered: its vtable (0x8051C40C) also has UnkSlot0 (0x80008D40), UnkSlot3 (0x80008DA4) and
 * the destructor (0x80009274), and 0x80008D4C creates an object. The original file name is
 * unknown. */
#include "Game/AngleConversion.h"
#include "Game/CameraModeUser.h"
#include "Game/fn_8040DB6C.h"
#include "Game/fn_8040_vector_types.h"

namespace SonicteamUSA {
namespace System {
namespace Camera {

/* Recovered view: the class adds no members that the two functions use. The size comes from the
 * 0xAC bytes that 0x80008D4C allocates for an object. */
class CameraModeMtnViewer : public CameraMode {
  public:
    /* Returns -0x3FF (0x80008D40). */
    virtual int UnkSlot0();
    virtual void UnkSlot1(UnkCameraModeEntry_801CCBF4 *entry, float arg1);
    virtual void UnkSlot2(UnkCameraModeEntry_801CCBF4 *entry);
    /* Does nothing (0x80008DA4). */
    virtual void UnkSlot3(UnkCameraModeEntry_801CCBF4 *entry);
    /* 0x80009274. */
    virtual ~CameraModeMtnViewer();
};
CHECK_SIZEOF(CameraModeMtnViewer, 0xAC);

/* Moves the two points with the values of element 0 of the array that fn_80009208 returns
 * (behaviour from the body; what the element's bits and floats stand for is not established).
 * Does nothing when there is no element or when bit 0x4000 or bit 0x8000 of its flags is set.
 * Otherwise, with "offset" for the start point minus the end point:
 * - builds a matrix from the identity with three rotations in this order: about the y axis by
 *   the offset's yaw plus 2 degrees times field_0x1C, about the x axis by 2 degrees times
 *   field_0x20, and about the y axis by minus the yaw; sets the start point to the end point
 *   plus the offset transformed by that matrix;
 * - adds to both points the vector (field_0x10, 0, field_0x14), transformed by the rotation
 *   about the y axis by the new offset's yaw and scaled by a 25th of the new offset's length;
 * - adds half that scale to the y of both points when bit 0x40 is set and subtracts it when bit
 *   0x80 is set;
 * - takes a factor of 1, plus 0.03 when bit 0x1000 is set, minus 0.03 when bit 0x2000 is set,
 *   and, when the offset is longer than 0.1 or the factor is above 1, sets the start point to
 *   the end point plus the offset times the factor.
 * It then passes both points to fn_8000A5D4.
 * Each bit test compares the masked flags with the mask, as the original does (rlwinm and
 * cmplwi at 0x80008E70); testing the masked flags alone gives 216 instructions instead of 222
 * (measured). */
void CameraModeMtnViewer::UnkSlot1(UnkCameraModeEntry_801CCBF4 *, float) {
    Fn8040DB6CEntry *element = fn_8040DB6C(fn_80009208(), 0);

    if (element == NULL || (element->flags & 0x4000) == 0x4000 ||
        (element->flags & 0x8000) == 0x8000) {
        return;
    }

    Fn8040Vector3 offset;
    Fn8040Vector3 start(m_start);
    Fn8040Vector3 end(m_end);

    {
        float turnY = element->field_0x1C;
        float turnX = element->field_0x20;
        Fn8040Matrix34 matrix(lbl_805E0720);

        offset = start - end;

        UnkRotation3 angles(0.0f, 0.0f, 0.0f);
        fn_8003B9E8(&offset, &angles.x, &angles.y);
        fn_8040B848(&matrix, angles.y + DegreesToRadians(2.0f * turnY));
        fn_8040B884(&matrix, DegreesToRadians(2.0f * turnX));
        fn_8040B848(&matrix, -angles.y);
        start = matrix * offset + end;
    }

    float scale = fn_8040CF50(start - end) / 25.0f;

    {
        float moveX = element->field_0x10;
        float moveZ = element->field_0x14;
        Fn8040Matrix34 matrix(lbl_805E0720);

        offset = start - end;

        UnkRotation3 angles(0.0f, 0.0f, 0.0f);
        fn_8003B9E8(&offset, &angles.x, &angles.y);
        fn_8040B848(&matrix, angles.y);
        offset = Fn8040Vector3(moveX, 0.0f, moveZ);
        offset = matrix * offset;
        AddVector(&start, offset * scale);
        AddVector(&end, offset * scale);
    }

    if ((element->flags & 0x40) == 0x40) {
        start.y += 0.5f * scale;
        end.y += 0.5f * scale;
    }
    if ((element->flags & 0x80) == 0x80) {
        start.y -= 0.5f * scale;
        end.y -= 0.5f * scale;
    }

    float factor = 1.0f;

    if ((element->flags & 0x1000) == 0x1000) {
        factor += 0.03f;
    }
    if ((element->flags & 0x2000) == 0x2000) {
        factor -= 0.03f;
    }
    if (fn_8040CF50(start - end) > 0.1f || factor > 1.0f) {
        start = (start - end) * factor + end;
    }

    fn_8000A5D4(this, &start, &end);
}

} // namespace Camera
} // namespace System
} // namespace SonicteamUSA

/* Sets the three components of a 3-float vector and returns it (behaviour inferred; callers use
 * the returned address). The original names of the function and the vector type are unknown.
 * It follows the function with its lowest call (0x80008DD0), like the copies of the vector's
 * inline functions after UnkSlot1, so it may be such a copy itself; that is not established. */
Fn8040Vector3 *fn_80008E1C(Fn8040Vector3 *vector, float x, float y, float z) {
    vector->x = x;
    vector->y = y;
    vector->z = z;
    return vector;
}

namespace SonicteamUSA {
namespace System {
namespace Camera {

/* Clears field_0x18 and sets the points: the start to (0, 10, 20), then the end to the origin. */
void CameraModeMtnViewer::UnkSlot2(UnkCameraModeEntry_801CCBF4 *) {
    field_0x18 = 0.0f;

    Fn8040Vector3 start;
    fn_80008E1C(&start, 0.0f, 10.0f, 20.0f);
    fn_8000A5D4(this, &start, &m_end);

    Fn8040Vector3 end;
    fn_80008E1C(&end, 0.0f, 0.0f, 0.0f);
    fn_8000A5D4(this, &m_start, &end);
}

} // namespace Camera
} // namespace System
} // namespace SonicteamUSA
