/* Matrix-stack construction (inferred); original filename unknown. */
#include "Game/fn_8041116C_matrix_stack.h"

MatrixStack::MatrixStack(int count) : capacity(count), top(0) {
    matrices = new Fn8040Matrix34[capacity];
    fn_8040C148(matrices, &lbl_805E0720);
}
