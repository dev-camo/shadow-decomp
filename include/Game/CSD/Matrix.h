#ifndef GAME_CSD_MATRIX_H
#define GAME_CSD_MATRIX_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The Chao::CSD 2D matrices and the matrix stack of csdMatrix.cpp. Type, member and function
 * names are inferred from that code. */
namespace Chao {
namespace CSD {

/* A 2x3 matrix: rows {m[0], m[1], m[2]} and {m[3], m[4], m[5]}, with the translation in m[2]
 * and m[5]. */
struct MatrixValue {
    /* 0x00 */ f32 m[6];
};
CHECK_SIZEOF(Chao::CSD::MatrixValue, 0x18);

/* An entry's matrix and two optional extra matrices taken from the palette's matrix pool. The
 * matrix stack scales sub0 along with the matrix and rotates sub1 along with it; the translations
 * only copy them. */
struct Matrix {
    /* 0x00 */ f32 m[6];
    /* 0x18 */ MatrixValue *sub0;
    /* 0x1C */ MatrixValue *sub1;
};
CHECK_SIZEOF(Chao::CSD::Matrix, 0x20);

/* The matrix stack starts with an identity matrix. Pushing a matrix makes it the target of the
 * transformations, which read the previous top of the stack (the source) and write the target. */
void InitMatrixStack();
void TermMatrixStack();
void PushMatrix(Matrix *matrix);
void PopMatrix();
Matrix *GetTopMatrix();

/* Makes the top of the stack the source as well, so that further transformations accumulate. */
void UseTopMatrixAsSource();

void SetMatrixIdentity(Matrix *matrix);

/* The translations set the target's m[2] and m[5]. When the target is not the source they also copy
 * m[0], m[1], m[3] and m[4] from the source, and the same four values of sub1 and sub0 (which
 * TranslateMatrix copies whole). */

/* The target's translation is the point (x, y) transformed by the source. */
void TranslateMatrix(f32 x, f32 y);
/* Only x is transformed (by m[0] and m[2]); y is the target's translation. */
void TranslateMatrixX(f32 x, f32 y);
/* Only y is transformed (by m[4] and m[5]); x is the target's translation. */
void TranslateMatrixY(f32 x, f32 y);
void SetMatrixTranslation(f32 x, f32 y);

/* Rotates the source's rows by the angle and copies its translation; sub1 is rotated likewise
 * (its translation is left), and sub0 is copied whole when the target is not the source. */
void RotateMatrix(f32 degrees);
/* Scales the source's columns and those of sub0 (translations are left). When the target is not
 * the source, also copies the translation and the whole of sub1. */
void ScaleMatrix(f32 x, f32 y);

} // namespace CSD
} // namespace Chao

#endif
