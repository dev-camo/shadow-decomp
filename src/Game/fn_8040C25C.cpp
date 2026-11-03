/* Matrix-stack destruction (inferred); original filename unknown. */
#include "Game/fn_8041116C_matrix_stack.h"

MatrixStack::~MatrixStack() {
    if (matrices) {
        delete[] matrices;
    }
}
