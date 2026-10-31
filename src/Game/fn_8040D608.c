#include "layout_check.h"

/* A 0x1C-byte object that the function initialises. The field types follow the access widths;
 * their meanings are unknown. The alignment padding after field_0x0 and field_0x4 is left to the
 * compiler. */
typedef struct Fn8040D608Object {
    /* 0x00 */ unsigned char field_0x0;
    /* 0x02 */ unsigned short field_0x2;
    /* 0x04 */ unsigned short field_0x4;
    /* 0x08 */ unsigned int field_0x8;
    /* 0x0C */ unsigned int field_0xC;
    /* 0x10 */ unsigned int field_0x10;
    /* 0x14 */ unsigned int field_0x14;
    /* 0x18 */ unsigned int field_0x18;
} Fn8040D608Object;
CHECK_SIZEOF(Fn8040D608Object, 0x1C);

void fn_8040D608(Fn8040D608Object *object) {
    object->field_0x0 = 0;
    object->field_0x2 = 0;
    object->field_0x4 = 0xFFFF;
    object->field_0x8 = 0;
    object->field_0xC = 4;
    object->field_0x10 = 30;
    object->field_0x14 = 0;
    object->field_0x18 = 0;
}
