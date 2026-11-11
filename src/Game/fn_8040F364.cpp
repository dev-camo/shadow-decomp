/* Rotation and segment helpers (the unit's original file name is unknown): rotation vectors
 * (angles about the x, y and z axes) from direction vectors, the angle between two vectors, the
 * projection of a vector, the closest point on a segment, axis-angle matrices, and the conversions
 * between rotation vectors, matrices and quaternions. Function names stay fn_ placeholders;
 * descriptions of behaviour are inferred from the code. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include <cmath>

#include "Game/SharedConstants.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"

extern "C" {

void fn_804102F4(Fn8040Vector4 *quaternion, const UnkRotation3 *rotation, int order) {
    Fn8040Matrix34 matrix;
    fn_8040C148(&matrix, &lbl_805E0720);

    switch (order) {
    case 0:
        fn_8040B80C(&matrix, rotation->z);
        fn_8040B848(&matrix, rotation->y);
        fn_8040B884(&matrix, rotation->x);
        break;
    case 1:
        fn_8040B80C(&matrix, rotation->z);
        fn_8040B884(&matrix, rotation->x);
        fn_8040B848(&matrix, rotation->y);
        break;
    case 2:
        fn_8040B848(&matrix, rotation->y);
        fn_8040B884(&matrix, rotation->x);
        fn_8040B80C(&matrix, rotation->z);
        break;
    }

    Fn8040Vector4 temporary;
    fn_8040C55C(&temporary, &matrix);
    fn_801B1DD4(quaternion, &temporary);
}

/* Every order except 2 gives the zero vector; cases 0 and 1 are empty in the original. */
UnkRotation3 fn_80410204(const Fn8040Vector4 *quaternion, int order) {
    Fn8040Matrix34 matrix;
    Fn8040Matrix34 matrixTemporary;
    fn_8040B228(&matrixTemporary, quaternion);
    fn_8040C148(&matrix, &matrixTemporary);

    switch (order) {
    case 2: {
        Fn8040Vector3 rotatedUp;
        Fn8040Vector3 rotatedForward;
        rotatedUp = matrix * Fn8040Vector3(0.0f, 1.0f, 0.0f);

        rotatedForward = matrix * Fn8040Vector3(0.0f, 0.0f, -1.0f);

        return fn_8040F364(rotatedForward, rotatedUp);
    }
    case 0:
    case 1:
        break;
    }
    return UnkRotation3(0.0f, 0.0f, 0.0f);
}

Fn8040Vector3 fn_80410098(const Fn8040Vector3 &vector, const UnkRotation3 *rotation, int order) {
    Fn8040Vector3 rotated = vector;

    switch (order) {
    case 0: {
        rotated = fn_8040D118(rotated, rotation->x);
        rotated = fn_8040D0AC(rotated, rotation->y);
        rotated = fn_8040D044(rotated, rotation->z);
        break;
    }
    case 1: {
        rotated = fn_8040D0AC(rotated, rotation->y);
        rotated = fn_8040D118(rotated, rotation->x);
        rotated = fn_8040D044(rotated, rotation->z);
        break;
    }
    case 2: {
        rotated = fn_8040D044(rotated, rotation->z);
        rotated = fn_8040D118(rotated, rotation->x);
        rotated = fn_8040D0AC(rotated, rotation->y);
        break;
    }
    }

    return rotated;
}

void fn_8040FFBC(Fn8040Matrix34 *result, const Fn8040Matrix34 *matrix, const UnkRotation3 *rotation,
                 int order) {
    Fn8040Matrix34 rotated;
    fn_8040C148(&rotated, matrix);

    switch (order) {
    case 0:
        fn_8040B80C(&rotated, rotation->z);
        fn_8040B848(&rotated, rotation->y);
        fn_8040B884(&rotated, rotation->x);
        break;
    case 1:
        fn_8040B80C(&rotated, rotation->z);
        fn_8040B884(&rotated, rotation->x);
        fn_8040B848(&rotated, rotation->y);
        break;
    case 2:
        fn_8040B848(&rotated, rotation->y);
        fn_8040B884(&rotated, rotation->x);
        fn_8040B80C(&rotated, rotation->z);
        break;
    }

    fn_8040C148(result, &rotated);
}

void fn_8040FEBC(Fn8040Matrix34 *result, const Fn8040Matrix34 *matrix, const UnkRotation3 *rotation,
                 int order) {
    Fn8040Matrix34 rotated;
    fn_8040C148(&rotated, matrix);

    switch (order) {
    case 0:
        fn_8040B884(&rotated, -rotation->x);
        fn_8040B848(&rotated, -rotation->y);
        fn_8040B80C(&rotated, -rotation->z);
        break;
    case 1:
        fn_8040B848(&rotated, -rotation->y);
        fn_8040B884(&rotated, -rotation->x);
        fn_8040B80C(&rotated, -rotation->z);
        break;
    case 2:
        fn_8040B80C(&rotated, -rotation->z);
        fn_8040B884(&rotated, -rotation->x);
        fn_8040B848(&rotated, -rotation->y);
        break;
    }

    fn_8040C148(result, &rotated);
}

Fn8040Matrix34 fn_8040FD8C(const Fn8040Vector3 *axis, float angle) {
    float x = axis->x;
    float y = axis->y;
    float z = axis->z;
    float sine = std::sin(angle);
    float cosine = std::cos(angle);
    float complement = 1.0f - cosine;

    return Fn8040Matrix34(x * x * complement + cosine, x * y * complement - z * sine,
                          z * x * complement + y * sine, 0.0f, x * y * complement + z * sine,
                          y * y * complement + cosine, y * z * complement - x * sine, 0.0f,
                          z * x * complement - y * sine, y * z * complement + x * sine,
                          z * z * complement + cosine, 0.0f);
}

/* The closest point of the segment to the point; fraction receives its position along the segment
 * (0 at the start, 1 at the end, outside that range when the point lies beyond an end) unless it
 * is null. A segment too short to have a direction counts as fraction 1. */
Fn8040Vector3 fn_8040FC78(float *fraction, const UnkSegment_80020A20 *segment,
                          const Fn8040Vector3 *point) {
    Fn8040Vector3 direction(segment->end - segment->start);
    Fn8040Vector3 toPoint(*point - segment->start);

    float lengthSquared = fn_8040CF2C(direction);
    float t = fn_8000A950(lengthSquared) ? 1.0f : fn_8040CF90(&direction, &toPoint) / lengthSquared;
    if (fraction) {
        *fraction = t;
    }

    if (t < 0.0f) {
        return segment->start;
    } else if (t > 1.0f) {
        return segment->end;
    } else {
        return segment->start + direction * t;
    }
}

Fn8040Vector3 fn_8040FC3C(const UnkSegment_80020A20 *segment, const Fn8040Vector3 *point) {
    return fn_8040FC78(0, segment, point);
}

bool fn_8040FB88(const UnkSegment_80020A20 *segment, const Fn8040Vector3 *point) {
    Fn8040Vector3 direction = segment->end - segment->start;

    const Fn8040Vector3 &fromStart = *point - segment->start;
    if (fn_8040CF90(&fromStart, &direction) < 0.0f) {
        return false;
    }

    const Fn8040Vector3 &fromEnd = *point - segment->end;
    if (fn_8040CF90(&fromEnd, &direction) > 0.0f) {
        return false;
    }
    return true;
}

Fn8040Vector3 fn_8040FB08(Fn8040Matrix34 *matrix) {
    return Fn8040Vector3(fn_8040C168(matrix, 0, 3), fn_8040C168(matrix, 1, 3),
                         fn_8040C168(matrix, 2, 3));
}

void fn_8040FAA0(Fn8040Vector3 *translation, Fn8040Matrix34 *matrix) {
    translation->x = fn_8040C168(matrix, 0, 3);
    translation->y = fn_8040C168(matrix, 1, 3);
    translation->z = fn_8040C168(matrix, 2, 3);
}

void fn_8040FA20(Fn8040Matrix34 *matrix, const Fn8040Vector3 *translation) {
    *fn_8040C17C(matrix, 0, 3) = translation->x;
    *fn_8040C17C(matrix, 1, 3) = translation->y;
    *fn_8040C17C(matrix, 2, 3) = translation->z;
}

void fn_8040F990(Fn8040Vector3 *parallel, Fn8040Vector3 *perpendicular, const Fn8040Vector3 *vector,
                 const Fn8040Vector3 *onto) {
    float length = fn_8040CF90(vector, onto);
    Fn8040Vector3 projection = *onto * length;

    if (perpendicular) {
        *perpendicular = *vector - projection;
    }
    if (parallel) {
        *parallel = projection;
    }
}

float fn_8040F8FC(const Fn8040Vector3 *a, const Fn8040Vector3 *b) {
    Fn8040Vector3 normalizedA = fn_8040CEE8(*a);
    Fn8040Vector3 normalizedB = fn_8040CEE8(*b);

    float cosine = fn_8040CF90(&normalizedA, &normalizedB);
    if (cosine > 1.0f) {
        cosine = 1.0f;
    } else if (cosine < -1.0f) {
        cosine = -1.0f;
    }
    return fn_801B1DB0(cosine);
}

UnkRotation3 fn_8040F7F8(const Fn8040Vector3 *direction) {
    UnkRotation3 rotation;
    rotation.y = fn_80016260(direction->x, direction->z);
    float reference;
    if (fn_8000A980(direction->x) > fn_8000A980(direction->z)) {
        reference = std::sin(rotation.y);
        rotation.x = fn_80016260(-direction->y * reference, direction->x);
    } else {
        reference = std::cos(rotation.y);
        rotation.x = fn_80016260(-direction->y * reference, direction->z);
    }
    if (reference < 0.0f) {
        rotation.x += (rotation.x < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
    }
    rotation.z = 0.0f;
    return rotation;
}

Fn8040Vector2 fn_8040F760(const Fn8040Vector3 *direction) {
    Fn8040Vector2 rotation;
    float horizontalLength = std::sqrt(direction->x * direction->x + direction->z * direction->z);
    float yaw = fn_80016260(-direction->x, -direction->z);
    float pitch = fn_80016260(direction->y, horizontalLength);
    rotation.x = pitch;
    rotation.y = yaw;
    return rotation;
}

Fn8040Vector2 fn_8040F664(const Fn8040Vector3 &direction) {
    Fn8040Vector2 rotation;
    float yaw = fn_80016260(direction.x, direction.z);
    float pitch;
    float reference;
    if (fn_8000A980(direction.x) > fn_8000A980(direction.z)) {
        reference = std::sin(yaw);
        pitch = fn_80016260(-direction.y * reference, direction.x);
    } else {
        reference = std::cos(yaw);
        pitch = fn_80016260(-direction.y * reference, direction.z);
    }
    if (reference < 0.0f) {
        pitch += (pitch < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
    }
    rotation.x = pitch;
    rotation.y = yaw;
    return rotation;
}

UnkRotation3 fn_8040F5F4(const Fn8040Vector3 *forward, const Fn8040Vector3 *up, int order) {
    switch (order) {
    case 0:
        return fn_8040F450(forward, up);
    case 2:
        return fn_8040F364(*forward, *up);
    default:
        return UnkRotation3(0.0f, 0.0f, 0.0f);
    }
}

UnkRotation3 fn_8040F450(const Fn8040Vector3 *forward, const Fn8040Vector3 *up) {
    UnkRotation3 rotation;
    Fn8040Vector3 cross;
    Fn8040Vector3 upCopy;
    Fn8040Vector3 negatedForward;
    cross = fn_8040CFB8(*forward, *up);
    upCopy = *up;
    negatedForward = fn_80016284(*forward);

    rotation.z = fn_80016260(cross.y, cross.x);
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, rotation.z);

    if (fn_8000A980(cross.x) > fn_8000A980(cross.y)) {
        rotation.y = fn_80016260(-cross.z * cosine, cross.x);
        if (cosine < 0.0f) {
            rotation.y += (rotation.y < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    } else {
        rotation.y = fn_80016260(-cross.z * sine, cross.y);
        if (sine < 0.0f) {
            rotation.y += (rotation.y < 0.0f) ? lbl_805FAD38 : -lbl_805FAD38;
        }
    }

    rotation.x = fn_80016260(negatedForward.x * sine - negatedForward.y * cosine,
                             upCopy.y * cosine - upCopy.x * sine);
    return rotation;
}

UnkRotation3 fn_8040F364(const Fn8040Vector3 &forward, const Fn8040Vector3 &up) {
    UnkRotation3 rotation;
    float horizontalLength = std::sqrt(forward.x * forward.x + forward.z * forward.z);
    float yaw = fn_80016260(-forward.x, -forward.z);
    float pitch = fn_80016260(forward.y, horizontalLength);

    Fn8040Vector3 rotatedUp = fn_8040D0AC(up, -yaw);
    rotatedUp = fn_8040D118(rotatedUp, -pitch);
    float roll = fn_80016260(-rotatedUp.x, rotatedUp.y);

    rotation.x = pitch;
    rotation.y = yaw;
    rotation.z = roll;
    return rotation;
}
}
