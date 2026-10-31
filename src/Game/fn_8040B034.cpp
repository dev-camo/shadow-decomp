/* 3x4 matrix helpers (the unit's original file name is unknown): matrices of translation, scale and
 * rotation, products, inverses, and the identity and zero matrices that are built at startup.
 * Function names stay fn_ placeholders; descriptions of behaviour are inferred. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/fn_8040_vector_types.h"

extern "C" {

float *fn_8040C17C(Fn8040Matrix34 *matrix, int row, int column) {
    float *elements = matrix->values[0];
    return &elements[row * 4 + column];
}

float fn_8040C168(Fn8040Matrix34 *matrix, int row, int column) {
    float *elements = matrix->values[0];
    return elements[row * 4 + column];
}

void fn_8040C148(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    float *to = destination->values[0];
    const float *from = source->values[0];
    for (int i = 0; i < 12; i++) {
        to[i] = from[i];
    }
}

void fn_8040BEF8(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    float m00 = source->values[0][0];
    float m01 = source->values[0][1];
    float m02 = source->values[0][2];
    float m10 = source->values[1][0];
    float m11 = source->values[1][1];
    float m12 = source->values[1][2];
    float m20 = source->values[2][0];
    float m21 = source->values[2][1];
    float m22 = source->values[2][2];

    float determinant = m00 * (m11 * m22 - m12 * m21) + m01 * (m12 * m20 - m10 * m22) +
                        m02 * (m10 * m21 - m11 * m20);
    if (determinant == 0.0f) {
        fn_8040BCB4(destination, source);
        return;
    }

    float m03 = source->values[0][3];
    float m13 = source->values[1][3];
    float m23 = source->values[2][3];
    float scale = 1.0f / determinant;
    destination->values[0][0] = scale * (m11 * m22 - m12 * m21);
    destination->values[0][1] = scale * -(m01 * m22 - m02 * m21);
    destination->values[0][2] = scale * (m01 * m12 - m02 * m11);
    destination->values[0][3] = scale * -(m02 * m13 * m21 + m01 * m12 * m23 + m03 * m11 * m22 -
                                          m03 * m12 * m21 - m02 * m11 * m23 - m01 * m13 * m22);
    destination->values[1][0] = scale * -(m10 * m22 - m12 * m20);
    destination->values[1][1] = scale * (m00 * m22 - m02 * m20);
    destination->values[1][2] = scale * -(m00 * m12 - m02 * m10);
    destination->values[1][3] = scale * (m02 * m13 * m20 + m00 * m12 * m23 + m03 * m10 * m22 -
                                         m03 * m12 * m20 - m02 * m10 * m23 - m00 * m13 * m22);
    destination->values[2][0] = scale * (m10 * m21 - m11 * m20);
    destination->values[2][1] = scale * -(m00 * m21 - m01 * m20);
    destination->values[2][2] = scale * (m00 * m11 - m01 * m10);
    destination->values[2][3] = scale * -(m01 * m13 * m20 + m00 * m11 * m23 + m03 * m10 * m21 -
                                          m03 * m11 * m20 - m01 * m10 * m23 - m00 * m13 * m21);
}

/* The in-place form of fn_8040BEF8: the same formula, with the matrix as both source and
 * destination. Unlike fn_8040BEF8, the original computes the products m10 * m22 and m12 * m20
 * of element [1][0] again instead of reusing those of the determinant. */
void fn_8040BCD4(Fn8040Matrix34 *matrix) {
    float m00 = matrix->values[0][0];
    float m01 = matrix->values[0][1];
    float m02 = matrix->values[0][2];
    float m10 = matrix->values[1][0];
    float m11 = matrix->values[1][1];
    float m12 = matrix->values[1][2];
    float m20 = matrix->values[2][0];
    float m21 = matrix->values[2][1];
    float m22 = matrix->values[2][2];

    float determinant = m00 * (m11 * m22 - m12 * m21) + m01 * (m12 * m20 - m10 * m22) +
                        m02 * (m10 * m21 - m11 * m20);
    if (determinant == 0.0f) {
        return;
    }

    float m03 = matrix->values[0][3];
    float m13 = matrix->values[1][3];
    float m23 = matrix->values[2][3];
    float scale = 1.0f / determinant;
    matrix->values[0][0] = scale * (m11 * m22 - m12 * m21);
    matrix->values[0][1] = scale * -(m01 * m22 - m02 * m21);
    matrix->values[0][2] = scale * (m01 * m12 - m02 * m11);
    matrix->values[0][3] = scale * -(m02 * m13 * m21 + m01 * m12 * m23 + m03 * m11 * m22 -
                                     m03 * m12 * m21 - m02 * m11 * m23 - m01 * m13 * m22);
    matrix->values[1][0] = scale * -(m10 * m22 - m12 * m20);
    matrix->values[1][1] = scale * (m00 * m22 - m02 * m20);
    matrix->values[1][2] = scale * -(m00 * m12 - m02 * m10);
    matrix->values[1][3] = scale * (m02 * m13 * m20 + m00 * m12 * m23 + m03 * m10 * m22 -
                                    m03 * m12 * m20 - m02 * m10 * m23 - m00 * m13 * m22);
    matrix->values[2][0] = scale * (m10 * m21 - m11 * m20);
    matrix->values[2][1] = scale * -(m00 * m21 - m01 * m20);
    matrix->values[2][2] = scale * (m00 * m11 - m01 * m10);
    matrix->values[2][3] = scale * -(m01 * m13 * m20 + m00 * m11 * m23 + m03 * m10 * m21 -
                                     m03 * m11 * m20 - m01 * m10 * m23 - m00 * m13 * m21);
}

/* The same code as fn_8040C148: the original has two copies of the function. */
void fn_8040BCB4(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    float *to = destination->values[0];
    const float *from = source->values[0];
    for (int i = 0; i < 12; i++) {
        to[i] = from[i];
    }
}

void fn_8040BAC4(Fn8040Matrix34 *matrix, const Fn8040Matrix34 *other) {
    Fn8040Matrix34 previous;
    fn_8040BCB4(&previous, matrix);

    matrix->values[0][0] = previous.values[0][0] * other->values[0][0] +
                           previous.values[0][1] * other->values[1][0] +
                           previous.values[0][2] * other->values[2][0];
    matrix->values[0][1] = previous.values[0][0] * other->values[0][1] +
                           previous.values[0][1] * other->values[1][1] +
                           previous.values[0][2] * other->values[2][1];
    matrix->values[0][2] = previous.values[0][0] * other->values[0][2] +
                           previous.values[0][1] * other->values[1][2] +
                           previous.values[0][2] * other->values[2][2];
    matrix->values[0][3] = previous.values[0][0] * other->values[0][3] +
                           previous.values[0][1] * other->values[1][3] +
                           previous.values[0][2] * other->values[2][3] + previous.values[0][3];
    matrix->values[1][0] = previous.values[1][0] * other->values[0][0] +
                           previous.values[1][1] * other->values[1][0] +
                           previous.values[1][2] * other->values[2][0];
    matrix->values[1][1] = previous.values[1][0] * other->values[0][1] +
                           previous.values[1][1] * other->values[1][1] +
                           previous.values[1][2] * other->values[2][1];
    matrix->values[1][2] = previous.values[1][0] * other->values[0][2] +
                           previous.values[1][1] * other->values[1][2] +
                           previous.values[1][2] * other->values[2][2];
    matrix->values[1][3] = previous.values[1][0] * other->values[0][3] +
                           previous.values[1][1] * other->values[1][3] +
                           previous.values[1][2] * other->values[2][3] + previous.values[1][3];
    matrix->values[2][0] = previous.values[2][0] * other->values[0][0] +
                           previous.values[2][1] * other->values[1][0] +
                           previous.values[2][2] * other->values[2][0];
    matrix->values[2][1] = previous.values[2][0] * other->values[0][1] +
                           previous.values[2][1] * other->values[1][1] +
                           previous.values[2][2] * other->values[2][1];
    matrix->values[2][2] = previous.values[2][0] * other->values[0][2] +
                           previous.values[2][1] * other->values[1][2] +
                           previous.values[2][2] * other->values[2][2];
    matrix->values[2][3] = previous.values[2][0] * other->values[0][3] +
                           previous.values[2][1] * other->values[1][3] +
                           previous.values[2][2] * other->values[2][3] + previous.values[2][3];
}

void fn_8040B8FC(Fn8040Matrix34 *result, const Fn8040Matrix34 *left, const Fn8040Matrix34 *right) {
    result->Set(
        left->values[0][0] * right->values[0][0] + left->values[0][1] * right->values[1][0] +
            left->values[0][2] * right->values[2][0],
        left->values[0][0] * right->values[0][1] + left->values[0][1] * right->values[1][1] +
            left->values[0][2] * right->values[2][1],
        left->values[0][0] * right->values[0][2] + left->values[0][1] * right->values[1][2] +
            left->values[0][2] * right->values[2][2],
        left->values[0][0] * right->values[0][3] + left->values[0][1] * right->values[1][3] +
            left->values[0][2] * right->values[2][3] + left->values[0][3],
        left->values[1][0] * right->values[0][0] + left->values[1][1] * right->values[1][0] +
            left->values[1][2] * right->values[2][0],
        left->values[1][0] * right->values[0][1] + left->values[1][1] * right->values[1][1] +
            left->values[1][2] * right->values[2][1],
        left->values[1][0] * right->values[0][2] + left->values[1][1] * right->values[1][2] +
            left->values[1][2] * right->values[2][2],
        left->values[1][0] * right->values[0][3] + left->values[1][1] * right->values[1][3] +
            left->values[1][2] * right->values[2][3] + left->values[1][3],
        left->values[2][0] * right->values[0][0] + left->values[2][1] * right->values[1][0] +
            left->values[2][2] * right->values[2][0],
        left->values[2][0] * right->values[0][1] + left->values[2][1] * right->values[1][1] +
            left->values[2][2] * right->values[2][1],
        left->values[2][0] * right->values[0][2] + left->values[2][1] * right->values[1][2] +
            left->values[2][2] * right->values[2][2],
        left->values[2][0] * right->values[0][3] + left->values[2][1] * right->values[1][3] +
            left->values[2][2] * right->values[2][3] + left->values[2][3]);
}

void fn_8040B8C0(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion) {
    Fn8040Matrix34 rotation;
    fn_8040B228(&rotation, quaternion);
    fn_8040BAC4(matrix, &rotation);
}

void fn_8040B884(Fn8040Matrix34 *matrix, float angle) {
    Fn8040Matrix34 rotation;
    fn_8040B1B0(&rotation, angle);
    fn_8040BAC4(matrix, &rotation);
}

void fn_8040B848(Fn8040Matrix34 *matrix, float angle) {
    Fn8040Matrix34 rotation;
    fn_8040B138(&rotation, angle);
    fn_8040BAC4(matrix, &rotation);
}

void fn_8040B80C(Fn8040Matrix34 *matrix, float angle) {
    Fn8040Matrix34 rotation;
    fn_8040B0C0(&rotation, angle);
    fn_8040BAC4(matrix, &rotation);
}

void fn_8040B790(Fn8040Matrix34 *matrix, const Fn8040Vector3 *scale) {
    matrix->values[0][0] *= scale->x;
    matrix->values[0][1] *= scale->y;
    matrix->values[0][2] *= scale->z;
    matrix->values[1][0] *= scale->x;
    matrix->values[1][1] *= scale->y;
    matrix->values[1][2] *= scale->z;
    matrix->values[2][0] *= scale->x;
    matrix->values[2][1] *= scale->y;
    matrix->values[2][2] *= scale->z;
}

void fn_8040B720(Fn8040Matrix34 *matrix, float x, float y, float z) {
    matrix->values[0][0] *= x;
    matrix->values[0][1] *= y;
    matrix->values[0][2] *= z;
    matrix->values[1][0] *= x;
    matrix->values[1][1] *= y;
    matrix->values[1][2] *= z;
    matrix->values[2][0] *= x;
    matrix->values[2][1] *= y;
    matrix->values[2][2] *= z;
}

void fn_8040B6F8(Fn8040Matrix34 *matrix) {
    fn_80325384(matrix, &lbl_805E0720);
}

/* The three results are computed before the first store. */
void fn_8040B67C(Fn8040Vector3 *result, const Fn8040Matrix34 *matrix, const Fn8040Vector3 *vector) {
    float x = matrix->values[0][0] * vector->x + matrix->values[0][1] * vector->y +
              matrix->values[0][2] * vector->z + matrix->values[0][3];
    float y = matrix->values[1][0] * vector->x + matrix->values[1][1] * vector->y +
              matrix->values[1][2] * vector->z + matrix->values[1][3];
    float z = matrix->values[2][0] * vector->x + matrix->values[2][1] * vector->y +
              matrix->values[2][2] * vector->z + matrix->values[2][3];
    result->x = x;
    result->y = y;
    result->z = z;
}

/* The three results are computed before the first store. */
void fn_8040B618(Fn8040Vector3 *result, const Fn8040Matrix34 *matrix, const Fn8040Vector3 *vector) {
    float x = matrix->values[0][0] * vector->x + matrix->values[0][1] * vector->y +
              matrix->values[0][2] * vector->z;
    float y = matrix->values[1][0] * vector->x + matrix->values[1][1] * vector->y +
              matrix->values[1][2] * vector->z;
    float z = matrix->values[2][0] * vector->x + matrix->values[2][1] * vector->y +
              matrix->values[2][2] * vector->z;
    result->x = x;
    result->y = y;
    result->z = z;
}

void fn_8040B59C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *offset) {
    matrix->values[0][3] += matrix->values[0][0] * offset->x + matrix->values[0][1] * offset->y +
                            matrix->values[0][2] * offset->z;
    matrix->values[1][3] += matrix->values[1][0] * offset->x + matrix->values[1][1] * offset->y +
                            matrix->values[1][2] * offset->z;
    matrix->values[2][3] += matrix->values[2][0] * offset->x + matrix->values[2][1] * offset->y +
                            matrix->values[2][2] * offset->z;
}

void fn_8040B52C(Fn8040Matrix34 *matrix, float x, float y, float z) {
    matrix->values[0][3] +=
        matrix->values[0][0] * x + matrix->values[0][1] * y + matrix->values[0][2] * z;
    matrix->values[1][3] +=
        matrix->values[1][0] * x + matrix->values[1][1] * y + matrix->values[1][2] * z;
    matrix->values[2][3] +=
        matrix->values[2][0] * x + matrix->values[2][1] * y + matrix->values[2][2] * z;
}

void fn_8040B4D0(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    destination->Set(source->values[0][0], source->values[1][0], source->values[2][0], 0.0f,
                     source->values[0][1], source->values[1][1], source->values[2][1], 0.0f,
                     source->values[0][2], source->values[1][2], source->values[2][2], 0.0f);
}

void fn_8040B33C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *eye, const Fn8040Vector3 *up,
                 const Fn8040Vector3 *target) {
    Fn8040Vector3 zAxis;
    Fn8040Vector3 xAxis;
    Fn8040Vector3 yAxis;

    Fn8040Vector3 toEye;
    fn_8040CD74(&toEye, eye, target);
    fn_800091EC(&zAxis, &toEye);
    fn_8040CE74(&zAxis);

    Fn8040Vector3 xAxisUnnormalized;
    fn_8040CFB8(&xAxisUnnormalized, up, &zAxis);
    fn_800091EC(&xAxis, &xAxisUnnormalized);
    fn_8040CE74(&xAxis);

    Fn8040Vector3 yAxisUnnormalized;
    fn_8040CFB8(&yAxisUnnormalized, &zAxis, &xAxis);
    fn_800091EC(&yAxis, &yAxisUnnormalized);

    matrix->Set(xAxis.x, xAxis.y, xAxis.z, -fn_8040CF90(eye, &xAxis), yAxis.x, yAxis.y, yAxis.z,
                -fn_8040CF90(eye, &yAxis), zAxis.x, zAxis.y, zAxis.z, -fn_8040CF90(eye, &zAxis));
}

void fn_8040B228(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion) {
    float scale = 2.0f / fn_8040CA84(quaternion);
    float xs = quaternion->x * scale;
    float ys = quaternion->y * scale;
    float zs = quaternion->z * scale;
    float xx = quaternion->x * xs;
    float yy = quaternion->y * ys;
    float zz = quaternion->z * zs;
    float xy = quaternion->x * ys;
    float xz = quaternion->x * zs;
    float yz = quaternion->y * zs;
    float wx = quaternion->w * xs;
    float wy = quaternion->w * ys;
    float wz = quaternion->w * zs;

    matrix->Set(1.0f - (yy + zz), xy - wz, xz + wy, 0.0f, xy + wz, 1.0f - (xx + zz), yz - wx, 0.0f,
                xz - wy, yz + wx, 1.0f - (xx + yy), 0.0f);
}

void fn_8040B1B0(Fn8040Matrix34 *matrix, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);
    matrix->Set(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cosine, -sine, 0.0f, 0.0f, sine, cosine, 0.0f);
}

void fn_8040B138(Fn8040Matrix34 *matrix, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);
    matrix->Set(cosine, 0.0f, sine, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -sine, 0.0f, cosine, 0.0f);
}

void fn_8040B0C0(Fn8040Matrix34 *matrix, float angle) {
    float sine;
    float cosine;
    fn_80031568(&sine, &cosine, angle);
    matrix->Set(cosine, -sine, 0.0f, 0.0f, sine, cosine, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void fn_8040B07C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *scale) {
    matrix->Set(scale->x, 0.0f, 0.0f, 0.0f, 0.0f, scale->y, 0.0f, 0.0f, 0.0f, 0.0f, scale->z, 0.0f);
}

void fn_8040B034(Fn8040Matrix34 *matrix, const Fn8040Vector3 *translation) {
    matrix->Set(1.0f, 0.0f, 0.0f, translation->x, 0.0f, 1.0f, 0.0f, translation->y, 0.0f, 0.0f,
                1.0f, translation->z);
}
}

/* The two matrices are made at startup (a __sinit_ function and a .ctors entry). */
Fn8040Matrix34 lbl_805E0720(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
Fn8040Matrix34 lbl_805E0750(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
