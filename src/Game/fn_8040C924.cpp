/* Four-component scaling (inferred). */
#include "Game/fn_8040C908.h"

extern "C" Fn8040Vector4 fn_8040C924(const Fn8040Vector4 *input, float scale) {
    return Fn8040Vector4(input->x * scale, input->y * scale, input->z * scale, input->w * scale);
}
