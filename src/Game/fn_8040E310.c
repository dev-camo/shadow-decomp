#include "layout_check.h"

/* The field types follow the access widths. mode records which member the setters stored: 1 for
 * the halfword field_0x0, 2 for the word field_0x4. The alignment padding after field_0x0 is left
 * to the compiler. */
typedef struct Fn8040E310Object {
    /* 0x00 */ unsigned short field_0x0;
    /* 0x04 */ unsigned int field_0x4;
    /* 0x08 */ unsigned int mode;
} Fn8040E310Object;
CHECK_SIZEOF(Fn8040E310Object, 0xC);

/* Both setters clear the object first; the cleared fields are then overwritten (inferred
 * helper). */
static inline void fn_8040E310_clear(Fn8040E310Object *object) {
    object->field_0x4 = 0;
    object->field_0x0 = 0;
    object->mode = 0;
}

void fn_8040E310(Fn8040E310Object *object, unsigned int value) {
    fn_8040E310_clear(object);
    object->field_0x4 = value;
    object->mode = 2;
}

void fn_8040E330(Fn8040E310Object *object, unsigned short value) {
    fn_8040E310_clear(object);
    object->field_0x0 = value;
    object->mode = 1;
}
