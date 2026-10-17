typedef struct Fn8040E38CObject {
    /* 0x00 */ short field_00;
    /* 0x02 */ short pad_02;
    /* 0x04 */ int field_04;
    /* 0x08 */ int field_08;
#include "layout_check.h"
} Fn8040E38CObject;
CHECK_SIZEOF(Fn8040E38CObject, 0xC);

void fn_8040E38C(Fn8040E38CObject *object) {
    object->field_04 = 0;
    object->field_00 = 0;
    object->field_08 = 0;
}
