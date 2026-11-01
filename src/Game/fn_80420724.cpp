/* Quaternion helpers of the library code at 0x80410630-0x8042166C: the interpolation of two
 * quaternions, the rotation of a matrix as a quaternion and the rotation matrix of a quaternion
 * (the unit's original file name is unknown). Names stay fn_ placeholders; descriptions of
 * behaviour are inferred from the code. Definitions are in address order, with automatic inlining
 * one level deep (see configure.py). */
#include <cmath>

#include "Game/fn_804157E8_motion_control.h"

extern "C" {

int fn_80420724(Fn8040Vector4 *result, const Fn8040Vector4 *a, const Fn8040Vector4 *b, float t) {
    // The components are read once; the original keeps them in registers across the sin calls.
    float ax = a->x;
    float ay = a->y;
    float az = a->z;
    float aw = a->w;
    float bx = b->x;
    float by = b->y;
    float bz = b->z;
    float bw = b->w;
    float dot = ax * bx + ay * by + az * bz + aw * bw;
    float weightB;
    float weightA = 1.0f;

    // Take the shorter way round: negate a's weight when the quaternions point apart.
    if (dot < 0.0f) {
        dot = -dot;
        weightA = -weightA;
    }

    if (dot < 0.9999f) {
        float angle = fn_801B1DB0(dot);
        float inverseSine = 1.0f / std::sin(angle);
        weightB = inverseSine * std::sin((1.0f - t) * angle);
        weightA = weightA * (inverseSine * std::sin(t * angle));
    } else {
        // Nearly parallel: interpolate linearly.
        weightA = weightA * t;
        weightB = 1.0f - t;
    }

    result->x = bx * weightB + ax * weightA;
    result->y = by * weightB + ay * weightA;
    result->z = bz * weightB + az * weightA;
    result->w = bw * weightB + aw * weightA;
    return 1;
}

void fn_804208EC(Fn8040Vector4 *quaternion, const Fn8040Matrix34 *matrix) {
    const float (*m)[4] = matrix->values;

    if (1.0f + (m[0][0] + m[1][1] + m[2][2]) < 1.0f) {
        // The trace is negative: build the quaternion around the largest diagonal element.
        if (m[0][0] > m[1][1]) {
            if (m[0][0] > m[2][2]) {
                float x = 0.5f * std::sqrt(1.0f + (m[0][0] - m[1][1] - m[2][2]));
                float scale = 1.0f / (4.0f * x);
                quaternion->x = x;
                quaternion->y = scale * (m[1][0] + m[0][1]);
                quaternion->z = scale * (m[2][0] + m[0][2]);
                quaternion->w = scale * (m[2][1] - m[1][2]);
            } else {
                float z = 0.5f * std::sqrt(1.0f + (m[2][2] - m[0][0] - m[1][1]));
                float scale = 1.0f / (4.0f * z);
                quaternion->z = z;
                quaternion->x = scale * (m[0][2] + m[2][0]);
                quaternion->y = scale * (m[1][2] + m[2][1]);
                quaternion->w = scale * (m[1][0] - m[0][1]);
            }
        } else if (m[1][1] > m[2][2]) {
            float y = 0.5f * std::sqrt(1.0f + (m[1][1] - m[2][2] - m[0][0]));
            float scale = 1.0f / (4.0f * y);
            quaternion->y = y;
            quaternion->z = scale * (m[2][1] + m[1][2]);
            quaternion->x = scale * (m[0][1] + m[1][0]);
            quaternion->w = scale * (m[0][2] - m[2][0]);
        } else {
            float z = 0.5f * std::sqrt(1.0f + (m[2][2] - m[0][0] - m[1][1]));
            float scale = 1.0f / (4.0f * z);
            quaternion->z = z;
            quaternion->x = scale * (m[0][2] + m[2][0]);
            quaternion->y = scale * (m[1][2] + m[2][1]);
            quaternion->w = scale * (m[1][0] - m[0][1]);
        }
    } else {
        float root = std::sqrt(1.0f + (m[0][0] + m[1][1] + m[2][2]));
        float scale = 1.0f / (2.0f * root);
        quaternion->x = scale * (m[2][1] - m[1][2]);
        quaternion->y = scale * (m[0][2] - m[2][0]);
        quaternion->z = scale * (m[1][0] - m[0][1]);
        quaternion->w = 0.5f * root;
    }
}

void fn_80420B60(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion) {
    float x = quaternion->x;
    float y = quaternion->y;
    float z = quaternion->z;
    float w = quaternion->w;
    float base = 2.0f * w * w - 1.0f;

    // The elements are stored column by column, as the original does; row by row compiles to
    // different code.
    matrix->values[0][0] = 2.0f * x * x + base;
    matrix->values[1][0] = 2.0f * (x * y + w * z);
    matrix->values[2][0] = 2.0f * (x * z - w * y);
    matrix->values[0][1] = 2.0f * (x * y - w * z);
    matrix->values[1][1] = 2.0f * y * y + base;
    matrix->values[2][1] = 2.0f * (y * z + w * x);
    matrix->values[0][2] = 2.0f * (x * z + w * y);
    matrix->values[1][2] = 2.0f * (y * z - w * x);
    matrix->values[2][2] = 2.0f * z * z + base;
    matrix->values[2][3] = 0.0f;
    matrix->values[1][3] = 0.0f;
    matrix->values[0][3] = 0.0f;
}
}
