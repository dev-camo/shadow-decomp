#ifndef GAME_FN_80027D5C_H
#define GAME_FN_80027D5C_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* Original type name unknown; values are consumed by the material callbacks. */
struct Unk801A35ACParameters {
    Unk801A35ACParameters(); // Original constructor at 0x80027D5C.

    /* 0x00 */ RwRGBAReal field_0x0;
    /* 0x10 */ float field_0x10[5]; /* Inferred grouping: two alpha factors, then RGB factors. */
    /* 0x24 */ signed char field_0x24;
    /* 0x28 */ float field_0x28;
};
CHECK_SIZEOF(Unk801A35ACParameters, 0x2C);

#endif
