/* Initializes the material callback parameters; original class and filename unknown. */
#include "Game/fn_80027D5C.h"

/* Default colour (name inferred). */
static const RwRGBAReal s_defaultColor = {-1.0f, -1.0f, -1.0f, -1.0f};

Unk801A35ACParameters::Unk801A35ACParameters() : field_0x0(s_defaultColor) {
    field_0x10[2] = 1.0f;
    field_0x10[3] = 1.0f;
    field_0x10[4] = 1.0f;
    for (int i = 0; i < 2; ++i) {
        field_0x10[i] = 1.0f;
    }
    field_0x24 = 0;
    field_0x28 = -1.0f;
}
