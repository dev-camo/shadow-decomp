/* Pops the matrix stack by decrementing its top index if it is positive (behaviour inferred;
 * the original name is unknown). */
#include "Game/fn_8041116C_matrix_stack.h"

void fn_8040C244(MatrixStack *stack) {
    if (stack->top > 0) {
        stack->top--;
    }
}
