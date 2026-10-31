#include "layout_check.h"

/* The field types follow the access widths; their meanings are unknown. The alignment padding
 * after field_0x0 is left to the compiler. */
typedef struct Fn8040E38CObject {
    /* 0x00 */ short field_0x0;
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
} Fn8040E38CObject;
CHECK_SIZEOF(Fn8040E38CObject, 0xC);

void fn_8040E38C(Fn8040E38CObject *object) {
    object->field_0x4 = 0;
    object->field_0x0 = 0;
    object->field_0x8 = 0;
}
