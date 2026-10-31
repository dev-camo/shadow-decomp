/* Transform helpers over the current matrix stack (the original file name is unknown; every name
 * here is inferred from the code). A null matrix stands for the matrix on top of the stack: most
 * helpers work on a copy of it and, when it is the destination, store the copy back
 * (fn_80411634 works on it in place). */
#include <cmath>
#include <float.h>

#include "Game/fn_8041116C_matrix_stack.h"

extern "C" {

MatrixStack *lbl_805F13A8;

void fn_8041116C(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    Fn8040Matrix34 sourceCopy;
    Fn8040Matrix34 destinationCopy;
    Fn8040Matrix34 *result;
    float length;

    if (source == 0) {
        fn_8040C148(&sourceCopy, fn_80411700(lbl_805F13A8));
        source = &sourceCopy;
    }
    if (destination == 0) {
        fn_8040C148(&destinationCopy, fn_80411700(lbl_805F13A8));
        result = &destinationCopy;
    } else {
        result = destination;
    }
    if (result != source) {
        fn_8040C148(result, source);
    }

    length = std::sqrt(result->values[0][0] * result->values[0][0] +
                       result->values[1][0] * result->values[1][0] +
                       result->values[2][0] * result->values[2][0]);
    if (length > FLT_EPSILON) {
        float inverseLength = 1.0f / length;

        result->values[0][0] *= inverseLength;
        result->values[1][0] *= inverseLength;
        result->values[2][0] *= inverseLength;
    }

    length = std::sqrt(result->values[0][1] * result->values[0][1] +
                       result->values[1][1] * result->values[1][1] +
                       result->values[2][1] * result->values[2][1]);
    if (length > FLT_EPSILON) {
        float inverseLength = 1.0f / length;

        result->values[0][1] *= inverseLength;
        result->values[1][1] *= inverseLength;
        result->values[2][1] *= inverseLength;
    }

    length = std::sqrt(result->values[0][2] * result->values[0][2] +
                       result->values[1][2] * result->values[1][2] +
                       result->values[2][2] * result->values[2][2]);
    if (length > FLT_EPSILON) {
        float inverseLength = 1.0f / length;

        result->values[0][2] *= inverseLength;
        result->values[1][2] *= inverseLength;
        result->values[2][2] *= inverseLength;
    }

    if (destination == 0) {
        fn_8040C148(fn_80411700(lbl_805F13A8), result);
    }
}

void fn_80411328(Fn8040Vector3 *scale, const Fn8040Matrix34 *matrix) {
    Fn8040Matrix34 matrixCopy;

    if (matrix == 0) {
        fn_8040C148(&matrixCopy, fn_80411700(lbl_805F13A8));
        matrix = &matrixCopy;
    }

    scale->x = std::sqrt(matrix->values[0][0] * matrix->values[0][0] +
                         matrix->values[1][0] * matrix->values[1][0] +
                         matrix->values[2][0] * matrix->values[2][0]);
    scale->y = std::sqrt(matrix->values[0][1] * matrix->values[0][1] +
                         matrix->values[1][1] * matrix->values[1][1] +
                         matrix->values[2][1] * matrix->values[2][1]);
    scale->z = std::sqrt(matrix->values[0][2] * matrix->values[0][2] +
                         matrix->values[1][2] * matrix->values[1][2] +
                         matrix->values[2][2] * matrix->values[2][2]);
}

/* Copies the rotation part (the first three columns) of source into destination. */
static inline void CopyRotation(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    destination->values[0][0] = source->values[0][0];
    destination->values[1][0] = source->values[1][0];
    destination->values[2][0] = source->values[2][0];
    destination->values[0][1] = source->values[0][1];
    destination->values[1][1] = source->values[1][1];
    destination->values[2][1] = source->values[2][1];
    destination->values[0][2] = source->values[0][2];
    destination->values[1][2] = source->values[1][2];
    destination->values[2][2] = source->values[2][2];
}

void fn_804113E8(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source) {
    Fn8040Matrix34 topCopy;

    if (destination != 0) {
        CopyRotation(destination, source);
    } else {
        fn_8040C148(&topCopy, fn_80411700(lbl_805F13A8));
        CopyRotation(&topCopy, source);
        fn_8040C148(fn_80411700(lbl_805F13A8), &topCopy);
    }
}

/* Stores the translation (the fourth column) of a matrix in a vector. */
static inline void CopyTranslation(Fn8040Vector3 *translation, const Fn8040Matrix34 *matrix) {
    translation->x = matrix->values[0][3];
    translation->y = matrix->values[1][3];
    translation->z = matrix->values[2][3];
}

void fn_804114D0(Fn8040Vector3 *translation, const Fn8040Matrix34 *matrix) {
    Fn8040Matrix34 topCopy;

    if (matrix != 0) {
        CopyTranslation(translation, matrix);
    } else {
        fn_8040C148(&topCopy, fn_80411700(lbl_805F13A8));
        CopyTranslation(translation, &topCopy);
    }
}

void fn_80411548(Fn8040Vector3 *destination, const Fn8040Vector3 *left,
                 const Fn8040Vector3 *right) {
    destination->x = left->x + right->x;
    destination->y = left->y + right->y;
    destination->z = left->z + right->z;
}

void fn_8041157C(Fn8040Vector3 *destination, const Fn8040Matrix34 *matrix,
                 const Fn8040Vector3 *vector) {
    Fn8040Vector3 output;
    Fn8040Matrix34 matrixCopy;

    if (matrix == 0) {
        fn_8040C148(&matrixCopy, fn_80411700(lbl_805F13A8));
        matrix = &matrixCopy;
    }

    output.x = vector->x * matrix->values[0][0] + vector->y * matrix->values[0][1] +
               vector->z * matrix->values[0][2];
    output.y = vector->x * matrix->values[1][0] + vector->y * matrix->values[1][1] +
               vector->z * matrix->values[1][2];
    output.z = vector->x * matrix->values[2][0] + vector->y * matrix->values[2][1] +
               vector->z * matrix->values[2][2];

    fn_800091D0(destination, &output);
}

/* The original computes the top matrix inline for both fn_8040BAC4 calls, not through
 * fn_80411700. */
void fn_80411634(Fn8040Matrix34 *result, const Fn8040Matrix34 *base, const Fn8040Matrix34 *other) {
    if (result != 0) {
        fn_8040C204(lbl_805F13A8);
        if (base != 0) {
            fn_8040C148(fn_80411700(lbl_805F13A8), base);
        }
        fn_8040BAC4(&lbl_805F13A8->matrices[lbl_805F13A8->top], other);
        fn_8040C148(result, fn_80411700(lbl_805F13A8));
        fn_8040C244(lbl_805F13A8);
    } else {
        if (base != 0) {
            fn_8040C148(fn_80411700(lbl_805F13A8), base);
        }
        fn_8040BAC4(&lbl_805F13A8->matrices[lbl_805F13A8->top], other);
    }
}

void fn_804116F8(MatrixStack *stack) {
    lbl_805F13A8 = stack;
}

Fn8040Matrix34 *fn_80411700(MatrixStack *stack) {
    return &stack->matrices[stack->top];
}
}
