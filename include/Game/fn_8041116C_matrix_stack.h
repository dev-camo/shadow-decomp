#ifndef FN_8041116C_MATRIX_STACK_H
#define FN_8041116C_MATRIX_STACK_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* The current matrix stack of the transform helpers in fn_8041116C.cpp (name inferred): the helpers
 * take a null matrix to mean the matrix on top of the stack. fn_8040C204 pushes a copy of the top
 * matrix and fn_8040C244 pops one. The layout comes from their accesses and from the constructor
 * fn_8040C2B8, which stores its argument as the capacity (name inferred) and allocates that many
 * matrices; the game constructs the stack with a capacity of 100. */
typedef struct MatrixStack {
    /* 0x00 */ Fn8040Matrix34 *matrices;
    /* 0x04 */ int capacity;
    /* 0x08 */ int top;
} MatrixStack;
CHECK_SIZEOF(MatrixStack, 0xC);

#ifdef __cplusplus
extern "C" {
#endif

extern MatrixStack *lbl_805F13A8;

void fn_8040C204(MatrixStack *stack);
void fn_8040C244(MatrixStack *stack);

/* Stores the matrix in destination with each of its first three columns scaled to length 1 (a
 * column that is too short stays as it is; the translation column is copied unchanged). A null
 * destination or source stands for the top matrix. */
void fn_8041116C(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Stores the lengths of the matrix's first three columns in scale. */
void fn_80411328(Fn8040Vector3 *scale, const Fn8040Matrix34 *matrix);
/* Copies the rotation part of source into destination, or into the top matrix when destination
 * is null. */
void fn_804113E8(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Stores the translation of the matrix in translation. */
void fn_804114D0(Fn8040Vector3 *translation, const Fn8040Matrix34 *matrix);
void fn_80411548(Fn8040Vector3 *destination, const Fn8040Vector3 *left, const Fn8040Vector3 *right);
void fn_8041157C(Fn8040Vector3 *destination, const Fn8040Matrix34 *matrix,
                 const Fn8040Vector3 *vector);
/* Multiplies a matrix by other, applying other first: the matrix is base, or the top matrix when
 * base is null. A non-null result receives the product and the stack is left as it was; with a
 * null result the product replaces the top matrix. */
void fn_80411634(Fn8040Matrix34 *result, const Fn8040Matrix34 *base, const Fn8040Matrix34 *other);
void fn_804116F8(MatrixStack *stack);
Fn8040Matrix34 *fn_80411700(MatrixStack *stack);

#ifdef __cplusplus
}
#endif

#endif
